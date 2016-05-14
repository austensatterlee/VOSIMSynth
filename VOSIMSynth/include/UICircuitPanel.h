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

namespace syn
{
	class UnitFactory;
	class VoiceManager;

	class UICircuitPanel : public UIComponent
	{
		friend VOSIMWindow;
		
	public:
		UICircuitPanel(VOSIMWindow* a_window, VoiceManager* a_vm, UnitFactory* a_unitFactory);
		bool onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		UIComponent* onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
		bool onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) override;
		UIUnitControlContainer* getUnit(const Vector2i& a_pt) const;
		UIUnitControlContainer* findUnit(int a_unitId) const;
		void requestAddConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void requestDeleteConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void requestDeleteUnit(int a_unitId);

		UIWire* getSelectedWire() {
			make_heap(m_wireSelectionQueue.begin(), m_wireSelectionQueue.end(), UIWire::compareByHoverDistance());
			return m_wireSelectionQueue.empty() ? nullptr : m_wireSelectionQueue.front();
		}

		void setSelectedWire(UIWire* a_wire);
		void clearSelectedWire(UIWire* a_wire);

		const vector<UIUnitControlContainer*>& getUnits() const {
			return m_unitControls;
		}

		void reset();
	protected:
		void draw(NVGcontext* a_nvg) override;
		void onAddConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void onDeleteConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void onAddUnit_(unsigned a_classId, int a_unitId);
		void requestAddUnit_(unsigned a_classId);
	private:
		void _onResize() override;
	private:
		vector<UIUnitControlContainer*> m_unitControls;
		vector<UIWire*> m_wires;
		vector<UIWire*> m_wireSelectionQueue;
		VoiceManager* m_vm;
		UnitFactory* m_unitFactory;
		UIUnitSelector* m_unitSelector;
		UIUnitControlContainer* m_inputs;
		UIUnitControlContainer* m_outputs;
	};
}
#endif
