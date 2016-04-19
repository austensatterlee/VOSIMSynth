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

namespace syn {
	class UICircuitPanel : public UIComponent
	{
	public:
		UICircuitPanel(VOSIMWindow* a_window, UIComponent* a_parent, VoiceManager* a_vm, UnitFactory* a_unitFactory) :
			UIComponent{a_window, a_parent},
			m_vm(a_vm),
			m_unitFactory(a_unitFactory)
			{}

	protected:
		void draw(NVGcontext* a_nvg) override;
	private:		
		vector<shared_ptr<UIUnitControlContainer> > m_unitControls;
		VoiceManager* m_vm;
		UnitFactory* m_unitFactory;
	};
}
#endif
