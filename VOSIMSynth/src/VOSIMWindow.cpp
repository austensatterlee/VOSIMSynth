#include "VOSIMWindow.h"
#include "GL/glew.h"
#include "nanovg_gl.h"
#include "Theme.h"
#include "UICircuitPanel.h"
#include "UIComponent.h"
#include "UIDefaultUnitControl.h"
#include "UIUnitControlContainer.h"

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>

static int nWndClassReg = 0;
static const char* wndClassName = "VOSIMWndClass";

sf::WindowHandle syn::VOSIMWindow::_sys_CreateChildWindow(sf::WindowHandle a_system_window) {
	int x = 0, y = 0, w = m_size[0], h = m_size[1];

	if (nWndClassReg++ == 0) {
		WNDCLASS wndClass = { NULL, drawFunc, 0, 0, m_HInstance, 0, NULL, 0, 0, wndClassName };
		RegisterClass(&wndClass);
	}

	//	m_childHandle1 = CreateWindow("EDIT", "IPlug2", WS_CHILD | WS_VISIBLE, // | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	//		x, y, w, h, (HWND)a_system_window, 0, m_HInstance, this);

	sf::ContextSettings settings;
	settings.depthBits = 32;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8;
	settings.majorVersion = 3;
	m_sfmlWindow = new sf::Window(sf::VideoMode(w,h), "VOSIMWindow", sf::Style::None, settings);
	SetWindowLongW(m_sfmlWindow->getSystemHandle(), GWL_STYLE, GetWindowLongW(m_sfmlWindow->getSystemHandle(), GWL_STYLE) | WS_CHILD);

	m_timerWindow = CreateWindow(wndClassName, "VOSIMTimerWindow", NULL, 0, 0, 0, 0, NULL, NULL, m_HInstance, this);

	SetParent(m_sfmlWindow->getSystemHandle(), a_system_window);

	if (!m_timerWindow && --nWndClassReg == 0) {
		UnregisterClass(wndClassName, m_HInstance);
	}

	return m_sfmlWindow->getSystemHandle();
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

LRESULT CALLBACK syn::VOSIMWindow::drawFunc(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam) {
	if (Message == WM_CREATE) {
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)LParam;
		SetWindowLongPtr(Handle, GWLP_USERDATA, (LPARAM)(lpcs->lpCreateParams));
		VOSIMWindow* _this = (VOSIMWindow*)GetWindowLongPtr(Handle, GWLP_USERDATA);

		if (_this->m_sfmlWindow) {
			_this->m_sfmlWindow->setActive(true);
			_this->m_sfmlWindow->setPosition({ 0,0 });
			_this->m_sfmlWindow->setFramerateLimit(60);

			if (!_this->isInitialized()) {
				// Initialize glew
				glewExperimental = GL_TRUE;
				if (glewInit() != GLEW_OK) {
					printf("Could not init glew.\n");
					return 0;
				}

				// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
				glGetError();

				// Setup NanoVG context
				if (!_this->m_vg) {
#ifndef NDEBUG
					_this->m_vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
#else
					_this->m_vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_ANTIALIAS);
#endif
				}
				if (!_this->m_theme) {
					_this->m_theme = make_shared<Theme>(_this->m_vg);
				}

				if (!_this->m_circuitPanel) {
					_this->m_circuitPanel = new UICircuitPanel(_this, _this->m_vm, _this->m_unitFactory);
					_this->m_circuitPanel->setSize(_this->size());
				}

				if (!_this->m_root) {
					_this->m_root = new UIComponent(_this);
					_this->m_root->setSize(_this->size());
					_this->m_root->addChild(_this->m_circuitPanel);
				}

				initGraph(&_this->m_fpsGraph, GRAPH_RENDER_FPS, "FPS");

				_this->m_timer.resume();
				_this->m_isInitialized = true;

				// Set up runloop timer
				int mSec = int(1000.0 / 60.0);
				SetTimer(Handle, NULL, mSec, NULL);
			}
		}
		return 0;
	}

	VOSIMWindow* _this = (VOSIMWindow*)GetWindowLongPtr(Handle, GWLP_USERDATA);
	if (!_this || !_this->isInitialized())
		return DefWindowProc(Handle, Message, WParam, LParam);

	double cpuStartTime, dCpuTime;
	
	switch (Message) {
	case WM_TIMER:
	{
		_this->m_sfmlWindow->setActive(true);
		cpuStartTime = _this->m_timer.getElapsedTime().asSeconds();
		Color bgColor = colorFromHSL(0.19,INVLERP<float>(-1, 1, sin(cpuStartTime * 2 * PI / 1000.0)), 0.15);
		glClearColor(bgColor.r(), bgColor.g(), bgColor.b(), 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(_this->m_vg, _this->getSize()[0], _this->getSize()[1], 1.0);
		_this->m_root->recursiveDraw(_this->m_vg);
		renderGraph(_this->m_vg, 5, 5, &_this->m_fpsGraph);
		nvgEndFrame(_this->m_vg);

		_this->m_sfmlWindow->display();

		_this->m_sfmlWindow->setActive(false);

		// Measure the CPU time taken excluding swap buffers (as the swap may wait for GPU)
		dCpuTime = _this->m_timer.getElapsedTime().asSeconds() - cpuStartTime;
		updateGraph(&_this->m_fpsGraph, dCpuTime);
		_this->m_frameCount++;

		if (_this->m_draggingComponent) {
			_this->m_draggingComponent->onMouseDrag(_this->cursorPos() - _this->m_draggingComponent->getAbsPos(), { 0,0 });
		}

		_this->_flushMessageQueues();

		// check all the window's events that were triggered since the last iteration of the loop
		Event event;
		while (_this->m_sfmlWindow->pollEvent(event)) {
			switch (event.type) {
			case Event::Closed:
				// "close requested" event: we close the window
				_this->CloseWindow();
				break;
			case Event::Resized:
				glViewport(0, 0, _this->getSize()[0], _this->getSize()[1]);
				break;
			case Event::LostFocus:
				_this->m_sfmlWindow->setActive(false);
				break;
			case Event::GainedFocus:
				_this->m_sfmlWindow->setActive(true);
				break;
			case Event::TextEntered:
				if (event.text.unicode >= 32 && event.text.unicode <= 126) {
					if (!_this->m_focusPath.empty()) {
						list<UIComponent*> focusPathCopy = _this->m_focusPath;
						for (list<UIComponent*>::reverse_iterator it = focusPathCopy.rbegin(); it != focusPathCopy.rend(); ++it)
							if ((*it)->focused())
								(*it)->onTextEntered(event.text.unicode);
					}
				}
				break;
			case Event::KeyPressed:
				if (!_this->m_focusPath.empty()) {
					list<UIComponent*> focusPathCopy = _this->m_focusPath;
					for (list<UIComponent*>::reverse_iterator it = focusPathCopy.rbegin(); it != focusPathCopy.rend(); ++it)
						if ((*it)->focused())
							(*it)->onKeyDown(event.key);
				}
				break;
			case Event::KeyReleased:
				if (!_this->m_focusPath.empty()) {
					list<UIComponent*> focusPathCopy = _this->m_focusPath;
					for (list<UIComponent*>::reverse_iterator it = focusPathCopy.rbegin(); it != focusPathCopy.rend(); ++it)
						if ((*it)->focused())
							(*it)->onKeyUp(event.key);
				}
				break;
			case Event::MouseWheelScrolled:
			{
				_this->m_lastScroll = { event.mouseWheelScroll, _this->m_timer.getElapsedTime() };
				_this->m_root->onMouseScroll(_this->cursorPos(), _this->diffCursorPos(), event.mouseWheelScroll.delta);
			}
			break;
			case Event::MouseButtonPressed:
			{
				_this->m_isClicked = true;
				bool isDblClick = (_this->m_timer.getElapsedTime().asSeconds() - _this->m_lastClick.time.asSeconds() < 0.125);
				isDblClick &= (_this->cursorPos() - Vector2i{ _this->m_lastClick.data.x, _this->m_lastClick.data.y }).norm() <= 2.0;
				_this->m_draggingComponent = _this->m_root->onMouseDown(_this->cursorPos(), _this->diffCursorPos(), isDblClick);
				if (_this->m_draggingComponent == _this->m_root)
					_this->m_draggingComponent = nullptr;
				if (_this->m_draggingComponent)
					_this->setFocus(_this->m_draggingComponent);
				else
					_this->setFocus(nullptr);
				_this->m_lastClick = { event.mouseButton, _this->m_timer.getElapsedTime() };
				break;
			}
			case Event::MouseButtonReleased:
				_this->m_lastClick = { event.mouseButton, _this->m_timer.getElapsedTime() };
				if (_this->m_draggingComponent)
					_this->m_draggingComponent->onMouseUp(_this->cursorPos() - _this->m_draggingComponent->getAbsPos(), _this->diffCursorPos());
				if (!_this->m_draggingComponent) {
					_this->m_root->onMouseUp(_this->cursorPos(), _this->diffCursorPos());
				}
				_this->m_draggingComponent = nullptr;
				_this->m_isClicked = false;
				break;
			case Event::MouseMoved:
				if (_this->m_draggingComponent) {
					_this->m_draggingComponent->onMouseDrag(_this->cursorPos() - _this->m_draggingComponent->getAbsPos(), _this->diffCursorPos());
				}

				_this->_updateCursorPos({ event.mouseMove.x - 1,event.mouseMove.y - 2 });
				_this->m_root->onMouseMove(_this->cursorPos(), _this->diffCursorPos());
				break;
			default:
				break;
			}
		} // end event-handler loop
	}
	break;
	default:
		return DefWindowProc(Handle, Message, WParam, LParam);
	} // windows message handler
	return NULL;
}
#endif

syn::VOSIMWindow::~VOSIMWindow() {
	CloseWindow();
	delete m_root; m_root = nullptr;
	delete m_sfmlWindow; m_sfmlWindow = nullptr;
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
		delete m_sfmlWindow;
		m_sfmlWindow = nullptr;
	}
	_sys_CreateChildWindow(a_system_window);

	return true;
}

void syn::VOSIMWindow::CloseWindow() {
	if (m_sfmlWindow) {
		m_sfmlWindow->setVisible(false);

		nvgDeleteGL3(m_vg);
		m_vg = nullptr;

		m_theme.reset();

		m_isInitialized = false;

		DestroyWindow(m_timerWindow);

		if (--nWndClassReg == 0)
		{
			UnregisterClass(wndClassName, m_HInstance);
		}
	}
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
	vector<UIUnitControlContainer*>& units = m_circuitPanel->m_unitControls;
	vector<UIWire*>& wires = m_circuitPanel->m_wires;
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