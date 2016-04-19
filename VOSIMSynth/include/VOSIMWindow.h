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
*  \file VOSIMWindow.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <thread>
#include <NDPoint.h>

#include "nanovg.h"
#include <UIComponent.h>

using sf::Event;

namespace syn
{
	class VOSIMWindow
	{
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		VOSIMWindow(int a_width, int a_height) :
			m_size(a_width,a_height),
			m_cursor({0,0}),
			m_isClicked(false), 
			m_running(false), 
			m_draggingComponent(nullptr), 
			m_focused(nullptr), 
			m_root(nullptr), 
			m_sfmlWindow(nullptr),
			m_frameCount(0) 
		{	
		}

		virtual ~VOSIMWindow() {
			if (m_sfmlWindow) {
				m_sfmlWindow->close();
				delete m_sfmlWindow;
			}
			if(m_root) {
				delete m_root;
			}
		}

		sf::RenderWindow* GetWindow() const {
			return m_sfmlWindow;
		}

		NDPoint<2, double> size() const {
			return m_size;
		}

		void setFocus(UIComponent* a_comp);
		Vector2i cursorPos() const { return m_cursor; }
		Vector2i diffCursorPos() const {return m_dCursor;}
		const Timestamped<Event::MouseButtonEvent>& lastClickEvent() const { return m_lastClick; }
		const Timestamped<Event::MouseWheelScrollEvent>& lastScrollEvent() const { return m_lastScroll; }

		bool OpenWindow(sf::WindowHandle a_system_window);
		void CloseWindow();
	private:
		/**
		* Creates a child window from the given system window handle.
		* \returns the system window handle for the child window.
		*/
		sf::WindowHandle sys_CreateChildWindow(sf::WindowHandle a_system_window);
		void drawThreadFunc();
		void updateCursorPos(const Vector2i& newPos) {
			m_dCursor = newPos - m_cursor;
			m_cursor = newPos;
		}
	private:
		Vector2i m_size;
		Vector2i m_cursor;
		Vector2i m_dCursor;
		Timestamped<Event::MouseButtonEvent> m_lastClick;
		Timestamped<Event::MouseWheelScrollEvent> m_lastScroll;
	
		bool m_isClicked;
		bool m_running;
		UIComponent* m_draggingComponent;
		UIComponent* m_focused;
		UIComponent* m_root;

		sf::RenderWindow* m_sfmlWindow;
		std::thread m_drawThread;
		unsigned m_frameCount;
	};
}
#endif

