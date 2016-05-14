#include "VOSIMWindow.h"
#include "GL/glew.h"
#include "SFML/OpenGL.hpp"
#include "nanovg_gl.h"
#include "Theme.h"
#include "UICircuitPanel.h"
#include "UIComponent.h"
#include "UIDefaultUnitControl.h"
#include "UIUnitControlContainer.h"

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>

static int nWndClassReg = 0;
static const char* wndClassName = "VOSIMWndClass";

sf::WindowHandle syn::VOSIMWindow::sys_CreateChildWindow(sf::WindowHandle a_system_window) {
	int x = 0, y = 0, w = m_size[0], h = m_size[1];

	if (nWndClassReg++ == 0) {
		WNDCLASS wndClass = {NULL, drawFunc, 0, 0, m_HInstance, 0, LoadCursor(NULL, IDC_ARROW), 0, 0, wndClassName};
		RegisterClass(&wndClass);
	}

	//	m_childHandle1 = CreateWindow("EDIT", "IPlug2", WS_CHILD | WS_VISIBLE, // | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	//		x, y, w, h, (HWND)a_system_window, 0, m_HInstance, this);

	sf::ContextSettings settings;
	settings.depthBits = 32;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8; // Optional  
	m_sfmlWindow = new sf::RenderWindow(sf::VideoMode(w, h), "", sf::Style::None, settings);
	m_childHandle1 = m_sfmlWindow->getSystemHandle();
	SetWindowLongW(m_childHandle1, GWL_STYLE, GetWindowLongW(m_childHandle1, GWL_STYLE) | WS_CHILD);


	m_childHandle2 = CreateWindow(wndClassName, "IPlug", WS_CHILD, // | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		x, y, w, h, (HWND)a_system_window, 0, m_HInstance, this);

	SetParent(m_childHandle1, a_system_window);

	if ((!m_childHandle2 || !m_childHandle1) && --nWndClassReg == 0) {
		UnregisterClass(wndClassName, m_HInstance);
	}

	return m_childHandle2;
}

void syn::VOSIMWindow::updateCursorPos(const Vector2i& newPos) {
	m_dCursor = toWorldCoords(newPos) - m_cursor;
	m_cursor = toWorldCoords(newPos);
}

void syn::VOSIMWindow::reset() {
	clearFocus();
	m_circuitPanel->reset();
}

syn::UIUnitControl* syn::VOSIMWindow::createUnitControl(unsigned a_classId, int a_unitId) {
	if (m_unitControlMap.find(a_classId) != m_unitControlMap.end()) {
		return m_unitControlMap[a_classId](this, m_vm, a_unitId);
	} else {
		return new DefaultUnitControl(this, m_vm, a_unitId);
	}
}

LRESULT CALLBACK syn::VOSIMWindow::drawFunc(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam) {
	Event event;
	double cpuStartTime, dCpuTime;
	if (Message == WM_CREATE) {
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)LParam;
		SetWindowLongPtr(Handle, GWLP_USERDATA, (LPARAM)(lpcs->lpCreateParams));
		VOSIMWindow* _this = (VOSIMWindow*)GetWindowLongPtr(Handle, GWLP_USERDATA);

		if (_this->m_sfmlWindow) {
			int mSec = int(1000.0 / 60.0);
			SetTimer(Handle, 2, mSec, NULL);

			_this->m_sfmlWindow->setActive(true);
			_this->m_sfmlWindow->requestFocus();
			_this->m_sfmlWindow->setFramerateLimit(60);
			_this->m_sfmlWindow->setPosition({0,0});

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

			if (!_this->m_root) {
				_this->m_root = new UIComponent(_this);
			}
			_this->m_root->setSize(_this->getViewSize());

			if (!_this->m_circuitPanel) {
				_this->m_circuitPanel = new UICircuitPanel(_this, _this->m_vm, _this->m_unitFactory);
				_this->m_root->addChild(_this->m_circuitPanel);
			}
			_this->m_circuitPanel->setSize(_this->getViewSize());

			initGraph(&_this->m_fpsGraph, GRAPH_RENDER_FPS, "Frame Time");
			initGraph(&_this->m_cpuGraph, GRAPH_RENDER_MS, "CPU Time");

			_this->m_timer.resume();
			_this->m_running = true;
			_this->m_isInitialized = true;
		}
		return 0;
	}

	VOSIMWindow* _this = (VOSIMWindow*)GetWindowLongPtr(Handle, GWLP_USERDATA);

	switch (Message) {
	case WM_TIMER:
		cpuStartTime = _this->m_timer.getElapsedTime().asSeconds();
		Color bgColor = colorFromHSL(0.0, 0.5 + 0.5 * sin(cpuStartTime), 0.25);
		glClearColor(bgColor.r(), bgColor.g(), bgColor.b(), 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(_this->m_vg, _this->getSize()[0], _this->getSize()[1], 1.0);
		_this->m_root->recursiveDraw(_this->m_vg);
		renderGraph(_this->m_vg, 5, 5, &_this->m_fpsGraph);
		renderGraph(_this->m_vg, 5 + 200 + 5, 5, &_this->m_cpuGraph);

		// Measure the CPU time taken excluding swap buffers (as the swap may wait for GPU)
		dCpuTime = _this->m_timer.getElapsedTime().asSeconds() - cpuStartTime;

		nvgEndFrame(_this->m_vg);

		updateGraph(&_this->m_fpsGraph, dCpuTime);
		updateGraph(&_this->m_cpuGraph, dCpuTime);

		_this->m_sfmlWindow->display();

		_this->m_frameCount++;

		if (_this->m_draggingComponent) {
			_this->m_draggingComponent->onMouseDrag(_this->cursorPos() - _this->m_draggingComponent->parent()->getAbsPos(), {0,0});
		}

		// check all the window's events that were triggered since the last iteration of the loop
		while (_this->m_sfmlWindow->pollEvent(event)) {
			switch (event.type) {
			case Event::Closed:
				// "close requested" event: we close the window
				_this->m_sfmlWindow->setActive(false);
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
						for (auto it = ++_this->m_focusPath.rbegin(); it != _this->m_focusPath.rend(); ++it)
							if ((*it)->focused())
								(*it)->onTextEntered(event.text.unicode);
					}
				}
				break;
			case Event::KeyPressed:
				if (!_this->m_focusPath.empty()) {
					for (auto it = ++_this->m_focusPath.rbegin(); it != _this->m_focusPath.rend(); ++it)
						if ((*it)->focused())
							(*it)->onKeyDown(event.key);
				}
				break;
			case Event::KeyReleased:
				if (!_this->m_focusPath.empty()) {
					for (auto it = ++_this->m_focusPath.rbegin(); it != _this->m_focusPath.rend(); ++it)
						if ((*it)->focused())
							(*it)->onKeyUp(event.key);
				}
				break;
			case Event::MouseWheelScrolled:
				{
				_this->m_lastScroll = {event.mouseWheelScroll, _this->m_timer.getElapsedTime()};
				if (!_this->m_root->onMouseScroll(_this->cursorPos(), _this->diffCursorPos(), event.mouseWheelScroll.delta)) {
					if (event.mouseWheelScroll.delta > 0) {
						_this->m_zoom += 0.1;
					} else if (event.mouseWheelScroll.delta < 0) {
						_this->m_zoom -= 0.1;
					}
					_this->m_zoom = CLAMP(_this->m_zoom, 1.0, 10.0);
					//_this->setViewSize(Vector2i{_this->m_sfmlWindow->getSize().x * _this->m_zoom, _this->m_sfmlWindow->getSize().y * _this->m_zoom});
				}
				}
				break;
			case Event::MouseButtonPressed:
				{
				_this->m_isClicked = true;

				_this->m_draggingComponent = _this->m_root->onMouseDown(_this->cursorPos(), _this->diffCursorPos(), (_this->m_timer.getElapsedTime().asSeconds() - _this->m_lastClick.time.asSeconds() < 0.25));
				if (_this->m_draggingComponent == _this->m_root)
					_this->m_draggingComponent = nullptr;
				if (_this->m_draggingComponent)
					_this->setFocus(_this->m_draggingComponent);
				else
					_this->setFocus(nullptr);
				_this->m_lastClick = {event.mouseButton, _this->m_timer.getElapsedTime()};
				break;
				}
			case Event::MouseButtonReleased:
				_this->m_lastClick = {event.mouseButton, _this->m_timer.getElapsedTime()};
				if (_this->m_draggingComponent)
					_this->m_draggingComponent->onMouseUp(_this->cursorPos() - _this->m_draggingComponent->parent()->getAbsPos(), _this->diffCursorPos());
				if (!_this->m_draggingComponent) {
					_this->m_root->onMouseUp(_this->cursorPos(), _this->diffCursorPos());
					_this->setFocus(nullptr);
				}
				_this->m_draggingComponent = nullptr;
				_this->m_isClicked = false;
				break;
			case Event::MouseMoved:

				_this->updateCursorPos({event.mouseMove.x - 1,event.mouseMove.y - 2});
				_this->m_root->onMouseMove(_this->cursorPos(), _this->diffCursorPos());

				if (_this->m_draggingComponent) {
					_this->m_draggingComponent->onMouseDrag(_this->cursorPos() - _this->m_draggingComponent->parent()->getAbsPos(), _this->diffCursorPos());
				}
				break;
			default:
				break;
			}
		} // end event-handler loop
		break;
	}
	return DefWindowProc(Handle, Message, WParam, LParam);
}
#endif

