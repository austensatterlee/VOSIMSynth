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
*  \file UIUnitControlContainer.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIUNITCONTROLCONTAINER__
#define __UIUNITCONTROLCONTAINER__
#include "UIComponent.h"
#include <VoiceManager.h>
#include "UIUnitControl.h"

namespace syn
{
	class UIUnitControlContainer : public UIComponent
	{
	public:
		UIUnitControlContainer(VOSIMWindow* a_window, UIComponent* a_parent, VoiceManager* a_vm, int a_unitId, UIUnitControl* a_unitControl) :
			UIComponent{a_window, a_parent},
			m_vm(a_vm),
			m_unitId(a_unitId),
			m_unitControl(a_unitControl)
		{}
	protected:
		VoiceManager* m_vm;
		int m_unitId;
		UIUnitControl* m_unitControl;
	};
}
#endif