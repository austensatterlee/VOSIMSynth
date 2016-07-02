#include "UIDigitControl.h"
#include "UITextBox.h"
#include "UIButton.h"
#include "UICell.h"
#include "UnitParameter.h"

namespace synui
{
	UIDigitControl::UIDigitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId, int a_paramId) :
		UIParamControl(a_window, a_vm, a_unitId, a_paramId),
		m_decimalLoc(-1),
		m_strValue(""),
		m_row( new UIRow(a_window) )
	{
		addChild(m_row);
		m_row->setChildrenSpacing(0);
		UIDigitControl::updateValue_();
	}

	void UIDigitControl::setNumDigits_(int a_numDigits) {
		while (m_digitCols.size() > a_numDigits) {
			m_row->removeChild(m_digitCols.back());
			m_digitCols.pop_back();
			m_textBoxes.pop_back();
			m_upArrows.pop_back();
			m_downArrows.pop_back();
		}

		while (m_textBoxes.size() <= a_numDigits) {
			UICol* digitCol = new UICol(m_window);
			digitCol->setChildrenSpacing(0);
			digitCol->setChildResizePolicy(UICell::CMATCHMAX);
			UIButton* upArrow = new UIButton(m_window, "", ENTYPO_UP);
			upArrow->setFontSize(10);
			UITextBox* digitBox = new UITextBox(m_window, "0");
			digitBox->setFontSize(10);
			digitBox->setAlignment(UITextBox::Alignment::Center);
			UIButton* downArrow = new UIButton(m_window, "", ENTYPO_DOWN); 
			downArrow->setFontSize(10);

			int digitNum = m_textBoxes.size();
			auto upFunc = [this, digitNum]() {this->setDigit(digitNum, syn::WRAP<int>(this->getDigit(digitNum) + 1, 10)); };
			auto downFunc = [this, digitNum]() {this->setDigit(digitNum, syn::WRAP<int>(this->getDigit(digitNum) - 1, 10)); };
			upArrow->setCallback(upFunc);
			downArrow->setCallback(downFunc);

			digitCol->addChild(upArrow);
			m_upArrows.push_back(upArrow);

			digitCol->addChild(digitBox);
			m_textBoxes.push_back(digitBox);

			digitCol->addChild(downArrow);
			m_downArrows.push_back(downArrow);

			m_digitCols.push_back(digitCol);
			m_row->addChild(digitCol);
		}

		setMinSize(m_row->minSize());
	}

	void UIDigitControl::setValue_(const string& a_num) {
		setNumDigits_(a_num.size());
		m_decimalLoc = static_cast<int>(a_num.find_first_of("."));
		if (m_decimalLoc == string::npos) m_decimalLoc = -1;
		for (int i = 0; i<a_num.size(); i++) {
			m_textBoxes[i]->setValue(a_num.substr(i, 1));
		}
		m_strValue = a_num;
	}

	double UIDigitControl::getValue() const {
		return stod(m_strValue);
	}

	int UIDigitControl::getDigit(int a_digit) {
		if (a_digit == m_decimalLoc) return -1;
		return stoi(m_textBoxes[a_digit]->value());
	}

	void UIDigitControl::setDigit(int a_digit, int a_value) {
		if (a_digit == m_decimalLoc) return;
		if (a_value < 0 || a_value > 9) return;
		m_textBoxes[a_digit]->setValue(to_string(a_value));
		m_strValue[a_digit] = to_string(a_value)[0];
		setParamFromString(m_strValue);
	}

	void UIDigitControl::updateValue_() {
		UIParamControl::updateValue_();
		setValue_(m_param->getValueString());
	}
}
