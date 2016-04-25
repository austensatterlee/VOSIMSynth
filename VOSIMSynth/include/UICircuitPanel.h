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
	class UICircuitPanel : public UIComponent
	{
	public:
		UICircuitPanel(VOSIMWindow* a_window, VoiceManager* a_vm, UnitFactory* a_unitFactory);
		bool onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) override;
		UIUnitControlContainer* getUnit(const Vector2i& a_pt) const;
		UIUnitControlContainer* findUnit(int a_unitId) const;
		void requestAddConnection(int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort);
		void onResize() override;
	protected:
		void draw(NVGcontext* a_nvg) override;

		void onAddUnit_(unsigned a_classId, int a_unitId);
		void requestAddUnit_(unsigned a_classId);
	private:		
		vector<UIUnitControlContainer*> m_unitControls;
		VoiceManager* m_vm;
		UnitFactory* m_unitFactory;
		UIUnitSelector* m_unitSelector;
		UIUnitControlContainer* m_inputs;
		UIUnitControlContainer* m_outputs;
	};
}
#endif
