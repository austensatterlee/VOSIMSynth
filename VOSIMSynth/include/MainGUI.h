/*
Copyright 2016, Austen Satterlee

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
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *  \file MainGUI.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once
#include "nanogui/formhelper.h"

namespace nanogui
{
	class Window;
	class Widget;
	class Screen;
}

namespace syn
{
	class VoiceManager;
	class UnitFactory;
}

struct GLFWwindow;

namespace synui
{
	class MainWindow;
	class CircuitWidget;

	class MainGUI
	{
	public:
		MainGUI(MainWindow *a_window, syn::VoiceManager *a_vm, syn::UnitFactory *a_uf);

		void show() const;
		void hide() const;
		void draw();

		nanogui::Screen *getScreen() const { return m_screen; }

		virtual ~MainGUI();

	protected:
		nanogui::Widget *_createUnitSelector(nanogui::Widget *a_widget) const;

	private:
		MainWindow *m_window;
		nanogui::Screen *m_screen;
		syn::VoiceManager *m_vm;
		syn::UnitFactory *m_uf;

		// Widgets
        nanogui::TabWidget *m_tabWidget;
		nanogui::Widget *m_unitSelector, *m_unitEditorHost;
		nanogui::Window *m_sidePanelL, *m_sidePanelR;
		CircuitWidget *m_circuit;
	};
}
