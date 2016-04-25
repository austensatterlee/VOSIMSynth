#include "VOSIMWindow.h"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include "nanovg_gl.h"
#include <Theme.h>
#include <UIWindow.h>
#include <UIUnitSelector.h>
#include <UICircuitPanel.h>
#include <UIComponent.h>
#include <sftools/Chronometer.hpp>
#include <perf.h>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>

static int nWndClassReg = 0;
static const char* wndClassName = "VOSIMWndClass";

sf::WindowHandle syn::VOSIMWindow::sys_CreateChildWindow(sf::WindowHandle a_system_window) {
	int x = 0, y = 0, w = m_size[0], h = m_size[1];

	if (nWndClassReg++ == 0) {
		WNDCLASS wndClass = { CS_DBLCLKS | CS_OWNDC , drawFunc, 0, 0, m_hinstance, 0, LoadCursor(NULL, IDC_ARROW), 0, 0, wndClassName };
		RegisterClass(&wndClass);
	}

	m_childHandle1 = CreateWindow(wndClassName, "IPlug2", WS_CHILD, // | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		x, y, w, h, (HWND)a_system_window, 0, m_hinstance, this);

	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8; // Optional  
	m_sfmlWindow = new sf::RenderWindow(m_childHandle1, settings);

	m_childHandle2 = CreateWindow(wndClassName, "IPlug", WS_CHILD, // | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		x, y, w, h, (HWND)a_system_window, 0, m_hinstance, this);

	if ((!m_childHandle2 || !m_childHandle1) && --nWndClassReg == 0) {
		UnregisterClass(wndClassName, m_hinstance);
	}

	return m_childHandle2;
}

LRESULT CALLBACK syn::VOSIMWindow::drawFunc(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam) {
	Event event;
	double cpuStartTime, dCpuTime;
	if (Message == WM_CREATE)
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)LParam;
		SetWindowLongPtr(Handle, GWLP_USERDATA, (LPARAM)(lpcs->lpCreateParams));
		VOSIMWindow* _this = (VOSIMWindow*)GetWindowLongPtr(Handle, GWLP_USERDATA);

		if (_this->m_sfmlWindow) {
			int mSec = int(1000.0 / 60.0);
			SetTimer(Handle, 2, mSec, NULL);

			_this->m_sfmlWindow->setActive(true);
			_this->m_sfmlWindow->setFramerateLimit(60);
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
				_this->m_vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
			}
			if (!_this->m_theme) {
				_this->m_theme = make_shared<Theme>(_this->m_vg);
			}

			if (!_this->m_root) {
				_this->m_root = new UIComponent(_this);
			}
			_this->m_root->setSize(_this->m_size);

			if (!_this->m_circuitPanel) {
				_this->m_circuitPanel = new UICircuitPanel(_this, _this->m_vm, _this->m_unitFactory);
				_this->m_root->addChild(_this->m_circuitPanel);
			}
			_this->m_circuitPanel->setSize(_this->m_size);

			initGraph(&_this->m_fpsGraph, GRAPH_RENDER_FPS, "Frame Time");
			initGraph(&_this->m_cpuGraph, GRAPH_RENDER_MS, "CPU Time");

			_this->m_timer.resume();
			_this->m_running = true;
		}
		return 0;
	}
	
	VOSIMWindow* _this = (VOSIMWindow*)GetWindowLongPtr(Handle, GWLP_USERDATA);

	switch(Message) {
	case WM_PAINT:
		break;
	case WM_TIMER:
		glClearColor(200, 200, 200, 255);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		cpuStartTime = _this->m_timer.getElapsedTime().asSeconds();
		nvgBeginFrame(_this->m_vg, _this->m_sfmlWindow->getSize().x, _this->m_sfmlWindow->getSize().y, 1.0);
		_this->m_root->recursiveDraw(_this->m_vg);
		renderGraph(_this->m_vg, 5, 5, &_this->m_fpsGraph);
		renderGraph(_this->m_vg, 5 + 200 + 5, 5, &_this->m_cpuGraph);
		nvgEndFrame(_this->m_vg);

		// Measure the CPU time taken excluding swap buffers (as the swap may wait for GPU)
		dCpuTime = _this->m_timer.getElapsedTime().asSeconds() - cpuStartTime;

		updateGraph(&_this->m_fpsGraph, dCpuTime);
		updateGraph(&_this->m_cpuGraph, dCpuTime);

		_this->m_frameCount++;
		
		if (_this->m_draggingComponent) {
			_this->m_draggingComponent->onMouseDrag(_this->cursorPos() - _this->m_draggingComponent->parent()->getAbsPos(), { 0,0 });
		}

		// check all the window's events that were triggered since the last iteration of the loop
		while (_this->m_sfmlWindow->pollEvent(event)) {
			switch (event.type) {
			case Event::Closed:
				// "close requested" event: we close the window
				_this->m_sfmlWindow->setActive(false);
				break;
			case Event::Resized:
				glViewport(0, 0, event.size.width, event.size.height);
				break;
			case Event::LostFocus:
				break;
			case Event::GainedFocus:
				break;
			case Event::TextEntered:
				break;
			case Event::KeyPressed:
				break;
			case Event::KeyReleased:
				break;
			case Event::MouseWheelScrolled:
			{
				_this->m_lastScroll = { event.mouseWheelScroll, _this->m_timer.getElapsedTime() };
				_this->m_root->onMouseScroll(_this->cursorPos(), _this->diffCursorPos(), event.mouseWheelScroll.delta);
			}
			break;
			case Event::MouseButtonPressed:
				_this->m_lastClick = { event.mouseButton, _this->m_timer.getElapsedTime() };
				_this->m_isClicked = true;

				_this->m_draggingComponent = _this->m_root->findChild(_this->m_cursor);
				if (_this->m_draggingComponent == _this->m_root)
					_this->m_draggingComponent = nullptr;
				if (_this->m_draggingComponent && !_this->m_draggingComponent->onMouseDown(_this->cursorPos() - _this->m_draggingComponent->parent()->getAbsPos(), _this->diffCursorPos()))
					_this->m_draggingComponent = nullptr;
				if (_this->m_draggingComponent)
					_this->setFocus(_this->m_draggingComponent);
				else
					_this->setFocus(nullptr);
				break;
			case Event::MouseButtonReleased:
				_this->m_lastClick = { event.mouseButton, _this->m_timer.getElapsedTime() };
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

				_this->updateCursorPos({ event.mouseMove.x - 1,event.mouseMove.y - 2 });
				if (!_this->m_draggingComponent) {
					_this->m_root->onMouseMove(_this->cursorPos(), _this->diffCursorPos());
				}
				else {
					_this->m_draggingComponent->onMouseDrag(_this->cursorPos() - _this->m_draggingComponent->parent()->getAbsPos(), _this->diffCursorPos());
				}
				break;
			default:
				break;
			}
		} // end event-handler loop
		_this->m_sfmlWindow->display();
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
	if(a_comp && a_comp!=m_focused) {
		if(m_focused) 
			m_focused->onFocusEvent(false);
		m_focused = a_comp;
		m_focused->onFocusEvent(true);
	}else if(a_comp==nullptr && a_comp!=m_focused) {
		m_focused->onFocusEvent(false);
		m_focused = nullptr;
	}
}

bool syn::VOSIMWindow::OpenWindow(sf::WindowHandle a_system_window) {
	if (m_sfmlWindow)
	{
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
};

