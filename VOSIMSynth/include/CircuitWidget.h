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
 *  \file CircuitWidget.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once
#include <nanogui/nanogui.h>

namespace syn
{
	class VoiceManager;
	class UnitFactory;
	class Unit;
}

namespace synui
{
	class MainWindow;
	class UnitWidget;
}

namespace synui
{
	class CircuitWire;

	class CircuitWidget : public nanogui::Widget
	{
		friend class UnitWidget;
		typedef std::pair<int,int> Port;
	public:
		CircuitWidget(nanogui::Widget* a_parent, synui::MainWindow* a_mainWindow, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf);

		bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;

		void draw(NVGcontext* ctx) override;

		void loadPrototype(const std::string& a_unitPrototype);

		Eigen::Vector2i fixToGrid(const Eigen::Vector2i& a_pixelLocation) const;

		syn::VoiceManager* voiceManager() const { return m_vm; }

		int gridSize() const { return m_gridSize; }

	protected:

		void createUnit_(const std::string& a_unitPrototypeName);
		void onUnitCreated_(int a_unitId);

		void createConnection_(const Port& a_inputPort, const Port& a_outputPort);
		void onConnectionCreated_(const Port& a_inputPort, const Port& a_outputPort);

		void startWireDraw_(int a_unitId, int a_portId, bool a_isOutput);
		void endWireDraw_(int a_unitId, int a_portId, bool a_isOutput);

	private:
		synui::MainWindow* m_window; // Allows queueing return messages to the GUI thread
		syn::UnitFactory* m_uf;
		syn::VoiceManager* m_vm;
		int m_gridSize;

		enum class State
		{
			Idle,
			PlacingUnit,
			DrawingWire
		} m_state;

		struct PlacingUnitState
		{
			int unitId;
			synui::UnitWidget* widget;
		} m_placingUnitState;
		std::unordered_map<int, UnitWidget*> m_unitWidgets;

		struct DrawingWireState
		{
			Port inputPort;
			Port outputPort;
			bool startedFromOutput;			
			CircuitWire* wire;
		} m_drawingWireState;
		std::vector<CircuitWire*> m_wires;

	};
}
