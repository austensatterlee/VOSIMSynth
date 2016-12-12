#include "MainWindow.h"
#include "MainGUI.h"

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>

static int nWndClassReg = 0;
static const char* wndClassName = "VOSIMWndClass";

void synui::MainWindow::_OpenWindowImplem(sf::WindowHandle a_system_window)
{
	if (nWndClassReg++ == 0) {
		WNDCLASS wndClass = {NULL, drawFunc, 0, 0, m_HInstance, 0, NULL, 0, 0, wndClassName};
		RegisterClass(&wndClass);
	}

	SetWindowLongW(m_window->getSystemHandle(), GWL_STYLE, GetWindowLongW(m_window->getSystemHandle(), GWL_STYLE) | WS_CHILD);
	SetParent(m_window->getSystemHandle(), a_system_window);

	m_timerWindow = CreateWindow(wndClassName, "VOSIMTimerWindow", NULL, 0, 0, 0, 0, NULL, NULL, m_HInstance, this);

	if (!m_timerWindow && --nWndClassReg == 0) {
		UnregisterClass(wndClassName, m_HInstance);
	}
}

void synui::MainWindow::_CloseWindowImplem()
{
	SetWindowLongW(m_window->getSystemHandle(), GWL_STYLE, GetWindowLongW(m_window->getSystemHandle(), GWL_STYLE) & ~WS_CHILD);

	DestroyWindow(m_timerWindow);
	m_timerWindow = nullptr;

	if (--nWndClassReg == 0) {
		UnregisterClass(wndClassName, m_HInstance);
	}
}

LRESULT CALLBACK synui::MainWindow::drawFunc(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam)
{
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
	case WM_TIMER: { _this->_runLoop(); }
		break;
	default:
		return DefWindowProc(Handle, Message, WParam, LParam);
	} // windows message handler
	return NULL;
}
#endif

synui::MainWindow::MainWindow(int a_width, int a_height, GUIConstructor a_guiConstructor) :
	m_HInstance(nullptr),
	m_timerWindow(nullptr),
	m_window(nullptr),
	m_mainGUI(nullptr),
	m_isOpen(false),
	m_size(a_width, a_height),
	m_frameCount(0),
	m_fps(1.0),
	m_guiInternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE),
	m_guiExternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE)
{
	sf::ContextSettings settings{32, 8, 8, 3};

	m_window = std::make_shared<sf::RenderWindow>(sf::VideoMode{getSize()[0], getSize()[1]}, "MainWindow", sf::Style::None, settings);
	m_window->setPosition({0,0});
	m_window->setVerticalSyncEnabled(true);

	m_mainGUI = std::shared_ptr<MainGUI>(a_guiConstructor(*this));

	m_window->setVisible(false);
	m_window->setActive(false);
}

synui::MainWindow::~MainWindow() { CloseWindow(); }

void synui::MainWindow::_runLoop()
{
	double cpuStartTime = m_timer.getElapsedTime().asSeconds();

	_flushMessageQueues();
	_handleEvents();

	m_window->setActive(true);
	m_window->clear();
	m_mainGUI->draw();
	m_window->display();
	m_frameCount++;
	m_window->setActive(false);

	double dCpuTime = m_timer.getElapsedTime().asSeconds() - cpuStartTime;
	double fps = 1. / dCpuTime;
	m_fps = m_fps + 0.0175 * (fps - m_fps);
}

void synui::MainWindow::_handleEvents()
{
	// check all the window's events that were triggered since the last iteration of the loop
	sf::Event event;
	while (m_window->isOpen() && m_window->pollEvent(event)) {
		if (event.type == sf::Event::Closed)
			m_window->close();

		m_mainGUI->handleEvent(event); // Pass the event to the widgets
	}
}

void synui::MainWindow::_queueInternalMessage(GUIMessage* a_msg) { m_guiInternalMsgQueue.push(a_msg); }

void synui::MainWindow::_flushMessageQueues()
{
	GUIMessage* msg;
	while (m_guiInternalMsgQueue.pop(msg)) { _processMessage(msg); }
	while (m_guiExternalMsgQueue.pop(msg)) { _processMessage(msg); }
}

void synui::MainWindow::_processMessage(GUIMessage* a_msg)
{
	a_msg->action(this, &a_msg->data);
	delete a_msg;
}

void synui::MainWindow::queueExternalMessage(GUIMessage* a_msg) { m_guiExternalMsgQueue.push(a_msg); }

bool synui::MainWindow::OpenWindow(sf::WindowHandle a_system_window)
{
	if (!m_isOpen) {
		m_window->setVisible(true);
		m_window->setActive(true);
		_OpenWindowImplem(a_system_window);
		m_timer.resume();
		m_isOpen = true;
	}
	return true;
}

void synui::MainWindow::CloseWindow()
{
	if (m_isOpen) {
		_CloseWindowImplem();
		m_isOpen = false;
		m_window->setVisible(false);
		m_window->setActive(false);
		m_timer.pause();
	}
}
