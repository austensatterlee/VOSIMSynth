#include "VOSIMWindow.h"
#include "GL/glew.h"
#include "nanovg_gl.h"
#include "Theme.h"
#include "UICircuitPanel.h"
#include "UIComponent.h"
#include "UIDefaultUnitControl.h"
#include "UIUnitControlContainer.h"
#include <vosimsynth_resources.h>
#include <sftools/ResourceManager.hpp>

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>

static int nWndClassReg = 0;
static const char* wndClassName = "VOSIMWndClass";

void syn::VOSIMWindow::_OpenWindowImplem(sf::WindowHandle a_system_window) {

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

void syn::VOSIMWindow::_CloseWindowImplem() {
	SetWindowLongW(m_sfmlWindow->getSystemHandle(), GWL_STYLE, GetWindowLongW(m_sfmlWindow->getSystemHandle(), GWL_STYLE) & ~WS_CHILD);

	DestroyWindow(m_timerWindow);
	m_timerWindow = nullptr;

	if (--nWndClassReg == 0)
	{
		UnregisterClass(wndClassName, m_HInstance);
	}
}

LRESULT CALLBACK syn::VOSIMWindow::drawFunc(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam) {
	if (Message == WM_CREATE) {
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)LParam;
		SetWindowLongPtr(Handle, GWLP_USERDATA, (LPARAM)(lpcs->lpCreateParams));		

		int mSec = int(1000.0 / 60.0);
		SetTimer(Handle, NULL, mSec, NULL);	
		
		return 0;
	}

	VOSIMWindow* _this = reinterpret_cast<VOSIMWindow*>(GetWindowLongPtr(Handle, GWLP_USERDATA));
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

syn::VOSIMWindow::VOSIMWindow(int a_width, int a_height, VoiceManager* a_vm, UnitFactory* a_unitFactory) :
	m_HInstance(nullptr),
	m_timerWindow(nullptr),
	m_sfmlWindow(nullptr),
	m_isOpen(false),
	m_vg(nullptr),
	m_size(a_width, a_height),
	m_cursor({ 0,0 }),
	m_draggingComponent(nullptr),
	m_vm(a_vm),
	m_unitFactory(a_unitFactory),
	m_root(nullptr),
	m_circuitPanel(nullptr),
	m_frameCount(0),
	m_fps(1.0),
	m_guiInternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE),
	m_guiExternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE)
{
	_initialize();
}


syn::VOSIMWindow::~VOSIMWindow() {
	CloseWindow();

	delete m_root; m_root = nullptr;
	delete m_sfmlWindow; m_sfmlWindow = nullptr;
	nvgDeleteGL3(m_vg); m_vg = nullptr;
}

void syn::VOSIMWindow::_initialize() {
	sf::ContextSettings settings;
	settings.depthBits = 32;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8;
	settings.majorVersion = 3;
	m_sfmlWindow = new sf::RenderWindow(sf::VideoMode(size()[0], size()[1]), "VOSIMWindow", sf::Style::None, settings);
	
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
	if(m_vg) {
		nvgDeleteGL3(m_vg);
	}

#ifndef NDEBUG
	m_vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG | NVG_ANTIALIAS);
#else
	m_vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_ANTIALIAS);
#endif

	m_theme = make_shared<Theme>(m_vg);
	
	if (!m_circuitPanel) {
		m_circuitPanel = new UICircuitPanel(this, m_vm, m_unitFactory);
		m_circuitPanel->setSize(size());
	}

	if (!m_root) {
		m_root = new UIComponent(this);
		m_root->setSize(size());
		m_root->addChild(m_circuitPanel);
	}

	initGraph(&m_fpsGraph, GRAPH_RENDER_FPS, "FPS");
}

