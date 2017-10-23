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

#pragma once

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
using SystemHandle = HWND;
#endif

#include <boost/lockfree/spsc_queue.hpp>

namespace syn {
    class Command;
}

struct GLFWwindow;

namespace synui {
    /**
     * Handles creation, event loop, and destruction of a system child window.
     */
    class ChildWindow {
    public:
        ChildWindow(int a_width, int a_height);
        virtual ~ChildWindow();

        /// Open the system window
        bool openWindow(SystemHandle a_systemWindow);
        /// Close the system window (the GUI is preserved)
        void closeWindow();
        /// Resize both the child and parent window
        void resize(int a_width, int a_height);
        /// Queue a task (to be called only from the real-time thread)
        bool queueExternalMessage(syn::Command* a_msg);
        /// Queue a task (to be called only from the GUI thread)
        bool queueInternalMessage(syn::Command* a_msg);
        int getHeight() const;
        int getWidth() const;

    protected:
        GLFWwindow* getGlfwWindow_() const { return m_window; };
    private:
#ifdef _WIN32
        static VOID CALLBACK _timerProc(HWND a_hwnd, UINT message, UINT idTimer, DWORD dwTime);
#endif
        void _openWindowImplem(SystemHandle a_systemWindow);
        void _closeWindowImplem();
        void _createGlfwWindow(int a_width, int a_height);
        void _resizeSelf(int a_width, int a_height);
        void _resizeParent(int a_width, int a_height);
        void _flushMessageQueues();
        /// Called on every tick from the OS timer
        virtual void _runLoop() = 0;
        virtual void _onOpen() {};
        virtual void _onClose() {};

    private:
#ifdef _WIN32
        UINT m_timerId;
#endif
        GLFWwindow* m_window;
        bool m_isOpen;

        boost::lockfree::spsc_queue<syn::Command*> m_guiInternalMsgQueue;
        boost::lockfree::spsc_queue<syn::Command*> m_guiExternalMsgQueue;
    };
}
