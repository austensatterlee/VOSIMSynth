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

#include <TGUI/TGUI.hpp>
#include "sftools/ResourceManager/SFMLManagers.hpp"

namespace syn
{
	class VoiceManager;
	class UnitFactory;
}

namespace synui{
	class MainWindow;

	class MainGUI
	{
	private:
		MainWindow& m_window;
		tgui::Gui m_gui;
		tgui::Theme::Ptr m_theme;
		syn::VoiceManager* m_vm;
		syn::UnitFactory* m_uf;

		// resource managers
		std::map<std::string, sf::Font> m_fonts;
		std::map<std::string,sf::Texture> m_textures;
		std::map<std::string, sf::Image> m_images;

		// state
		std::pair<std::string, std::string> m_unitToCreate;

	public:
		tgui::MenuBar::Ptr createUnitSelector();
		tgui::Panel::Ptr createCircuitController();
		tgui::Panel::Ptr createUnitEditor();

		MainGUI(MainWindow& a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf);

		void draw();
		void handleEvent(sf::Event& event)
		{
			m_gui.handleEvent(event);
		}
	};
}