syn::VOSIMWindow::~VOSIMWindow() {
	if (m_sfmlWindow) {
		m_sfmlWindow->close();
		delete m_sfmlWindow;
	}
	if (m_root) {
		delete m_root;
	}
}

void syn::VOSIMWindow::setFocus(UIComponent* a_comp) {
	if (a_comp && a_comp != getFocused()) {
		while (!m_focusPath.empty()) {
			m_focusPath.front()->onFocusEvent(false);
			m_focusPath.pop_front();
		}

		UIComponent* gainingFocus = a_comp;
		while (gainingFocus != nullptr) {
			gainingFocus->onFocusEvent(true);
			m_focusPath.push_back(gainingFocus);
			gainingFocus = gainingFocus->parent();
		}
	} else if (a_comp == nullptr) {
		while (!m_focusPath.empty()) {
			m_focusPath.front()->onFocusEvent(false);
			m_focusPath.pop_front();
		}
	}
}

Eigen::Vector2i syn::VOSIMWindow::cursorPos() const {
	return m_cursor;
}

Eigen::Vector2i syn::VOSIMWindow::diffCursorPos() const {
	return m_dCursor;
}

Eigen::Vector2i syn::VOSIMWindow::toWorldCoords(const Vector2i& a_pix) const {
	Vector2f normCoords{float(a_pix[0]) / float(m_sfmlWindow->getSize().x),float(a_pix[1]) / float(m_sfmlWindow->getSize().y)};
	Vector2i worldCoords{normCoords[0] * (m_view[2] - m_view[0]) + m_view[0], normCoords[1] * (m_view[3] - m_view[1]) + m_view[1]};
	return worldCoords;
}

