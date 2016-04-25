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
*  \file UIUnitControl.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIUNITCONTROL__
#define __UIUNITCONTROL__
#include "UIComponent.h"
#include <VoiceManager.h>
#include "UITextSlider.h"

namespace syn
{
	class UIUnitControl : public UIComponent
	{
	public:
		UIUnitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId) :
			UIComponent{a_window},
			m_vm(a_vm),
			m_unitId(a_unitId)
		{}
	protected:
		VoiceManager* m_vm;
		int m_unitId;
	};
}
#endif