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
		m_strValue(""),
		m_mainCol( new UICol(a_window) ),
		m_digitsRow( new UIRow(a_window) ),
		m_signCol( new UICol(a_window) ),
		m_signLabel( new UILabel(a_window) )
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

		m_strValue = m_signLabel->text();
		for (int i = 0; i<m_digitBoxes.size(); i++) {
			m_strValue += m_digitBoxes[i]->text();
		}

		setMinSize(m_digitsRow->minSize());
	}

	void UIDigitControl::onUpArrow_(int a_digit) {
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
			incrDigit(a_digit, 10);
		}else {
			incrDigit(a_digit, 1);
		}
	}

	void UIDigitControl::onDownArrow_(int a_digit) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
			decrDigit(a_digit, 1 + getDigit(a_digit));
		}
		else {
			decrDigit(a_digit, 1);
		}
	}

	void UIDigitControl::setValue_(const string& a_num) {
		m_decimalLoc = static_cast<int>(a_num.find_first_of("."));
		if (m_decimalLoc == string::npos)
			m_decimalLoc = -1;

		int c;
		int nStringDigits;
		if(a_num[0] == '-' || a_num[0] == '+') {
			// ignore leading sign
			c = 1; 
			m_decimalLoc = m_decimalLoc < 0 ? m_decimalLoc : m_decimalLoc - 1;
			nStringDigits = a_num.size() - 1;
		}else {
			c = 0;
			nStringDigits = a_num.size();
		}
		if (nStringDigits >= m_digitBoxes.size())
			setNumDigits_(nStringDigits);

		m_decimalLoc = m_decimalLoc + (m_digitBoxes.size() - a_num.size());		
		for (int i=m_digitBoxes.size()-a_num.size(); i<m_digitBoxes.size(); i++, c++) {
			m_upArrows[i]->setVisible(i != m_decimalLoc);
			m_downArrows[i]->setVisible(i != m_decimalLoc);
			m_digitBoxes[i]->setText(a_num.substr(c, 1));
		}

		double num = stod(a_num);
		if (num < 0)
			m_signLabel->setText("-");
		else
			m_signLabel->setText("+");

		m_strValue = m_signLabel->text();
		for(int i=0;i<m_digitBoxes.size();i++) {
			m_strValue += m_digitBoxes[i]->text();
		}
	}

	double UIDigitControl::getValue() const {
		return stod(getStrValue());
	}

	string UIDigitControl::getStrValue() const {
		return m_signLabel->text() + m_strValue;
	}

	int UIDigitControl::getDigit(int a_digit) {
		if (a_digit == m_decimalLoc) return -1;
		return stoi(m_digitBoxes[a_digit]->text());
	}

	void UIDigitControl::setDigit(int a_digit, int a_value) {
		if (a_digit == m_decimalLoc) return;
		a_value = syn::CLAMP<int>(a_value, 0, 9);
		m_digitBoxes[a_digit]->setText(to_string(a_value));
		m_strValue[a_digit] = to_string(a_value)[0];
		setParamFromString(getStrValue());
	}

	void UIDigitControl::incrDigit(int a_digit, int a_amt) {
		if (a_digit == m_decimalLoc) return;
		if (a_digit > m_decimalLoc) a_digit--;
		nudgeParam(m_digitCols.size() - 2 - a_digit, a_amt);
	}

	void UIDigitControl::decrDigit(int a_digit, int a_amt) {
		if (a_digit == m_decimalLoc) return;
		if (a_digit > m_decimalLoc) a_digit--;
		nudgeParam(m_digitCols.size() - 2 - a_digit, -a_amt);
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
			boxPos.x(), boxPos.y() + size().y()*3.0f / 4.0f,
			Color(0.3f, 1.0f), Color(0.1f, 1.0f));
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
			Color(0.0f,0.0f,0.2f,1.0f), Color(0.0f, 0.0f));
		nvgRect(a_nvg, boxPos.x() - 6, boxPos.y() - 6, boxSize.x() + 12, boxSize.y() + 12);
		nvgRoundedRect(a_nvg, boxPos.x(), boxPos.y(), boxSize.x(), boxSize.y(), 3.0f);
		nvgPathWinding(a_nvg, NVG_HOLE);
		nvgFillPaint(a_nvg, bgPaint);
		nvgFill(a_nvg);

		for(int i=0;i<m_digitBoxes.size();i++) {
			nvgBeginPath(a_nvg);
			nvgStrokeWidth(a_nvg, 2.0f);
			Vector2i sepBegin = m_digitCols[i]->getAbsPos() + Vector2i{ m_digitCols[i]->size().x() + 2.5, 0 } - getAbsPos();
			Vector2i sepEnd = sepBegin + Vector2i{0, m_digitCols[i]->size().y()};
			NVGpaint separatorPaint = nvgLinearGradient(a_nvg,
				sepBegin.x()+ 0.5, sepBegin.y(),
				sepBegin.x() + 1.5f, sepBegin.y(),
				Color(0.1f, 1.0f), Color(0.3f, 1.0f));
			nvgMoveTo(a_nvg, sepBegin.x() + 1.0f, sepBegin.y());
			nvgLineTo(a_nvg, sepEnd.x() + 1.0f, sepEnd.y());
			nvgStrokePaint(a_nvg, separatorPaint);
			nvgStroke(a_nvg); 

			nvgBeginPath(a_nvg);
			separatorPaint = nvgLinearGradient(a_nvg,
				sepBegin.x() + 1.5f, sepBegin.y(),
				sepBegin.x() + 2.5f, sepBegin.y(),
				Color(0.3f, 1.0f), Color(0.1f, 1.0f)); 
			nvgMoveTo(a_nvg, sepBegin.x() + 2.0f, sepBegin.y());
			nvgLineTo(a_nvg, sepEnd.x() + 2.0f, sepEnd.y());
			nvgStrokePaint(a_nvg, separatorPaint);
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
