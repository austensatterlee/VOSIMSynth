#include "vosimsynth/MainWindow.h"
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

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

static int nGlfwClassReg = 0;

using nlohmann::json;

void synui::MainWindow::_openWindowImplem(HWND a_system_window) {
    TIME_TRACE

    HWND hwnd = glfwGetWin32Window(m_window);
    SetParent(hwnd, a_system_window);
    SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_POPUP | WS_CHILD);
    // Try to force the window to update
    SetWindowPos(hwnd, a_system_window, 0, 0, 0, 0, SWP_NOSIZE);

    m_timerId = SetTimer(hwnd, NULL, 1000 / 120, reinterpret_cast<TIMERPROC>(_timerProc)); // timer callback 
    if (!m_timerId) {
        TRACEMSG("Unable to create timer!");
        throw "Unable to create timer!";
    }
}

void synui::MainWindow::_closeWindowImplem() {
    TIME_TRACE
    if (m_timerId) {
        HWND hwnd = glfwGetWin32Window(m_window);
        KillTimer(hwnd, m_timerId);
        m_timerId = NULL;
    }
}

VOID CALLBACK synui::MainWindow::_timerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime) {
    GLFWwindow* window = static_cast<GLFWwindow*>(GetPropW(hwnd, L"GLFW"));
    MainWindow* _this = static_cast<MainWindow*>(glfwGetWindowUserPointer(window));
    _this->_runLoop();
}
#endif

synui::MainWindow::MainWindow(int a_width, int a_height, GUIConstructor a_guiConstructor)
    :
    m_HInstance(nullptr),
    m_window(nullptr),
    m_size(a_width, a_height),
    m_isOpen(false),
    m_guiConstructor(a_guiConstructor),
    m_gui(nullptr),
    m_guiInternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE),
    m_guiExternalMsgQueue(MAX_GUI_MSG_QUEUE_SIZE) {
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
    _createGlfwWindow();
}

synui::MainWindow::~MainWindow() {
    TIME_TRACE
    if (m_gui) {
        delete m_gui;
        m_gui = nullptr;
    }
    CloseWindow();
    if (!--nGlfwClassReg)
        glfwTerminate();
}

void synui::MainWindow::_createGlfwWindow() {
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
    m_window = glfwCreateWindow(m_size.x(), m_size.y(), "VOSIMSynth", nullptr, nullptr);
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

    // Construct GUI object
    if (!m_gui) {
        m_gui = m_guiConstructor(this);
    } else {
        m_gui->setGLFWWindow(m_window);
    }

#if defined(TRACER_BUILD)
    int glMajorVer = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MAJOR);
    int glMinorVer = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MINOR);
    TRACEMSG("Using OpenGL version: %d.%d", glMajorVer, glMinorVer)
#endif

    resize(m_size.x(), m_size.y());
}

void synui::MainWindow::_runLoop() {
    _flushMessageQueues();
    m_gui->draw();
}

synui::MainWindow::operator json() {
    TIME_TRACE;
    if (m_gui) {
        m_guiState = m_gui->operator json();
    }
    return m_guiState;
}

void synui::MainWindow::load(const json& j) {
    TIME_TRACE
    m_guiState = j;
    if (m_gui) {
        m_gui->load(j);
        m_guiState = m_gui->operator json();
    }
}

void synui::MainWindow::reset() {
    TIME_TRACE
    if (m_gui)
        m_gui->reset();
}

void synui::MainWindow::_flushMessageQueues() {
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

bool synui::MainWindow::queueInternalMessage(syn::Command* a_msg) {
    TIME_TRACE
    if (!m_guiInternalMsgQueue.write_available()) {
        return false;
    }
    m_guiInternalMsgQueue.push(a_msg);
    return true;
}

bool synui::MainWindow::queueExternalMessage(syn::Command* a_msg) {
    TIME_TRACE
    if (!m_guiExternalMsgQueue.write_available()) {
        return false;
    }
    m_guiExternalMsgQueue.push(a_msg);
    return true;
}

bool synui::MainWindow::OpenWindow(HWND a_system_window) {
    TIME_TRACE
    if (!m_isOpen) {
        _createGlfwWindow();
        m_gui->show();
        _openWindowImplem(a_system_window);
        glfwFocusWindow(m_window);
        m_isOpen = true;
    }
    return true;
}

void synui::MainWindow::CloseWindow() {
    TIME_TRACE
    if (m_isOpen) {
        _closeWindowImplem();
        m_isOpen = false;
    }
}

void synui::MainWindow::resize(int w, int h) {
    TIME_TRACE
    if (w < 800 && h < 600)
        return;
    m_size = {w,h};
    m_gui->resize(m_size);
    onResize(w, h);
}
