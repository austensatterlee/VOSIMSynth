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
*  \brief Complete GUI for a single unit, containing all necessary UIParamControls. Displayed in UIControlPanel.
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIDEFAULTUNITCONTROL__
#define __UIDEFAULTUNITCONTROL__
#include "UIUnitControl.h"
#include "UICell.h"

namespace synui
{
	class UIParamControl;

	class UIDefaultUnitControl : public UIUnitControl
	{
	public:
		UIDefaultUnitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId);
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		void _onResize() override;
	private:
		vector<UIParamControl*> m_paramControls;
	};
}
#endif
