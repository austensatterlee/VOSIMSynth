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

#include <SFML/Graphics/RenderWindow.hpp>
#include <sftools/Chronometer.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/policies.hpp>

#include "UI.h"

#define MAX_GUI_MSG_QUEUE_SIZE 64

using boost::lockfree::spsc_queue;
using boost::lockfree::capacity;

namespace synui
{
	class MainWindow;
	class MainGUI;

	struct GUIMessage
	{
		void(*action)(MainWindow*, ByteChunk*);
		ByteChunk data;
	};

	class MainWindow
	{
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		typedef std::function<MainGUI*(MainWindow&)> GUIConstructor; 

	public:

		MainWindow(int a_width, int a_height, GUIConstructor a_guiConstructor);
		virtual ~MainWindow();

		std::shared_ptr<sf::RenderWindow> getWindow() const { return m_window; }

		bool isOpen() const { return m_isOpen; }

		Vector2u getSize() const { return m_size; }

		double fps() const { return m_fps; }
		
		bool OpenWindow(sf::WindowHandle a_system_window);

		void CloseWindow();

		void queueExternalMessage(GUIMessage* a_msg);

#ifdef _WIN32
		void setHInstance(HINSTANCE a_newHInstance) { m_HInstance = a_newHInstance; }
		HINSTANCE getHInstance() const { return m_HInstance; }
		static LRESULT CALLBACK drawFunc(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam);
#endif
	private:
		/**
		 * Attach the sfml window to the given parent window.
		 */
		void _OpenWindowImplem(sf::WindowHandle a_system_window);
		void _CloseWindowImplem();
		
		void _runLoop();
		void _handleEvents();

		void _queueInternalMessage(GUIMessage* a_msg);
		void _flushMessageQueues();
		void _processMessage(GUIMessage* a_msg);
	private:
#ifdef _WIN32
		HINSTANCE m_HInstance;
#endif
		sf::WindowHandle m_timerWindow;
		std::shared_ptr<sf::RenderWindow> m_window;
		std::shared_ptr<MainGUI> m_mainGUI;
		bool m_isOpen;

		Vector2u m_size;

		sftools::Chronometer m_timer;
		unsigned m_frameCount;
		double m_fps;

		spsc_queue<GUIMessage*> m_guiInternalMsgQueue;
		spsc_queue<GUIMessage*> m_guiExternalMsgQueue;
	};
}
#endif
