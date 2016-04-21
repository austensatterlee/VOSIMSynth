#include "VOSIMWindow.h"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include "nanovg_gl.h"
#include <Theme.h>
#include <UIWindow.h>
#include <UIUnitSelector.h>
#include <UICircuitPanel.h>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <sftools/Chronometer.hpp>
#include <perf.h>

static int nWndClassReg = 0;
static const char* wndClassName = "VOSIMWndClass";
extern HINSTANCE gHInstance;

sf::WindowHandle syn::VOSIMWindow::sys_CreateChildWindow(sf::WindowHandle a_system_window) {
	int x = 0, y = 0, w = m_size[0], h = m_size[1];

	if (nWndClassReg++ == 0) {
		WNDCLASS wndClass = { CS_OWNDC | CS_DROPSHADOW, DefWindowProc , 0, 0, gHInstance, 0, LoadCursor(NULL, IDC_ARROW), 0, 0, wndClassName};
		RegisterClass(&wndClass);
	}

	sf::WindowHandle childWnd = CreateWindow(wndClassName, "VOSIMApp", WS_CHILD, // | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		x, y, w, h, (HWND)a_system_window, 0, gHInstance, this);


	if (!childWnd && --nWndClassReg == 0) {
		UnregisterClass(wndClassName, gHInstance);
	}

	return childWnd;
}
#endif

void syn::VOSIMWindow::drawThreadFunc() {
		// Make context active
		m_sfmlWindow->setActive(true);
		m_sfmlWindow->setVerticalSyncEnabled(true);

		// Initialize glew
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			printf("Could not init glew.\n");
			return;
		}
		// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
		glGetError();

		// Setup NanoVG context
		NVGcontext* vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);

		m_root = new UIComponent(this);
		m_root->setTheme(make_shared<Theme>(vg));
		m_root->setSize(m_size);
		
		
		UICircuitPanel* circuitPanel = new UICircuitPanel{ this, m_vm, m_unitFactory };
		circuitPanel->setSize(m_size);
		m_root->addChild(circuitPanel); 		

		PerfGraph fps, cpuGraph;

		initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
		initGraph(&cpuGraph, GRAPH_RENDER_MS, "CPU Time");

		sftools::Chronometer timer;
		timer.resume();

		m_running = true;
		Event event;
		while (m_running) {
			glClearColor(200, 200, 200, 255);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			double cpuStartTime = timer.getElapsedTime().asSeconds();
			nvgBeginFrame(vg, m_sfmlWindow->getSize().x, m_sfmlWindow->getSize().y, 1.0);
			m_root->recursiveDraw(vg);
			renderGraph(vg, 5, 5, &fps);
			renderGraph(vg, 5 + 200 + 5, 5, &cpuGraph);
			nvgEndFrame(vg);

			// Measure the CPU time taken excluding swap buffers (as the swap may wait for GPU)
			double dCpuTime = timer.getElapsedTime().asSeconds() - cpuStartTime;

			updateGraph(&fps, dCpuTime);
			updateGraph(&cpuGraph, dCpuTime);

			m_frameCount = (m_frameCount + 1);

			// check all the window's events that were triggered since the last iteration of the loop
			while (m_running && m_sfmlWindow->pollEvent(event)) {
				// "close requested" event: we close the window
				switch (event.type) {
				case Event::Closed:
					m_sfmlWindow->setActive(false);
					m_running = false;
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
					m_lastScroll = { event.mouseWheelScroll, timer.getElapsedTime() };
					m_root->onMouseScroll(cursorPos(), diffCursorPos(), event.mouseWheelScroll.delta);
				}
					break;
				case Event::MouseButtonPressed:
					m_lastClick = { event.mouseButton, timer.getElapsedTime() };
					m_isClicked = true;
					
					m_draggingComponent = m_root->findChild(m_cursor);
					if (m_draggingComponent == m_root)
						m_draggingComponent = nullptr;
					if (m_draggingComponent && !m_draggingComponent->onMouseDown(cursorPos() - m_draggingComponent->parent()->getAbsPos(), diffCursorPos()))
						m_draggingComponent = nullptr;
					if (m_draggingComponent)
						setFocus(m_draggingComponent);
					else
						setFocus(nullptr);
					break;
				case Event::MouseButtonReleased:
					m_lastClick = { event.mouseButton, timer.getElapsedTime() };
					if (m_draggingComponent)
						m_draggingComponent->onMouseUp(cursorPos() - m_draggingComponent->parent()->getAbsPos(), diffCursorPos());
					if(!m_draggingComponent) {
						m_root->onMouseUp(cursorPos(), diffCursorPos());
						setFocus(nullptr);
					}
					m_draggingComponent = nullptr;
					m_isClicked = false;
					break;
				case Event::MouseMoved:
				{
					updateCursorPos({ event.mouseMove.x,event.mouseMove.y });
					if (m_draggingComponent) {
						m_draggingComponent->onMouseDrag(cursorPos() - m_draggingComponent->parent()->getAbsPos(), diffCursorPos());
					} else {
						m_root->onMouseMove(cursorPos(), diffCursorPos());
					}
				}
					break;
				default:
					break;
				}
			} // end event-handler loop

			m_sfmlWindow->display();
		}
		nvgDeleteGL3(vg);
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
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8; // Optional  

	m_sfmlWindow = new sf::RenderWindow{ a_system_window,settings };
	//sf::WindowHandle system_window = sys_CreateChildWindow(a_system_window);
	//if (!system_window) {
		//return false;
	//}
	//m_sfmlWindow = new sf::RenderWindow(system_window, settings);
	m_sfmlWindow->setActive(false);

	m_drawThread = thread(&VOSIMWindow::drawThreadFunc,this);
	m_drawThread.detach();
	return true;
}

void syn::VOSIMWindow::CloseWindow() {
	if (m_sfmlWindow) {
		m_running = false;
		m_drawThread.join();
		m_sfmlWindow->close();
	}
};

