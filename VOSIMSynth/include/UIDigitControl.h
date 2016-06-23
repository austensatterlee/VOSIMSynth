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

#include "UIComponent.h"

namespace syn {
	class UIDigitControl : public UIComponent
	{
	public:
		UIDigitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_paramId);

		void setNumDigits(int a_numDigits);

		void setValue(const string& a_num);

		double getValue() const;

		int getDigit(int a_digit);

		void setDigit(int a_digit, int a_value);
	private:
		int m_decimalLoc;

		string m_value;
		vector<UITextBox*> m_textBoxes;
		vector<UIButton*> m_upArrows;
		vector<UIButton*> m_downArrows;
		UIRow* m_row;
		vector<UICol*> m_digitCols;

		VoiceManager* m_vm;
		int m_unitId, m_paramId;
	};
}

#endif
