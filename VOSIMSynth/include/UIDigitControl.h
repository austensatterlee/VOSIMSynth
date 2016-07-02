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
 *  \file UIDigitControl.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 05/2016
 */

#ifndef __UIDIGITCONTROL__
#define __UIDIGITCONTROL__

#include "UIParamControl.h"

namespace synui {
	class UIDigitControl : public UIParamControl
	{
	public:
		UIDigitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId, int a_paramId);
		double getValue() const;
		string getStrValue() const;
		int getDigit(int a_digit);
		void setDigit(int a_digit, int a_value);
		void incrDigit(int a_digit, int a_amt);
		void decrDigit(int a_digit, int a_amt);
	protected:
		void draw(NVGcontext* a_nvg) override;
		void updateValue_() override;
		void setValue_(const string& a_num);
		void setNumDigits_(int a_numDigits);
		void onUpArrow_(int a_digit);
		void onDownArrow_(int a_digit);
	private:
		int m_decimalLoc;

		string m_strValue;
		vector<UILabel*> m_digitBoxes;
		vector<UIButton*> m_upArrows;
		vector<UIButton*> m_downArrows;
		UICol* m_mainCol;
		UIRow* m_digitsRow;
		vector<UICol*> m_digitCols;
		UICol* m_signCol;
		UILabel* m_signLabel;
	};
}

#endif
