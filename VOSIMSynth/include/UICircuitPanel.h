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
*  \file UICircuitPanel.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UICIRCUITPANEL__
#define __UICIRCUITPANEL__
#include "UIComponent.h"
#include "UIWire.h"

#include <queue>
#include <map>



namespace synui
{
	class UICircuitPanel : public UIComponent
	{
		friend class VOSIMComponent;
	public:
		typedef function<UIUnitContainer*(MainWindow*, UICircuitPanel*, syn::VoiceManager*, UIUnitControl*)> UnitContainerConstructor;
		typedef function<UIUnitControl*(MainWindow*, syn::VoiceManager*, int)> UnitControllerConstructor;
	public:
		UICircuitPanel(MainWindow* a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_unitFactory, UIUnitSelector* a_unitSelector, UIControlPanel* a_controlPanel);
		UIUnitContainer* findUnitContainer(const UICoord& a_pt) const;
		UIUnitContainer* findUnitContainer(int a_unitId) const;
		void requestAddConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void requestDeleteConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void requestMoveConnection(UIWire* a_uiWire, int a_unitId, int a_portId, int a_toUnit, int a_toPort);
		void requestDeleteUnit(int a_unitId);

		UIWire* getSelectedWire();

		void setSelectedWire(UIWire* a_wire);
		void clearSelectedWire(UIWire* a_wire);

		const vector<UIUnitContainer*>& getUnits() const {
			return m_unitContainers;
		}

		void reset(); 

		UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		bool onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;

		template <typename UnitType>
		void registerUnitControl(UnitControllerConstructor a_unitControlConstructor);
		template <typename UnitType>
		void registerUnitContainer(UnitContainerConstructor a_unitContainerConstructor);

	protected:
		void draw(NVGcontext* a_nvg) override;
		void setChildrenStyles(NVGcontext* a_nvg) override;
		void onAddConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void onDeleteConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void onAddUnit_(unsigned a_classId, int a_unitId);
		void requestAddUnit_(unsigned a_classId);
		UIUnitContainer* createUnitContainer_(unsigned a_classId, int a_unitId);
	private:
		void _onResize() override;	
	private:
		vector<UIUnitContainer*> m_unitContainers;
		vector<UIWire*> m_wires;
		vector<UIWire*> m_wireSelectionQueue;
		syn::VoiceManager* m_vm;
		syn::UnitFactory* m_unitFactory;
		UIUnitSelector* m_unitSelector;
		UIControlPanel* m_unitControlPanel;
		UIUnitContainer* m_inputs;
		UIUnitContainer* m_outputs;

		map<unsigned, UnitControllerConstructor> m_unitControlMap;
		map<unsigned, UnitContainerConstructor> m_unitContainerMap;
	};

	template <typename UnitType>
	void UICircuitPanel::registerUnitControl(UnitControllerConstructor a_unitControlConstructor) {
		unsigned unitClassId = UnitType("").getClassIdentifier();
		m_unitControlMap[unitClassId] = a_unitControlConstructor;
	}

	template <typename UnitType>
	void UICircuitPanel::registerUnitContainer(UnitContainerConstructor a_unitContainerConstructor) {
		unsigned unitClassId = UnitType("").getClassIdentifier();
		m_unitContainerMap[unitClassId] = a_unitContainerConstructor;
	}
}
#endif
