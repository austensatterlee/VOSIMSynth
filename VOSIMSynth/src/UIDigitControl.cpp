#include "UIDigitControl.h"
#include "UITextBox.h"
#include "UIButton.h"
#include "UICell.h"

namespace syn
{
	UIDigitControl::UIDigitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_paramId) :
		UIComponent(a_window),
		m_decimalLoc(-1),
		m_value(""),
		m_row( new UIRow(a_window) ),
		m_vm(a_vm),
		m_unitId(a_unitId),
		m_paramId(a_paramId)
	{
		addChild(m_row);
		m_row->setChildrenSpacing(0);
	}

	void UIDigitControl::setNumDigits(int a_numDigits) {
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
			auto upFunc = [this, digitNum]() {this->setDigit(digitNum, WRAP<int>(this->getDigit(digitNum) + 1, 10)); };
			auto downFunc = [this, digitNum]() {this->setDigit(digitNum, WRAP<int>(this->getDigit(digitNum) - 1, 10)); };
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

	void UIDigitControl::setValue(const string& a_num) {
		setNumDigits(a_num.size());
		m_decimalLoc = static_cast<int>(a_num.find_first_of("."));
		if (m_decimalLoc == string::npos) m_decimalLoc = -1;
		for (int i = 0; i<a_num.size(); i++) {
			m_textBoxes[i]->setValue(a_num.substr(i, 1));
		}
		m_value = a_num;
	}

	double UIDigitControl::getValue() const {
		return stod(m_value);
	}

	int UIDigitControl::getDigit(int a_digit) {
		if (a_digit == m_decimalLoc) return -1;
		return stoi(m_textBoxes[a_digit]->value());
	}

	void UIDigitControl::setDigit(int a_digit, int a_value) {
		if (a_digit == m_decimalLoc) return;
		m_textBoxes[a_digit]->setValue(to_string(a_value));
	}
}