Eigen::Vector2i syn::VOSIMWindow::toPixelCoords(const Vector2i& a_world) const {
	Vector2f normCoords{float(a_world[0] - m_view[0]) / float(m_view[2] - m_view[0]),float(a_world[1] - m_view[1]) / float(m_view[3] - m_view[1])};
	Vector2i pixCoords{normCoords[0] * m_sfmlWindow->getSize().x,normCoords[1] * m_sfmlWindow->getSize().y};
	return pixCoords;
}

void syn::VOSIMWindow::setViewSize(const Vector2i& a_size) {
	m_view[2] = m_view[0] + a_size[0];
	m_view[3] = m_view[1] + a_size[1];
}

void syn::VOSIMWindow::setViewPos(const Vector2i& a_pos) {
	m_view[2] = a_pos[0] + (m_view[2] - m_view[0]);
	m_view[3] = a_pos[1] + (m_view[3] - m_view[1]);
	m_view[0] = a_pos[0];
	m_view[1] = a_pos[1];
}

Eigen::Vector2i syn::VOSIMWindow::getViewSize() const {
	return {m_view[2] - m_view[0],m_view[3] - m_view[1]};
}

bool syn::VOSIMWindow::OpenWindow(sf::WindowHandle a_system_window) {
	if (m_sfmlWindow) {
		m_sfmlWindow->close();
		delete m_sfmlWindow;
		m_sfmlWindow = nullptr;
	}

	sys_CreateChildWindow(a_system_window);

	return true;
}

void syn::VOSIMWindow::CloseWindow() {
	if (m_sfmlWindow) {
		m_running = false;

		nvgDeleteGL3(m_vg);
		m_vg = nullptr;

		m_sfmlWindow->close();
		delete m_sfmlWindow;

		m_sfmlWindow = nullptr;

		m_theme.reset();

		DestroyWindow(m_childHandle1);
		DestroyWindow(m_childHandle2);
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

		m_circuitPanel->onAddUnit_(m_vm->getUnit(unitId).getClassIdentifier(), unitId);
		m_circuitPanel->findUnit(unitId)->setRelPos(pos);
	}
	// Add wires
	const vector<ConnectionRecord>& records = m_vm->getCircuit().getConnections();
	for (int i = 0; i < records.size(); i++) {
		const ConnectionRecord& rec = records[i];
		m_circuitPanel->onAddConnection_(rec.from_id, rec.from_port, rec.to_id, rec.to_port);
	}
	return startPos;
};
