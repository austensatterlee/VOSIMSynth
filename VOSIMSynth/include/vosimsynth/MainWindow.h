/*
This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2016, Austen Satterlee
*/

/**
*  \file MainWindow.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include "vosimsynth/UI.h"
#include "vosimsynth/Signal.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>
#include <json/json.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define MAX_GUI_MSG_QUEUE_SIZE 64

namespace syn {
    class Command;
}

using boost::lockfree::spsc_queue;
using boost::lockfree::capacity;

struct GLFWwindow;

namespace synui
{
    class MainWindow;
    class MainGUI;

    /**
     * Handles creation and management of the system window, and provides an
     * interface for safely sending commands to the GUI from another thread.
     */
    class MainWindow
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        typedef std::function<MainGUI*(MainWindow*)> GUIConstructor; 

    public:
        MainWindow(int a_width, int a_height, GUIConstructor a_guiConstructor);
        virtual ~MainWindow();

        GLFWwindow* getWindow() const { return m_window; }
        bool isOpen() const { return m_isOpen; }
        Vector2i getSize() const { return m_size; }
        MainGUI* getGUI() const { return m_gui; }
        
        /// Open the system window
        bool OpenWindow(HWND a_system_window);
        /// Close the system window (the GUI is preserved)
        void CloseWindow();

        /// Resize the system window and the GUI
        void resize(int w, int h);
        /// Queue a task (to be called only from the real-time thread)
        bool queueExternalMessage(syn::Command* a_msg);
        /// Queue a task (to be called only from the GUI thread)
        bool queueInternalMessage(syn::Command* a_msg);

        /// Serialize GUI
        operator nlohmann::json();
        /// Deserialize GUI
        void load(const nlohmann::json& j);
        /// Reset the GUI to its initial state
        void reset();

        Signal<int, int> onResize;

#ifdef _WIN32
        HINSTANCE m_HInstance;
        UINT m_timerId;
        void setHInstance(HINSTANCE a_newHInstance) { m_HInstance = a_newHInstance; }
        HINSTANCE getHInstance() const { return m_HInstance; }
        static VOID CALLBACK _TimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);
        void _OpenWindowImplem(HWND a_system_window);
        void _CloseWindowImplem();
#endif
    private:
        void _createGLFWWindow();        
        void _runLoop();
        void _flushMessageQueues();

    private:
        GLFWwindow* m_window;
        Vector2i m_size;
        bool m_isOpen;

        GUIConstructor m_guiConstructor;
        MainGUI* m_gui;
        nlohmann::json m_guiState;

        spsc_queue<syn::Command*> m_guiInternalMsgQueue;
        spsc_queue<syn::Command*> m_guiExternalMsgQueue;
    };
}
#endif
