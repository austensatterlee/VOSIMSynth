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
*  \file UIDefaultUnitControl.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIDEFAULTUNITCONTROL__
#define __UIDEFAULTUNITCONTROL__
#include "UIUnitControl.h"
#include "UITextSlider.h"
#include "UICell.h"

namespace syn
{
	class DefaultUnitControl : public UIUnitControl
	{
	public:
		DefaultUnitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId);
		void notifyChildResized(UIComponent* a_child) override;

	private:
		void _onResize() override;

	private:
		vector<UITextSlider*> m_paramControls;
		UICol* m_col;
	};
}
#endif
