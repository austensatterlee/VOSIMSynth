/*
Copyright 2016, Austen Satterlee

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
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *  \file UIParamControl.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 06/2016
 */

#ifndef __UIPARAMCONTROL__
#define __UIPARAMCONTROL__
#include "UIComponent.h"

namespace syn
{
	class UnitParameter;
}

namespace synui {
	class UIParamControl : public UIComponent
	{
	public:
		UIParamControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId, int a_paramId);
		void setParamFromString(const string& a_str);
		void setParamValue(double a_val);
		void setParamNorm(double a_normval);
		void nudgeParam(double a_logScale, double a_linScale);
	protected:
		virtual void updateValue_();
		void draw(NVGcontext* a_nvg) override;
	protected:
		syn::VoiceManager* m_vm;
		int m_unitId;
		int m_paramId;
		const syn::UnitParameter* m_param;
		UILabel* m_nameLabel;
		UILabel* m_valueLabel;
		UILabel* m_unitsLabel;

		double m_normValue;
		bool m_isValueDirty = false;
	};
}
#endif
