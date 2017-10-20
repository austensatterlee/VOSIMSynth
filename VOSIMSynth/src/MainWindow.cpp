#include "vosimsynth/MainWindow.h"
#include "vosimsynth/MainGUI.h"
#include "vosimsynth/Logging.h"
#include "vosimsynth/common.h"
#include <vosimlib/Command.h>
#include <winuser.h>
#include <winuser.h>
#include <winuser.h>

#if !defined(GL_VERSION_MAJOR)
#define GL_VERSION_MAJOR 3
#endif
#if !defined(GL_VERSION_MINOR)
#define GL_VERSION_MINOR 3
#endif

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
    TIME_TRACE

    HWND hwnd = glfwGetWin32Window(m_window);
    SetParent(hwnd, a_system_window);
    SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_POPUP | WS_CHILD);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_APPWINDOW);
    SetWindowPos(hwnd, a_system_window, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

    m_timerId = SetTimer(hwnd, NULL, 1000/120, reinterpret_cast<TIMERPROC>(_TimerProc)); // timer callback 
    if (!m_timerId) {
        TRACEMSG("No timer is available");
        throw "No timer is available";
    }
}

void synui::MainWindow::_CloseWindowImplem()
{
    TIME_TRACE
    if (m_timerId) {
        HWND hwnd = glfwGetWin32Window(m_window);
        KillTimer(hwnd, m_timerId);
        m_timerId = NULL;
    }
}

VOID CALLBACK synui::MainWindow::_TimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime)
{
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(GetPropW(hwnd, L"GLFW"));
    MainWindow* _this = reinterpret_cast<MainWindow*>(glfwGetWindowUserPointer(window));
    _this->_runLoop();
}
#endif

synui::MainWindow::MainWindow(int a_width, int a_height, GUIConstructor a_guiConstructor) :
    m_HInstance(nullptr),
    m_window(nullptr),
    m_size(a_width, a_height), 
    m_isOpen(false),
    m_guiConstructor(a_guiConstructor),
    m_gui(nullptr),
    m_guiInternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE),
    m_guiExternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE)
{
    TIME_TRACE
    auto error_callback = [](int error, const char* description)
            {
                TRACEMSG(description);
                puts(description);
            };
    glfwSetErrorCallback(error_callback);  
    // Create GLFW window
    if (!(nGlfwClassReg++) && !glfwInit()) {
        TRACEMSG("Failed to init GLFW.");
        throw "Failed to init GLFW.";
    } else {
        TRACEMSG("Successfully initialized GLFW.");
    }
    _createGLFWWindow();
}

synui::MainWindow::~MainWindow()
{
    TIME_TRACE
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
    TIME_TRACE 

#if defined(GL_VERSION_MAJOR)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
#endif
#if defined(GL_VERSION_MINOR)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
#endif
//#if !defined(NDEBUG)
//    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
//#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    if(m_window) {
        glfwDestroyWindow(m_window);
    }
    m_window = glfwCreateWindow(m_size.x(), m_size.y(), "VOSIMSynth", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        TRACEMSG("Failed to create GLFW window.");
        throw "Failed to create GLFW window.";
    }
    else
    {
        TRACEMSG("Successfully created GLFW window.");
    }

    glfwSetWindowUserPointer(m_window, this);
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
}

void synui::MainWindow::_runLoop()
{
    _flushMessageQueues();
    m_gui->draw();
}

synui::MainWindow::operator json()
{
    TIME_TRACE;
    if (m_gui)
    {
        m_guiState = m_gui->operator json();
    }
    return m_guiState;
}

void synui::MainWindow::load(const json& j)
{
    TIME_TRACE
    m_guiState = j;
    if (m_gui) {
        m_gui->load(j);
        m_guiState = m_gui->operator json();
    }
}

void synui::MainWindow::reset()
{
    TIME_TRACE
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
    TIME_TRACE
    if (!m_isOpen) {
        _createGLFWWindow();
        m_gui->show();
        _OpenWindowImplem(a_system_window);
        glfwFocusWindow(m_window);
        m_isOpen = true;
    }
    return true;
}

void synui::MainWindow::CloseWindow()
{
    TIME_TRACE
    if (m_isOpen) {
        _CloseWindowImplem();
        m_isOpen = false;
    }
}

void synui::MainWindow::resize(int w, int h)
{
    TIME_TRACE
    w = syn::MAX(w, 800);
    h = syn::MAX(h, 600);
    glfwSetWindowSize(m_window, w, h);
    m_gui->resize(w,h);
    m_size = { w,h };
    onResize(w,h);
}
