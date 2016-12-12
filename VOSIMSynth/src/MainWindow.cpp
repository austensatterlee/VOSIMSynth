#include "MainWindow.h"

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

static int nWndClassReg = 0;
static const char* wndClassName = "VOSIMWndClass";

void synui::MainWindow::_OpenWindowImplem(HWND a_system_window)
{
    if (nWndClassReg++ == 0) {
        WNDCLASS wndClass = {NULL, drawFunc, 0, 0, m_HInstance, 0, NULL, 0, 0, wndClassName};
        RegisterClass(&wndClass);
    }

    SetWindowLongW(glfwGetWin32Window(m_window), GWL_STYLE, GetWindowLongW(glfwGetWin32Window(m_window), GWL_STYLE) | WS_CHILD);
    SetParent(glfwGetWin32Window(m_window), a_system_window);

    m_timerWindow = CreateWindow(wndClassName, "VOSIMTimerWindow", NULL, 0, 0, 0, 0, NULL, NULL, m_HInstance, this);

    if (!m_timerWindow && --nWndClassReg == 0) {
        UnregisterClass(wndClassName, m_HInstance);
    }
}

void synui::MainWindow::_CloseWindowImplem()
{
    SetWindowLongW(glfwGetWin32Window(m_window), GWL_STYLE, GetWindowLongW(glfwGetWin32Window(m_window), GWL_STYLE) & ~WS_CHILD);

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

        int mSec = int(1000.0 / 120.0);
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

synui::MainWindow::MainWindow(int a_width, int a_height, GUIConstructor a_guiConstructor) :
    m_HInstance(nullptr),
    m_timerWindow(nullptr), 
    m_screen(nullptr),
    m_window(nullptr),
    m_isOpen(false),
    m_size(a_width, a_height), 
    m_frameCount(0),
    m_fps(1.0),
    m_guiInternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE),
    m_guiExternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE)
{
    if (!glfwInit()) {
        throw "Failed to init GLFW.";
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);


    m_window = glfwCreateWindow(m_size.x(), m_size.y(), "VOSIMSynth", nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        throw "Failed to create GLFW window";
    }
    glfwSetWindowUserPointer(m_window, this);
    glfwMakeContextCurrent(m_window);

    m_screen = new nanogui::Screen();
    m_screen->initialize(m_window, true); 

    glViewport(0, 0, m_size.x(), m_size.y());
    glfwSwapInterval(0);
    glfwSwapBuffers(m_window);

    nanogui::Window *window = new nanogui::Window(m_screen, "Grid of small widgets");
    window->setPosition(Vector2i(425, 300));
	window->setFixedSize(Vector2i(0,0));
    nanogui::GridLayout *layout =
        new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2,
                nanogui::Alignment::Middle, 15, 5);
    layout->setColAlignment(
            {nanogui::Alignment::Maximum, nanogui::Alignment::Fill });
    layout->setSpacing(0, 10);
    window->setLayout(layout);

    {
        nanogui::Label *lbl = new nanogui::Label(window, "Floating point :", "sans-bold");
		lbl->setFixedSize({0,0});
        nanogui::TextBox *textBox = new nanogui::TextBox(window);
		textBox->setFixedSize(Vector2i(0,0));
        textBox->setEditable(true);
        textBox->setFixedSize(Vector2i(100, 20));
        textBox->setValue("50");
        textBox->setUnits("GiB");
        textBox->setDefaultValue("0.0");
        textBox->setFontSize(16);
        textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");
    }  
    {
        nanogui::Label *lbl = new nanogui::Label(window, "Checkbox :", "sans-bold");
		lbl->setFixedSize({0,0});
        nanogui::CheckBox *cb = new nanogui::CheckBox(window, "Check me");
        cb->setFontSize(16);
        cb->setChecked(true);
    }

    m_screen->setVisible(true);
    m_screen->performLayout();
	m_screen->setPosition({0,0});

    glfwSetCursorPosCallback(m_window,
            [](GLFWwindow *w, double x, double y) {
            static_cast<MainWindow*>(glfwGetWindowUserPointer(w))->m_screen->cursorPosCallbackEvent(x, y);
            }
            );

    glfwSetMouseButtonCallback(m_window,
            [](GLFWwindow *w, int button, int action, int modifiers) {
            static_cast<MainWindow*>(glfwGetWindowUserPointer(w))->m_screen->mouseButtonCallbackEvent(button, action, modifiers);
            }
            );

    glfwSetKeyCallback(m_window,
            [](GLFWwindow *w, int key, int scancode, int action, int mods) {
            static_cast<MainWindow*>(glfwGetWindowUserPointer(w))->m_screen->keyCallbackEvent(key, scancode, action, mods);
            }
            );

    glfwSetCharCallback(m_window,
            [](GLFWwindow *w, unsigned int codepoint) {
            static_cast<MainWindow*>(glfwGetWindowUserPointer(w))->m_screen->charCallbackEvent(codepoint);
            }
            );

    glfwSetDropCallback(m_window,
            [](GLFWwindow *w, int count, const char **filenames) {
            static_cast<MainWindow*>(glfwGetWindowUserPointer(w))->m_screen->dropCallbackEvent(count, filenames);
            }
            );

    glfwSetScrollCallback(m_window,
            [](GLFWwindow *w, double x, double y) {
            static_cast<MainWindow*>(glfwGetWindowUserPointer(w))->m_screen->scrollCallbackEvent(x, y);
            }
            );

    glfwSetFramebufferSizeCallback(m_window,
            [](GLFWwindow *w, int width, int height) {
            static_cast<MainWindow*>(glfwGetWindowUserPointer(w))->m_screen->resizeCallbackEvent(width, height);
            }
            );

    glfwMakeContextCurrent(nullptr);
}

synui::MainWindow::~MainWindow()
{
    CloseWindow();
}

void synui::MainWindow::_runLoop()
{
    glfwSetTime(0);
    double cpuStartTime = glfwGetTime();

    _flushMessageQueues();

    glfwMakeContextCurrent(m_window);   
    glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_screen->drawContents();
    m_screen->drawWidgets();
    glfwSwapBuffers(m_window);
    glfwMakeContextCurrent(nullptr);
    m_frameCount++;

    double dCpuTime = glfwGetTime() - cpuStartTime;
    double fps = 1. / dCpuTime;
    m_fps = m_fps + 0.0175 * (fps - m_fps);
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

bool synui::MainWindow::OpenWindow(HWND a_system_window)
{
    if (!m_isOpen) {
        glfwShowWindow(m_window);
        _OpenWindowImplem(a_system_window);
        m_isOpen = true;
    }
    return true;
}

void synui::MainWindow::CloseWindow()
{
    if (m_isOpen) {
        _CloseWindowImplem();
        glfwHideWindow(m_window);
        m_isOpen = false;
    }
}
