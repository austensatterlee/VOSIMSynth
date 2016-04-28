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

namespace syn
{


	class DefaultUnitControl : public UIUnitControl
	{
	public:
		DefaultUnitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId)
			: UIUnitControl{ a_window, a_vm, a_unitId }
		{
			const Unit&	unit = a_vm->getUnit(a_unitId);
			int nParams = unit.getNumParameters();
			for (int i = 0; i<nParams; i++) {
				UITextSlider* paramControl = new UITextSlider(m_window, m_vm, m_unitId, i);
				addChild(paramControl);
				m_paramControls.push_back(paramControl);
				paramControl->setSize({ 0,m_controlHeight });
				paramControl->setRelPos({ 0,i*m_controlHeight });
				paramControl->setAutosize(false);
			}
		}
		Vector2i calcAutoSize() const override;
		void onResize() override;
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		vector<UITextSlider*> m_paramControls;

		int m_controlHeight = 14;
	};
}
#endif