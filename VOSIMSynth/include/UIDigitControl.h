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
		void incrDigit(int a_digit, int a_amt);
		void decrDigit(int a_digit, int a_amt);

		bool onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;
		bool onMouseScroll(const UICoord& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) override;
		UIComponent* onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) override;
	protected:
		void draw(NVGcontext* a_nvg) override;
		void updateValue_() override;
		void setValue_(const string& a_num);
		void setNumDigits_(int a_numDigits);
		void onUpArrow_(int a_digit);
		void onDownArrow_(int a_digit);
		void setDecimalLoc(int a_digit); /// set the decimal point to the right of the given column (e.g. 0 removes any fractional part)
	private:
		virtual void _onResize() override;;
	private:
		UICell* m_mainCell;
		shared_ptr<UICol> m_decimalCol;
		UIRow* m_digitsRow;
		vector<UILabel*> m_digitBoxes;
		vector<UICol*> m_digitCols;
		Color m_digitColor;
		vector<UIButton*> m_upArrows;
		vector<UIButton*> m_downArrows;
		UICol* m_signCol;
		UILabel* m_signLabel;
	};

	string extractDigits(const string& a_str);
	int extractPrecision(const string& a_str);
}

#endif
