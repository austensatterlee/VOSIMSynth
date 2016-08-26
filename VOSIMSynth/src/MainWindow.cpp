#ifndef STB_IMAGE_IMPLEMENTATION
	#define STB_IMAGE_IMPLEMENTATION
#endif
#include "MainWindow.h"
#ifdef NANOVG_GLEW
	#include "GL/glew.h"
#endif
#ifdef NANOVG_GL3_IMPLEMENTATION 
	#include "nanovg_gl.h"
#endif
#include "Theme.h"
#include "VOSIMComponent.h"
#include <sftools/ResourceManager.hpp>

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>

static int nWndClassReg = 0;
static const char* wndClassName = "VOSIMWndClass";

void synui::MainWindow::_OpenWindowImplem(sf::WindowHandle a_system_window) {
	if (nWndClassReg++ == 0) {
		WNDCLASS wndClass = { NULL, drawFunc, 0, 0, m_HInstance, 0, NULL, 0, 0, wndClassName };
		RegisterClass(&wndClass);
	}

	SetWindowLongW(m_sfmlWindow->getSystemHandle(), GWL_STYLE, GetWindowLongW(m_sfmlWindow->getSystemHandle(), GWL_STYLE) | WS_CHILD);
	SetParent(m_sfmlWindow->getSystemHandle(), a_system_window);

	m_timerWindow = CreateWindow(wndClassName, "VOSIMTimerWindow", NULL, 0, 0, 0, 0, NULL, NULL, m_HInstance, this);

	if (!m_timerWindow && --nWndClassReg == 0) {
		UnregisterClass(wndClassName, m_HInstance);
	}
}

void synui::MainWindow::_CloseWindowImplem() {
	SetWindowLongW(m_sfmlWindow->getSystemHandle(), GWL_STYLE, GetWindowLongW(m_sfmlWindow->getSystemHandle(), GWL_STYLE) & ~WS_CHILD);

	DestroyWindow(m_timerWindow);
	m_timerWindow = nullptr;

	if (--nWndClassReg == 0)
	{
		UnregisterClass(wndClassName, m_HInstance);
	}
}

LRESULT CALLBACK synui::MainWindow::drawFunc(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam) {
	if (Message == WM_CREATE) {
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)LParam;
		SetWindowLongPtr(Handle, GWLP_USERDATA, (LPARAM)(lpcs->lpCreateParams));

		int mSec = int(1000.0 / 60.0);
		SetTimer(Handle, NULL, mSec, NULL);

		return 0;
	}

	MainWindow* _this = reinterpret_cast<MainWindow*>(GetWindowLongPtr(Handle, GWLP_USERDATA));
	if (!_this || !_this->isOpen())
		return DefWindowProc(Handle, Message, WParam, LParam);

	switch (Message) {
	case WM_TIMER:
	{
		_this->_runLoop();
	}
	break;
	default:
		return DefWindowProc(Handle, Message, WParam, LParam);
	} // windows message handler
	return NULL;
}
#endif

synui::MainWindow::MainWindow(int a_width, int a_height, std::function<UIComponent*(MainWindow*)> a_rootComponentConstructor) :
	m_HInstance(nullptr),
	m_timerWindow(nullptr),
	m_sfmlWindow(nullptr),
	m_isOpen(false),
	m_vg(nullptr),
	m_size(a_width, a_height),
	m_cursor({ 0,0 }),
	m_draggingComponent(nullptr),
	m_root(nullptr),
	m_rootConstructor(a_rootComponentConstructor),
	m_frameCount(0),
	m_fps(1.0),
	m_guiInternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE),
	m_guiExternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE)
{
	_initialize();
}

synui::MainWindow::~MainWindow() {
	CloseWindow();

	delete m_root; m_root = nullptr;
	delete m_sfmlWindow; m_sfmlWindow = nullptr;
	nvgDeleteGL3(m_vg); m_vg = nullptr;
}

void synui::MainWindow::_initialize() {
	sf::ContextSettings settings;
	settings.depthBits = 32;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8;
	settings.majorVersion = 3;
	m_sfmlWindow = new sf::RenderWindow(sf::VideoMode(size()[0], size()[1]), "MainWindow", sf::Style::None, settings);

	m_sfmlWindow->setVisible(false);
	m_sfmlWindow->setActive(false);

	// Initialize glew
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		printf("Could not init glew.\n");
		return;
	}

	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	// Setup NanoVG context
	if (m_vg) {
		nvgDeleteGL3(m_vg);
	}

