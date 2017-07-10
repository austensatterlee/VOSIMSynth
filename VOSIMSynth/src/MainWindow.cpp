#include "MainWindow.h"
#include "MainGUI.h"

#include <Command.h>
#include <GLFW/glfw3.h>
#include <IPlug/Log.h>

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

static int nWndClassReg = 0;
static int nGlfwClassReg = 0;
static const char* wndClassName = "VOSIMWndClass";

void synui::MainWindow::_OpenWindowImplem(HWND a_system_window)
{
    TRACE
    if (nWndClassReg++ == 0) {
        WNDCLASS wndClass = { NULL, drawFunc, 0, 0, m_HInstance, 0, NULL, 0, 0, wndClassName };
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
    TRACE
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

    MainWindow *_this = reinterpret_cast<MainWindow*>(GetWindowLongPtr(Handle, GWLP_USERDATA));
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
    m_window(nullptr),
    m_size(a_width, a_height), 
    m_isOpen(false),
    m_guiConstructor(a_guiConstructor),
    m_gui(nullptr),
    m_guiInternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE),
    m_guiExternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE)
{
    TRACE
    auto error_callback = [](int error, const char* description)
            {
                TRACEMSG(description);
                puts(description);
            };
    glfwSetErrorCallback(error_callback);
    // Create GLFW window
    if (!(nGlfwClassReg++) && !glfwInit())
    {
        TRACEMSG("Failed to init GLFW.");
        throw "Failed to init GLFW.";
    }
    else
    {
        TRACEMSG("Successfully initialized GLFW.");        
    }
    _createGLFWWindow();
}

synui::MainWindow::~MainWindow()
{
    TRACE
    if(m_gui)
    {
        delete m_gui; m_gui = nullptr;
    }
    CloseWindow();
    if(!--nGlfwClassReg)
        glfwTerminate();
}

void synui::MainWindow::_createGLFWWindow()
{
    TRACE
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_size.x(), m_size.y(), "VOSIMSynth", nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        TRACEMSG("Failed to create GLFW window.");
        throw "Failed to create GLFW window.";
    }
    else
    {
        TRACEMSG("Successfully created GLFW window.");
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    glfwSwapBuffers(m_window);

    // Construct GUI object
    if(!m_gui){
        m_gui = m_guiConstructor(this);
    }else {
        m_gui->setGLFWWindow(m_window);        
    }

#if defined(TRACER_BUILD)
    int glMajorVer = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MAJOR);
    int glMinorVer = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MINOR);
    TRACEMSG("Using OpenGL version: %d.%d", glMajorVer, glMinorVer)
#endif

    resize(m_size.x(), m_size.y());

    glfwMakeContextCurrent(nullptr);
}

void synui::MainWindow::_runLoop()
{
    _flushMessageQueues();
    m_gui->draw();
}

synui::MainWindow::operator json()
{
    TRACE;
    if (m_gui)
    {
        m_guiState = m_gui->operator json();
    }
    return m_guiState;
}

void synui::MainWindow::load(const json& j)
{
    TRACE
    m_guiState = j;
    if (m_gui)
        m_gui->load(j);
}

void synui::MainWindow::reset()
{
    TRACE
    if (m_gui)
        m_gui->reset();
}

void synui::MainWindow::_flushMessageQueues()
{
    syn::Command* msg;
    while (m_guiInternalMsgQueue.pop(msg))
    {
        (*msg)();
        delete msg;
    }
    while (m_guiExternalMsgQueue.pop(msg))
    {
        (*msg)();
        delete msg;
    }
}

bool synui::MainWindow::queueInternalMessage(syn::Command* a_msg) {
    if (!m_guiInternalMsgQueue.write_available()) {
        return false;
    }
    m_guiInternalMsgQueue.push(a_msg);
    return true;
}

bool synui::MainWindow::queueExternalMessage(syn::Command* a_msg) {
    if (!m_guiExternalMsgQueue.write_available()) {
        return false;
    }
    m_guiExternalMsgQueue.push(a_msg);
    return true;
}

bool synui::MainWindow::OpenWindow(HWND a_system_window)
{
    TRACE
    if (!m_isOpen) {
        _createGLFWWindow();
        m_gui->show();
        _OpenWindowImplem(a_system_window);
        m_isOpen = true;
    }
    return true;
}

void synui::MainWindow::CloseWindow()
{
    TRACE
    if (m_isOpen) {
        glfwDestroyWindow(m_window);
        _CloseWindowImplem();
        m_isOpen = false;
    }
}

void synui::MainWindow::resize(int w, int h)
{
    TRACE
    w = syn::MAX(w, 800);
    h = syn::MAX(h, 600);
    glfwSetWindowSize(m_window, w, h);
    m_gui->resize(w,h);
    m_size = { w,h };
    onResize(w,h);
}
