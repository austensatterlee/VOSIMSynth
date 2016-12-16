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
 *  \file UnitWidget.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once
#include <nanogui/nanogui.h>

namespace synui
{
	class CircuitWidget;
}

namespace syn
{
	class VoiceManager;
	class UnitFactory;
}

namespace synui
{
	class UnitWidget : public nanogui::Widget
	{
	public:
		UnitWidget(CircuitWidget* a_parent, int a_unitId);

		void draw(NVGcontext* ctx) override;
		bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;
		bool mouseDragEvent(const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) override;

		Eigen::Vector2i getInputPortPosition(int a_portId);
		Eigen::Vector2i getOutputPortPosition(int a_portId);
	private:
		CircuitWidget* m_parentCircuit;
		Widget* m_titleLabel;
		std::unordered_map<int, Widget*> m_inputLabels;
		std::unordered_map<int, Widget*> m_outputLabels;

		bool m_drag;
		int m_unitId;
	};
}