#ifndef NDEBUG
	m_vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG | NVG_ANTIALIAS);
#else
	m_vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_ANTIALIAS);
#endif

	m_theme = make_shared<Theme>(m_vg);
	if (!m_root) {
		m_root = m_rootConstructor(this);
		m_root->setSize(size());
	}

	initGraph(&m_fpsGraph, GRAPH_RENDER_FPS, "FPS");
}

void synui::MainWindow::_runLoop() {
	double cpuStartTime = m_timer.getElapsedTime().asSeconds();
	m_sfmlWindow->setActive(true);

	Color bgColor = colorFromHSL(0.9f, 0.4f, 0.22f);

	glViewport(0, 0, getSize()[0], getSize()[1]);
	glClearColor(bgColor.r(), bgColor.g(), bgColor.b(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(m_vg, getSize()[0], getSize()[1], 1.0);
	m_root->recursiveDraw(m_vg);
	//renderGraph(m_vg, 5, 5, &m_fpsGraph);
	nvgEndFrame(m_vg);

	m_sfmlWindow->display();
	m_frameCount++;
	m_sfmlWindow->setActive(false);

	_flushMessageQueues();
	_handleEvents();

	double dCpuTime = m_timer.getElapsedTime().asSeconds() - cpuStartTime;
	double fps = 1. / dCpuTime;
	m_fps = m_fps + 0.0125*(fps - m_fps);
	//updateGraph(&m_fpsGraph, 1./m_fps);
}

void synui::MainWindow::_handleEvents() {
	if (m_draggingComponent) {
		m_draggingComponent->onMouseDrag(UICoord(cursorPos()), { 0,0 });
	}

	// check all the window's events that were triggered since the last iteration of the loop
	Event event;
	while (m_sfmlWindow->isOpen() && m_sfmlWindow->pollEvent(event)) {
		switch (event.type) {
		case Event::Closed:
			// "close requested" event: we close the window
			CloseWindow();
			break;
		case Event::Resized:
			m_size = { event.size.width, event.size.height };
			m_root->setSize(m_size);
			glViewport(0, 0, event.size.width, event.size.height);
			break;
		case Event::LostFocus:
			break;
		case Event::GainedFocus:
			break;
		case Event::TextEntered:
			if (event.text.unicode >= 32 && event.text.unicode <= 126) {
				if (!m_focusPath.empty()) {
					list<UIComponent*> focusPathCopy = m_focusPath;
					for (list<UIComponent*>::reverse_iterator it = focusPathCopy.rbegin(); it != focusPathCopy.rend(); ++it)
						if ((*it)->focused())
							(*it)->onTextEntered(event.text.unicode);
				}
			}
			break;
		case Event::KeyPressed:
			if (!m_focusPath.empty()) {
				list<UIComponent*> focusPathCopy = m_focusPath;
				for (list<UIComponent*>::reverse_iterator it = focusPathCopy.rbegin(); it != focusPathCopy.rend(); ++it)
					if ((*it)->focused())
						(*it)->onKeyDown(event.key);
			}
			break;
		case Event::KeyReleased:
			if (!m_focusPath.empty()) {
				list<UIComponent*> focusPathCopy = m_focusPath;
				for (list<UIComponent*>::reverse_iterator it = focusPathCopy.rbegin(); it != focusPathCopy.rend(); ++it)
					if ((*it)->focused())
						(*it)->onKeyUp(event.key);
			}
			break;
		case Event::MouseWheelScrolled:
		{
			m_lastScroll = { event.mouseWheelScroll, m_timer.getElapsedTime() };
			m_root->onMouseScroll(UICoord(cursorPos()), diffCursorPos(), event.mouseWheelScroll.delta);
		}
		break;
		case Event::MouseButtonPressed:
		{
			float clickTime = m_timer.getElapsedTime().asSeconds() - m_lastClick.time.asSeconds();
			float clickDist = (cursorPos().cast<float>() - Vector2f{ m_lastClick.data.x, m_lastClick.data.y }).norm();
			bool isDblClick = clickTime < 0.1 && clickDist <= 3.0;
			m_draggingComponent = m_root->onMouseDown(UICoord(cursorPos()), diffCursorPos(), isDblClick);
			if (m_draggingComponent == m_root)
				m_draggingComponent = nullptr;
			if (m_draggingComponent)
				setFocus(m_draggingComponent);
			else
				setFocus(nullptr);
			m_lastClick = { event.mouseButton, m_timer.getElapsedTime() };
			break;
		}
		case Event::MouseButtonReleased:
			m_lastClick = { event.mouseButton, m_timer.getElapsedTime() };
			if (m_draggingComponent)
				m_draggingComponent->onMouseUp(UICoord(cursorPos()), diffCursorPos());
			if (!m_draggingComponent) {
				m_root->onMouseUp(UICoord(cursorPos()), diffCursorPos());
			}
			m_draggingComponent = nullptr;
			break;
		case Event::MouseMoved:
			if (m_draggingComponent) {
				m_draggingComponent->onMouseDrag(UICoord(cursorPos()), diffCursorPos());
			}

			_updateCursorPos({ event.mouseMove.x - 1,event.mouseMove.y - 2 });
			m_root->onMouseMove(UICoord(cursorPos()), diffCursorPos());
			break;
		default:
			break;
		}
	} // end event-handler loop
}

void synui::MainWindow::_updateCursorPos(const Vector2i& newPos) {
	m_dCursor = newPos - m_cursor;
	m_cursor = newPos;
}

void synui::MainWindow::_queueInternalMessage(GUIMessage* a_msg) {
	m_guiInternalMsgQueue.push(a_msg);
}

void synui::MainWindow::_flushMessageQueues() {
	GUIMessage* msg;
	while (m_guiInternalMsgQueue.pop(msg)) {
		_processMessage(msg);
	}
	while (m_guiExternalMsgQueue.pop(msg)) {
		_processMessage(msg);
	}
}

void synui::MainWindow::_processMessage(GUIMessage* a_msg) {
	a_msg->action(this, &a_msg->data);
	delete a_msg;
}

void synui::MainWindow::queueExternalMessage(GUIMessage* a_msg) {
	m_guiExternalMsgQueue.push(a_msg);
}

void synui::MainWindow::setFocus(UIComponent* a_comp) {
	while (!m_focusPath.empty()) {  // clear focus
		if (m_focusPath.front() == a_comp)
			break;
		m_focusPath.front()->onFocusEvent(false);
		m_focusPath.pop_front();
	}

	if (a_comp != getFocused()) { // set focus path
		UIComponent* gainingFocus = a_comp;
		while (gainingFocus != nullptr) {
			gainingFocus->onFocusEvent(true);
			m_focusPath.push_back(gainingFocus);
			gainingFocus = gainingFocus->parent();
		}
	}
}

Eigen::Vector2i synui::MainWindow::cursorPos() const {
	return m_cursor;
}

Eigen::Vector2i synui::MainWindow::diffCursorPos() const {
	return m_dCursor;
}

Eigen::Vector2i synui::MainWindow::size() const {
	return m_size;
}

void synui::MainWindow::forfeitFocus(UIComponent* a_comp) {
	if (!a_comp->focused())
		return;
	while (!m_focusPath.empty()) {
		UIComponent* focused = m_focusPath.front();
		focused->onFocusEvent(false);
		m_focusPath.pop_front();
		if (focused == a_comp)
			break;
	}
	if (a_comp == m_draggingComponent)
		m_draggingComponent = nullptr;
}

bool synui::MainWindow::OpenWindow(sf::WindowHandle a_system_window) {
	if (m_sfmlWindow) {
		sf::ContextSettings settings;
		settings.depthBits = 32;
		settings.stencilBits = 8;
		settings.antialiasingLevel = 8;
		settings.majorVersion = 3;
		m_sfmlWindow->create(sf::VideoMode(size()[0], size()[1]), "MainWindow", sf::Style::None, settings);
	}
	_OpenWindowImplem(a_system_window);
	m_sfmlWindow->setPosition({ 0,0 });
	m_sfmlWindow->setVisible(true);
	m_timer.resume();
	m_isOpen = true;
	return true;
}

void synui::MainWindow::CloseWindow() {
	m_isOpen = false;
	m_sfmlWindow->setVisible(false);
	m_timer.pause();
	_CloseWindowImplem();
}