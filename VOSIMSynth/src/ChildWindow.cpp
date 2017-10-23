#include "vosimsynth/ChildWindow.h"
#include "vosimsynth/MainGUI.h"
#include "vosimsynth/Logging.h"
#include "vosimsynth/common.h"
#include <vosimlib/Command.h>
#include <winuser.h>

#if !defined(GL_VERSION_MAJOR)
#define GL_VERSION_MAJOR 3
#endif
#if !defined(GL_VERSION_MINOR)
#define GL_VERSION_MINOR 3
#endif

#if defined(_WIN32)
#include <windows.h>
#include <winuser.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

static int g_nGlfwClassReg = 0;

VOID CALLBACK synui::ChildWindow::_timerProc(HWND a_hwnd, UINT /*message*/, UINT /*idTimer*/, DWORD /*dwTime*/) {
    GLFWwindow* window = static_cast<GLFWwindow*>(GetPropW(a_hwnd, L"GLFW"));
    ChildWindow* _this = static_cast<ChildWindow*>(glfwGetWindowUserPointer(window));
    _this->_flushMessageQueues();
    _this->_runLoop();
}

void synui::ChildWindow::_openWindowImplem(HWND a_system_window) {
    TIME_TRACE

    HWND hwnd = glfwGetWin32Window(m_window);
    SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_POPUP | WS_CHILD);
    SetParent(hwnd, a_system_window);
    // Try to force the window to update
    SetWindowPos(hwnd, a_system_window, 0, 0, 0, 0, SWP_NOSIZE);

    m_timerId = SetTimer(hwnd, NULL, 1000 / 120, reinterpret_cast<TIMERPROC>(_timerProc)); // timer callback 
    if (!m_timerId) {
        TRACEMSG("Unable to create timer!");
        throw "Unable to create timer!";
    }
}

void synui::ChildWindow::_closeWindowImplem() {
    TIME_TRACE
    if (m_timerId) {
        HWND hwnd = glfwGetWin32Window(m_window);
        KillTimer(hwnd, m_timerId);
        m_timerId = NULL;
    }
}
#endif // _WIN32

synui::ChildWindow::ChildWindow(int a_width, int a_height)
    : m_window(nullptr),
      m_isOpen(false),
      m_guiInternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE),
      m_guiExternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE) {
    TIME_TRACE
    auto errorCallback = [](int a_error, const char* a_description)
    {
        TRACEMSG(a_description);
        puts(a_description);
    };
    glfwSetErrorCallback(errorCallback);
    // Create GLFW window
    if (!(g_nGlfwClassReg++) && !glfwInit()) {
        TRACEMSG("Failed to init GLFW.");
        throw "Failed to init GLFW.";
    } else {
        TRACEMSG("Successfully initialized GLFW.");
    }
    _createGlfwWindow(a_width, a_height);
}

synui::ChildWindow::~ChildWindow() {
    TIME_TRACE
    closeWindow();
    if (!--g_nGlfwClassReg)
        glfwTerminate();
}

void synui::ChildWindow::_createGlfwWindow(int a_width, int a_height) {
    TIME_TRACE

#if defined(GL_VERSION_MAJOR)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
#endif
#if defined(GL_VERSION_MINOR)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    m_window = glfwCreateWindow(a_width, a_height, "VOSIMSynth", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        TRACEMSG("Failed to create GLFW window.");
        throw "Failed to create GLFW window.";
    } else {
        TRACEMSG("Successfully created GLFW window.");
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    glfwSwapBuffers(m_window);

#if defined(TRACER_BUILD)
        int glMajorVer = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MAJOR);
        int glMinorVer = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MINOR);
        TRACEMSG("Using OpenGL version: %d.%d", glMajorVer, glMinorVer)
#endif
}

#if defined(_WIN32)
bool synui::ChildWindow::openWindow(HWND a_systemWindow) {
    TIME_TRACE
    if (!m_isOpen) {
        _openWindowImplem(a_systemWindow);
        glfwShowWindow(m_window);
        glfwFocusWindow(m_window);
        m_isOpen = true;
        _onOpen();
    }
    return true;
}
#endif

void synui::ChildWindow::closeWindow() {
    TIME_TRACE
    if (m_isOpen) {
        _closeWindowImplem();
        glfwHideWindow(m_window);
        m_isOpen = false;
        _onClose();
    }
}

void synui::ChildWindow::resize(int a_width, int a_height) {
    _resizeParent(a_width, a_height);
    _resizeSelf(a_width, a_height);
}

bool synui::ChildWindow::queueExternalMessage(syn::Command* a_msg) {
    TIME_TRACE
    if (!m_guiExternalMsgQueue.write_available()) {
        return false;
    }
    m_guiExternalMsgQueue.push(a_msg);
    return true;
}

bool synui::ChildWindow::queueInternalMessage(syn::Command* a_msg) {
    TIME_TRACE
    if (!m_guiInternalMsgQueue.write_available()) {
        return false;
    }
    m_guiInternalMsgQueue.push(a_msg);
    return true;
}

int synui::ChildWindow::getHeight() const {
    int width;
    glfwGetWindowSize(m_window, &width, nullptr);
    return width;
}

int synui::ChildWindow::getWidth() const {
    int height;
    glfwGetWindowSize(m_window, nullptr, &height);
    return height;
}


void synui::ChildWindow::_resizeSelf(int a_width, int a_height) {
    glfwSetWindowSize(m_window, a_width, a_height);
}

#if defined(_WIN32)
void synui::ChildWindow::_resizeParent(int a_width, int a_height) {
    RECT rcClient, rcWindow;
    POINT ptDiff;
    HWND hwnd = glfwGetWin32Window(m_window);
    HWND parent = GetAncestor(hwnd, GA_PARENT);
    GetClientRect(parent, &rcClient);
    GetWindowRect(parent, &rcWindow);
    ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
    SetWindowPos(parent, HWND_TOP, 0, 0, a_width + ptDiff.x, a_height + ptDiff.y, SWP_NOMOVE);
}
#endif

void synui::ChildWindow::_flushMessageQueues() {
    syn::Command* msg;
    while (m_guiInternalMsgQueue.pop(msg)) {
        (*msg)();
        delete msg;

    }
    while (m_guiExternalMsgQueue.pop(msg)) {
        (*msg)();
        delete msg;

    }
}