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
#include "nanovg.h"
#include <UIComponent.h>
#include <UnitFactory.h>
#include <VoiceManager.h>
#include "UIUnitControlContainer.h"
#include "UIUnitSelector.h"

namespace syn {

	class UIWire : public UIComponent
	{
	public:
		UIWire(VOSIMWindow* a_window, int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort)
			: UIComponent{a_window}, m_fromUnit(a_fromUnit), m_fromPort(a_fromPort), m_toUnit(a_toUnit), m_toPort(a_toPort) {}

		bool contains(const Vector2i& a_pt) override;
		bool onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseEnter(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering) override;
		bool onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		int fromUnit() const { return m_fromUnit; }
		int fromPort() const { return m_fromPort; }
		int toUnit() const { return m_toUnit; }
		int toPort() const { return m_toPort; }
		Vector2i fromPt() const;
		Vector2i toPt() const;
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		int m_fromUnit, m_fromPort, m_toUnit, m_toPort;
		bool m_isDragging = false;
		bool m_isDraggingInput = false;
	};

	class UICircuitPanel : public UIComponent
	{
		friend VOSIMWindow;
	public:
		UICircuitPanel(VOSIMWindow* a_window, VoiceManager* a_vm, UnitFactory* a_unitFactory);
		bool onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) override;
		UIUnitControlContainer* getUnit(const Vector2i& a_pt) const;
		UIUnitControlContainer* findUnit(int a_unitId) const;
		void requestAddConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void requestDeleteConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);	
		void onResize() override;
		UIWire* getSelectedWire() const { return m_selectedWire; }
		void setSelectedWire(UIWire* a_wire);
		void clearSelectedWire(UIWire* a_wire);
		const vector<UIUnitControlContainer*>& getUnits() const { return m_unitControls; }		
		void reset();
	protected:
		void draw(NVGcontext* a_nvg) override;
		void onAddConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void onDeleteConnection_(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void onAddUnit_(unsigned a_classId, int a_unitId);
		void requestAddUnit_(unsigned a_classId);
	private:		
		vector<UIUnitControlContainer*> m_unitControls;
		vector<UIWire*> m_wires;
		UIWire* m_selectedWire;
		VoiceManager* m_vm;
		UnitFactory* m_unitFactory;
		UIUnitSelector* m_unitSelector;
		UIUnitControlContainer* m_inputs;
		UIUnitControlContainer* m_outputs;
	};
}
#endif
