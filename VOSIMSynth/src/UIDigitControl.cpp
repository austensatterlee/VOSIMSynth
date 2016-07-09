#include "UIDigitControl.h"
#include "UITextBox.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UICell.h"
#include "UnitParameter.h"
#include <Theme.h>

namespace synui
{
	UIDigitControl::UIDigitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId, int a_paramId) :
		UIParamControl(a_window, a_vm, a_unitId, a_paramId),
		m_decimalLoc(-1),
		m_mainCol(new UICol(a_window)),
		m_digitsRow(new UIRow(a_window)),
		m_signCol(new UICol(a_window)),
		m_signLabel(new UILabel(a_window))
	{
		addChild(m_mainCol);
		// Set up first row
		m_nameLabel = new UILabel(a_window);
		m_nameLabel->setFontSize(16);
		m_nameLabel->setFontSize(16);
		m_unitsLabel = new UILabel(a_window);
		UIRow* nameRow = new UIRow(a_window);
		nameRow->addChild(m_nameLabel);
		nameRow->setGreedyChild(m_nameLabel);
		// Set up second row
		UIRow* valueRow = new UIRow(a_window);
		valueRow->addChild(m_digitsRow);
		valueRow->addChild(m_unitsLabel);
		valueRow->setGreedyChild(m_digitsRow);
		valueRow->setChildResizePolicy(UICell::CMATCHMAX);
		// Add rows to main column
		m_mainCol->addChild(nameRow);
		m_mainCol->addChild(valueRow);
		m_mainCol->setChildResizePolicy(UICell::CMAX);

		m_digitsRow->setChildrenSpacing(6);
		m_digitsRow->setPadding({ 6,6,6,6 });
		m_digitsRow->setChildResizePolicy(UICell::CMATCHMAX);
		m_digitsRow->addChild(m_signCol);
		m_signCol->addChild(m_signLabel);
		m_signCol->setGreedyChild(m_signLabel);
		m_signLabel->setAlignment(NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
		m_signLabel->setFontSize(14);
		UIDigitControl::updateValue_();
	}

	void UIDigitControl::setNumDigits_(int a_numDigits) {
		while (m_digitCols.size() > a_numDigits) {
			m_digitsRow->removeChild(m_digitCols.back());
			m_digitCols.pop_back();
			m_digitBoxes.pop_back();
			m_upArrows.pop_back();
			m_downArrows.pop_back();
		}

		while (m_digitBoxes.size() < a_numDigits) {
			UICol* digitCol = new UICol(m_window);
			digitCol->setChildrenSpacing(0);
			digitCol->setChildResizePolicy(UICell::CMATCHMAX);
			UIButton* upArrow = new UIButton(m_window, "", ENTYPO_UP);
			upArrow->setFontSize(12);
			UILabel* digitBox = new UILabel(m_window);
			digitBox->setAlignment(NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
			digitBox->setFontSize(12);
			digitBox->setText("0");
			digitBox->setFontFace(theme()->mFontMono);
			UIButton* downArrow = new UIButton(m_window, "", ENTYPO_DOWN);
			downArrow->setFontSize(13);

			int digitNum = m_digitBoxes.size();
			auto upFunc = [this, digitNum]() {this->onUpArrow_(digitNum); };
			auto downFunc = [this, digitNum]() {this->onDownArrow_(digitNum); };
			upArrow->setCallback(upFunc);
			downArrow->setCallback(downFunc);

			digitCol->addChild(upArrow);
			m_upArrows.push_back(upArrow);

			digitCol->addChild(digitBox);
			m_digitBoxes.push_back(digitBox);

			digitCol->addChild(downArrow);
			m_downArrows.push_back(downArrow);

			m_digitCols.push_back(digitCol);
			m_digitsRow->addChild(digitCol);
		}

		setMinSize(m_digitsRow->minSize());
	}

	void UIDigitControl::onUpArrow_(int a_digit) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
			incrDigit(a_digit, 10);
		}
		else {
			incrDigit(a_digit, 1);
		}
	}

	void UIDigitControl::onDownArrow_(int a_digit) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
			decrDigit(a_digit, getDigit(a_digit));
		}
		else {
			decrDigit(a_digit, 1);
		}
	}

	string extractDigits(const string& a_str)
	{
		string digitStr = "";
		for (int i = 0; i < a_str.size(); i++) {
			if (isdigit(a_str[i])) {
				digitStr += a_str[i];
			}
		}
		return digitStr;
	}

	void UIDigitControl::setValue_(const string& a_num) {
		m_decimalLoc = static_cast<int>(a_num.find_first_of("."));
		if (m_decimalLoc == string::npos)
			m_decimalLoc = -1;
		else if (a_num[0] == '-' || a_num[0] == '+')
			m_decimalLoc -= 1;

		string digitStr = extractDigits(a_num);
		if (m_digitBoxes.size() <= digitStr.size())
			setNumDigits_(digitStr.size());

		for (int i = 0; i < m_digitBoxes.size(); i++) {
			if (i == m_decimalLoc) {
				m_digitBoxes[m_digitBoxes.size() - 1 - i]->setText(".0");
			}
			else {
				m_digitBoxes[m_digitBoxes.size() - 1 - i]->setText("0");
			}
		}

		for (int i = 0; i < digitStr.size(); i++) {
			if (i == m_decimalLoc)
				m_digitBoxes[m_digitBoxes.size() - digitStr.size() + i]->setText("." + digitStr.substr(i, 1));
			else
				m_digitBoxes[m_digitBoxes.size() - digitStr.size() + i]->setText(digitStr.substr(i, 1));
		}

		double num = stod(a_num);
		if (num < 0)
			m_signLabel->setText("-");
		else
			m_signLabel->setText("+");
	}

	double UIDigitControl::getValue() const {
		return stod(getStrValue());
	}

	string UIDigitControl::getStrValue() const {
		string strValue = m_signLabel->text();
		for (int i = 0; i < m_digitBoxes.size(); i++) {
			strValue += m_digitBoxes[i]->text();
		}
		return strValue;
	}

	int UIDigitControl::getDigit(int a_digit) {
		if (a_digit == m_decimalLoc) return -1;
		return stoi(m_digitBoxes[a_digit]->text());
	}

	void UIDigitControl::incrDigit(int a_digit, int a_amt) {
		nudgeParam(m_digitCols.size() - 1 - a_digit, a_amt);
	}

	void UIDigitControl::decrDigit(int a_digit, int a_amt) {
		nudgeParam(m_digitCols.size() - 1 - a_digit, -a_amt);
	}

	void UIDigitControl::draw(NVGcontext* a_nvg) {
		UIParamControl::draw(a_nvg);

		Vector2i boxPos;
		Vector2i boxSize;
		NVGpaint bgPaint;

		nvgBeginPath(a_nvg);
		boxPos = m_digitsRow->getAbsPos() - getAbsPos() + Vector2i{ 6, 6 };
		boxSize = m_digitsRow->size() - Vector2i{ 12, 12 };
		bgPaint = nvgLinearGradient(a_nvg,
			boxPos.x(), boxPos.y(),
			boxPos.x(), boxPos.y() + size().y()*4.0f / 5.0f,
			Color(0.3f, 1.0f), Color(0.2f, 1.0f));
		nvgRoundedRect(a_nvg, boxPos.x(), boxPos.y(), boxSize.x(), boxSize.y(), 3.0f);
		nvgFillPaint(a_nvg, bgPaint);
		nvgFill(a_nvg);

		nvgBeginPath(a_nvg);
		boxPos = m_digitsRow->getAbsPos() - getAbsPos() + Vector2i{ 6, 6 };
		boxSize = m_digitsRow->size() - Vector2i{ 12, 12 };
		bgPaint = nvgBoxGradient(a_nvg,
			boxPos.x(), boxPos.y(),
			boxSize.x(), boxSize.y(),
			6.0f, 12.0f,
			Color(0.0f, 0.0f, 0.2f, 1.0f), Color(0.0f, 0.0f));
		nvgRect(a_nvg, boxPos.x() - 6, boxPos.y() - 6, boxSize.x() + 12, boxSize.y() + 12);
		nvgRoundedRect(a_nvg, boxPos.x(), boxPos.y(), boxSize.x(), boxSize.y(), 3.0f);
		nvgPathWinding(a_nvg, NVG_HOLE);
		nvgFillPaint(a_nvg, bgPaint);
		nvgFill(a_nvg);

		for (int i = 0; i < m_digitBoxes.size() - 1; i++) {
			nvgBeginPath(a_nvg);
			nvgStrokeWidth(a_nvg, 1.0f);
			Vector2i sepBegin = m_digitBoxes[i]->getAbsPos() - getAbsPos() + Vector2i{m_digitBoxes[i]->size().x(), 0};
			nvgMoveTo(a_nvg, sepBegin.x() + 3.0f, sepBegin.y());
			nvgLineTo(a_nvg, sepBegin.x() + 3.0f, sepBegin.y() + m_digitBoxes[i]->size().y());
			nvgStrokeColor(a_nvg, Color(0.7f,1.0f));
			nvgStroke(a_nvg);
		}
	}

	void UIDigitControl::updateValue_() {
		UIParamControl::updateValue_();
		int nDigits = 1 + ceil(log10(m_param->getMax())) + (m_param->getPrecision() > 0 ? m_param->getPrecision() + 1 : 0);
		setNumDigits_(nDigits);
		setValue_(m_param->getValueString());
		setMinSize(m_mainCol->minSize());
	}
}