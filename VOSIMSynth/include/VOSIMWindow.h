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

//#define DRAW_COMPONENT_BOUNDS

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <NDPoint.h>

#include "nanovg.h"
#include <UnitFactory.h>
#include <VoiceManager.h>
#include <Theme.h>
#include <perf.h>
#include <sftools/Chronometer.hpp>
#include <mutex>

using sf::Event;

namespace syn
{
	class UIComponent;

	class UICircuitPanel;

	template <typename T>
	struct Timestamped
	{
		T data;
		sf::Time time;
	};

	class VOSIMWindow
	{
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		VOSIMWindow(int a_width, int a_height, VoiceManager* a_vm, UnitFactory* a_unitFactory) : 
			m_hinstance(0),
			m_childHandle1(nullptr),
			m_childHandle2(nullptr),
			m_size(a_width,a_height),
			m_cursor({0,0}),
			m_isClicked(false), 
			m_running(false), 
			m_isInitialized(false),
			m_draggingComponent(nullptr), 
			m_focused(nullptr), 
			m_root(nullptr), 
			m_sfmlWindow(nullptr),
			m_frameCount(0),
			m_vm(a_vm),
			m_unitFactory(a_unitFactory),
			m_circuitPanel(nullptr), 
			m_vg(nullptr),
			m_view(0,0,a_width,a_height)
		{
		}

		virtual ~VOSIMWindow();

		sf::RenderWindow* GetWindow() const {
			return m_sfmlWindow;
		}

		Vector2i getSize() const {
			return m_size;
		}

		void setFocus(UIComponent* a_comp);
		Vector2i cursorPos() const;

		Vector2i diffCursorPos() const;

		const Timestamped<Event::MouseButtonEvent>& lastClickEvent() const { return m_lastClick; }
		const Timestamped<Event::MouseWheelScrollEvent>& lastScrollEvent() const { return m_lastScroll; }
		shared_ptr<Theme> theme() const { return m_theme; }
		UICircuitPanel* getCircuitPanel() const { return m_circuitPanel; }
		UnitFactory* getUnitFactory() const { return m_unitFactory; }
		UIComponent* getFocused() const { return m_focused; }
		void clearFocus() { setFocus(nullptr); }

		Vector2i toWorldCoords(const Vector2i& a_pix) const;
		Vector2i toPixelCoords(const Vector2i& a_world) const;

		void setViewSize(const Vector2i& a_size);
		void setViewPos(const Vector2i& a_size);
		Vector2i getViewSize() const;

		bool OpenWindow(sf::WindowHandle a_system_window);
		void CloseWindow();

		void save(ByteChunk* a_data) const;
		int  load(ByteChunk* a_data, int startPos);

		bool isInitialized() const { return m_isInitialized; }

		void reset();

#ifdef _WIN32
		static LRESULT CALLBACK drawFunc(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam);
	public:
		HINSTANCE m_hinstance;
#endif
	private:
		/**
		* Creates a child window from the given system window handle.
		* \returns the system window handle for the child window.
		*/
		sf::WindowHandle sys_CreateChildWindow(sf::WindowHandle a_system_window);
		void updateCursorPos(const Vector2i& newPos);
	private:
		sf::WindowHandle m_childHandle1, m_childHandle2;
		Vector2i m_size;
		Vector2i m_cursor;
		Vector2i m_dCursor;
		Timestamped<Event::MouseButtonEvent> m_lastClick;
		Timestamped<Event::MouseWheelScrollEvent> m_lastScroll;
	
		bool m_isClicked;
		bool m_running;
		bool m_isInitialized;
		UIComponent* m_draggingComponent;
		UIComponent* m_focused;
		UIComponent* m_root;

		sf::RenderWindow* m_sfmlWindow;
		unsigned m_frameCount;

		VoiceManager* m_vm;
		UnitFactory* m_unitFactory;

		UICircuitPanel* m_circuitPanel;

		shared_ptr<Theme> m_theme;

		PerfGraph m_fpsGraph, m_cpuGraph;
		sftools::Chronometer m_timer;

		NVGcontext* m_vg;

		Vector4i m_view;
		double m_zoom = 1.0;		
	};
}
#endif
