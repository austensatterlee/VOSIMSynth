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
#include <UIComponent.h>
#include "UICell.h"
#include "UITextBox.h"

namespace syn {
	class UIDigitalControl : public UIComponent
	{
	public:
		UIDigitalControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_paramId)
			: 
			UIComponent(a_window),
			m_value(0.0),
			m_row(nullptr),
			m_vm(a_vm),
			m_unitId(a_unitId),
			m_paramId(a_paramId) 
		{
			setNumDigits(5);
		}

		void setNumDigits(int a_numDigits) {
			while(m_digits.size()>a_numDigits)
				m_digits.pop_back();
			while (m_digits.size() <= a_numDigits)
				m_digits.push_back(0); 

			m_textBoxes.clear();
			removeChild(m_row);
			m_row = new UIRow(m_window);
			addChild(m_row);			
			while (m_textBoxes.size() <= a_numDigits) {
				UITextBox* digitBox = new UITextBox(m_window, "0");
				m_textBoxes.push_back(digitBox);
				m_row->addChild(digitBox);
			}
		}
	protected:
	private:
		double m_value;
		vector<int> m_digits;

		vector<UITextBox*> m_textBoxes;
		UIRow* m_row;

		VoiceManager* m_vm;
		int m_unitId, m_paramId;
	};
}

#endif