void syn::VOSIMWindow::_runLoop() {
	m_sfmlWindow->setActive(true); 

	Color bgColor = colorFromHSL(0.09f, 1.0f, 0.10f);

	glViewport(0, 0, getSize()[0], getSize()[1]);
	glClearColor(bgColor.r(), bgColor.g(), bgColor.b(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	nvgBeginFrame(m_vg, getSize()[0], getSize()[1], 1.0);
	m_root->recursiveDraw(m_vg);
	renderGraph(m_vg, 5, 5, &m_fpsGraph);
	nvgEndFrame(m_vg);
	
	m_sfmlWindow->display();
	m_frameCount++;

	_flushMessageQueues();
	_handleEvents();

	m_sfmlWindow->setActive(false);

	double dCpuTime = m_timer.getElapsedTime().asSeconds();
	m_fps = 1. / dCpuTime;
	m_timer.reset(true);
	updateGraph(&m_fpsGraph, dCpuTime);
}

void syn::VOSIMWindow::_handleEvents() {
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
			m_circuitPanel->setSize(m_size);
			m_root->setSize(m_size);
			glViewport(0, 0, event.size.width, event.size.height);
			break;
		case Event::LostFocus:
			break;
		case Event::GainedFocus:
			m_sfmlWindow->setActive(true);
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
			bool isDblClick = (m_timer.getElapsedTime().asSeconds() - m_lastClick.time.asSeconds() < 0.125);
			isDblClick &= (cursorPos() - Vector2i{ m_lastClick.data.x, m_lastClick.data.y }).norm() <= 2.0;
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

void syn::VOSIMWindow::_updateCursorPos(const Vector2i& newPos) {
	m_dCursor = newPos - m_cursor;
	m_cursor = newPos;
}

void syn::VOSIMWindow::reset() {
	clearFocus();
	m_circuitPanel->reset();
}

syn::UIUnitControl* syn::VOSIMWindow::createUnitControl(unsigned a_classId, int a_unitId) {
	if (m_unitControlMap.find(a_classId) != m_unitControlMap.end()) {
		return m_unitControlMap[a_classId](this, m_vm, a_unitId);
	}
	else {
		return new DefaultUnitControl(this, m_vm, a_unitId);
	}
}

void syn::VOSIMWindow::_queueInternalMessage(GUIMessage* a_msg) {
	m_guiInternalMsgQueue.push(a_msg);
}

void syn::VOSIMWindow::_flushMessageQueues() {
	GUIMessage* msg;
	while (m_guiInternalMsgQueue.pop(msg)) {
		_processMessage(msg);
	}
	while (m_guiExternalMsgQueue.pop(msg)) {
		_processMessage(msg);
	}
}

void syn::VOSIMWindow::_processMessage(GUIMessage* a_msg) {
	a_msg->action(this, &a_msg->data);
	delete a_msg;
}

void syn::VOSIMWindow::queueExternalMessage(GUIMessage* a_msg) {
	m_guiExternalMsgQueue.push(a_msg);
}

void syn::VOSIMWindow::setFocus(UIComponent* a_comp) {
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

Eigen::Vector2i syn::VOSIMWindow::cursorPos() const {
	return m_cursor;
}

Eigen::Vector2i syn::VOSIMWindow::diffCursorPos() const {
	return m_dCursor;
}

Eigen::Vector2i syn::VOSIMWindow::size() const {
	return m_size;
}

void syn::VOSIMWindow::forfeitFocus(UIComponent* a_comp) {
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

bool syn::VOSIMWindow::OpenWindow(sf::WindowHandle a_system_window) {
	if (m_sfmlWindow) {
		sf::ContextSettings settings;
		settings.depthBits = 32;
		settings.stencilBits = 8;
		settings.antialiasingLevel = 8;
		settings.majorVersion = 3;
		m_sfmlWindow->create(sf::VideoMode(size()[0], size()[1]), "VOSIMWindow", sf::Style::None, settings);
	}
	_OpenWindowImplem(a_system_window);
	m_sfmlWindow->setPosition({ 0,0 });
	m_sfmlWindow->setVisible(true);
	m_timer.resume();
	m_isOpen = true;
	return true;
}

void syn::VOSIMWindow::CloseWindow() {
	m_isOpen = false;
	m_sfmlWindow->setVisible(false);
	m_timer.pause();
	_CloseWindowImplem();
}

void syn::VOSIMWindow::save(ByteChunk* a_data) const {
	const vector<UIUnitControlContainer*>& units = m_circuitPanel->getUnits();
	int nUnits = units.size() - 2; // skip input/output units
	a_data->Put<int>(&nUnits);
	for (int i = 0; i < nUnits; i++) {
		int unitId = units[i + 2]->getUnitId();
		Vector2i pos = units[i + 2]->getRelPos();
		a_data->Put<int>(&unitId);
		a_data->Put<Vector2i>(&pos);
		// reserved
		int reservedBytes = 128;
		a_data->Put<int>(&reservedBytes);
		for (int j = 0; j < reservedBytes; j++) {
			char tmp = 0;
			a_data->Put<char>(&tmp);
		}
	}
}

int syn::VOSIMWindow::load(ByteChunk* a_data, int startPos) {
	/* Reset CircuitPanel */
	reset();

	/* Load CircuitPanel */
	// Add units
	int nUnits;
	startPos = a_data->Get<int>(&nUnits, startPos);
	for (int i = 0; i < nUnits; i++) {
		int unitId;
		Vector2i pos;
		startPos = a_data->Get<int>(&unitId, startPos);
		startPos = a_data->Get<Vector2i>(&pos, startPos);
		// reserved
		int reservedBytes;
		startPos = a_data->Get<int>(&reservedBytes, startPos);
		startPos += reservedBytes;

		m_circuitPanel->onAddUnit_(m_vm->getPrototypeCircuit()->getUnit(unitId).getClassIdentifier(), unitId);
		m_circuitPanel->findUnit(unitId)->setRelPos(pos);
	}
	// Add wires
	const vector<ConnectionRecord>& records = m_vm->getPrototypeCircuit()->getConnections();
	for (int i = 0; i < records.size(); i++) {
		const ConnectionRecord& rec = records[i];
		m_circuitPanel->onAddConnection_(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
	}
	return startPos;
};