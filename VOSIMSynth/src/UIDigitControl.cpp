#include "UIDigitControl.h"
#include "UITextBox.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UICell.h"
#include "UnitParameter.h"
#include <Theme.h>

namespace synui
{
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

	int extractPrecision(const string& a_str) {
		int count = 0;
		int i = a_str.size() - 1;
		while(i--){
			if (isdigit(a_str[i]) || a_str[i]=='.') {
				if (a_str[i] == '.') return count;
				count++;
			}
		}
		return -1;
	}

	UIDigitControl::UIDigitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId, int a_paramId) :
		UIParamControl(a_window, a_vm, a_unitId, a_paramId),
		m_mainCell(new UIRow(a_window)),
		m_decimalCol(new UICol(a_window)),
		m_digitsRow(new UIRow(a_window)),
		m_digitColor(0.05f,0.01f,0.01f,1.0f),
		m_signCol(new UICol(a_window)),
		m_signLabel(new UILabel(a_window))
	{
		addChild(m_mainCell);
		// Set up second row
		UIRow* valueRow = new UIRow(a_window);
		valueRow->addChild(m_digitsRow);
		valueRow->setGreedyChild(m_digitsRow);
		valueRow->setChildResizePolicy(UICell::CMATCHMAX);
		valueRow->setPadding({ 2,2,2,2 });
		// Add rows to main column
		m_mainCell->addChild(valueRow);
		m_mainCell->setChildResizePolicy(UICell::CMAX);
		m_mainCell->setSelfMinSizePolicy(UICell::SNONE);
		m_mainCell->setChildrenSpacing(0.0);
		m_mainCell->setGreedyChild(valueRow);
		
		UILabel* decimalLbl = m_decimalCol->add<UILabel>("");
		decimalLbl->setText(".");
		decimalLbl->setFontFace(theme()->mFontMono);
		decimalLbl->setAlignment(NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
		decimalLbl->setFontSize(14);
		decimalLbl->setFontColor(m_digitColor);
		m_decimalCol->setVisible(false);
		m_decimalCol->setGreedyChild(decimalLbl);

		m_signCol->addChild(m_signLabel);
		m_signCol->setGreedyChild(m_signLabel);
		m_signLabel->setAlignment(NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT);
		m_signLabel->setFontSize(12);
		m_signLabel->setFontColor(m_digitColor);
		m_signLabel->setFontFace(theme()->mFontMono);

		m_digitsRow->setChildrenSpacing(1);
		m_digitsRow->setChildResizePolicy(UICell::CMATCHMIN);
		m_digitsRow->addChild(m_signCol);
		m_digitsRow->addChild(m_decimalCol);
		
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
			upArrow->setIconPosition(UIButton::IconPosition::LeftCentered);
			upArrow->setTextColor(Color(0.95f, 0.95f, 0.90f, 1.0f));
			upArrow->setFontSize(12);
			upArrow->setBackgroundColor(Color(0.1f, 0.1f, 0.1f, 1.0f));
			UILabel* digitBox = new UILabel(m_window);
			digitBox->setAlignment(NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
			digitBox->setFontSize(14);
			digitBox->setText("0");
			digitBox->setFontFace(theme()->mFontMono);
			digitBox->setFontColor(m_digitColor);
			UIButton* downArrow = new UIButton(m_window, "", ENTYPO_DOWN);
			downArrow->setIconPosition(UIButton::IconPosition::LeftCentered);
			downArrow->setTextColor(Color(0.95f, 0.95f, 0.90f, 1.0f));
			downArrow->setBackgroundColor(Color(0.1f, 0.1f, 0.1f, 1.0f));
			downArrow->setFontSize(12);

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

	void UIDigitControl::setDecimalLoc(int a_digit) {
		if (a_digit < 0 || a_digit>(m_digitBoxes.size()-1)) {
			m_decimalCol->setVisible(false);
			return;
		}
		m_decimalCol->setVisible(true);
		m_decimalCol->getChild(0)->setSize({ -1, m_digitsRow->size().y() });
		m_digitsRow->setChildIndex(m_digitsRow->getChildIndex(m_decimalCol.get()), a_digit);
	}

	void UIDigitControl::_onResize() {
		m_mainCell->setSize(size());
	}

	void UIDigitControl::setValue_(const string& a_num) {
		string digitStr = extractDigits(a_num);
		if (m_digitBoxes.size() <= digitStr.size())
			setNumDigits_(digitStr.size());

		int decimalLoc = extractPrecision(a_num);
		setDecimalLoc(m_digitBoxes.size() - decimalLoc);

		for (int i = 0; i < m_digitBoxes.size(); i++) {
			m_digitBoxes[m_digitBoxes.size() - 1 - i]->setText("0");
		}

		for (int i = 0; i < digitStr.size(); i++) {
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
		return stoi(extractDigits(m_digitBoxes[a_digit]->text()));
	}

	void UIDigitControl::incrDigit(int a_digit, int a_amt) {
		nudgeParam(m_digitCols.size() - 1 - a_digit, a_amt);
	}

	void UIDigitControl::decrDigit(int a_digit, int a_amt) {
		nudgeParam(m_digitCols.size() - 1 - a_digit, -a_amt);
	}

	bool UIDigitControl::onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
		return UIParamControl::onMouseDrag(a_relCursor, a_diffCursor);
	}

	bool UIDigitControl::onMouseScroll(const UICoord& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) {
		return UIParamControl::onMouseScroll(a_relCursor, a_diffCursor, a_scrollAmt);
		
	}

	UIComponent* UIDigitControl::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
		return UIParamControl::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	}

	void UIDigitControl::draw(NVGcontext* a_nvg) {
		UIParamControl::draw(a_nvg);

		Vector2i boxPos;
		Vector2i boxSize;
		NVGpaint bgPaint;

		nvgBeginPath(a_nvg);
		boxPos = m_digitsRow->getAbsPos() - getAbsPos();
		boxSize = m_digitsRow->size();
		bgPaint = nvgLinearGradient(a_nvg,
			boxPos.x(), boxPos.y() + boxSize.y() * 15. / 16,
			boxPos.x(), boxPos.y() + boxSize.y(),
			Color(0.1f, 1.0f), Color(0.3f, 1.0f));
		nvgRoundedRect(a_nvg, boxPos.x(), boxPos.y(), boxSize.x(), boxSize.y(), 6.0f);
		nvgFillPaint(a_nvg, bgPaint);
		nvgFill(a_nvg);

		nvgBeginPath(a_nvg);
		boxPos = { m_digitsRow->getAbsPos().x(), m_digitBoxes[0]->getAbsPos().y() };
		boxPos -= getAbsPos();
		boxSize = { m_digitBoxes.back()->getAbsPos().x() + m_digitBoxes.back()->size().x() - m_digitsRow->getAbsPos().x(), m_digitBoxes.front()->size().y() };
		bgPaint = nvgBoxGradient(a_nvg,
			boxPos.x()+2.0, boxPos.y()+2.0,
			boxSize.x()-4.0, boxSize.y()-4.0,
			1.0f, 1.0f,
			Color(0.65,0.6,0.55f, 1.0f), Color(0.2,0.2,0.2f, 1.0f));
		nvgRoundedRect(a_nvg, boxPos.x(), boxPos.y(), boxSize.x(), boxSize.y(), 1.0f);
		nvgFillPaint(a_nvg, bgPaint);
		nvgFill(a_nvg);

		nvgBeginPath(a_nvg);
		boxPos = m_digitsRow->getAbsPos() - getAbsPos();
		boxSize = m_digitsRow->size();
		bgPaint = nvgBoxGradient(a_nvg,
			boxPos.x(), boxPos.y(),
			boxSize.x(), boxSize.y(),
			6.0f, 12.0f,
			Color(0.2f, 0.0f, 0.0f, 0.9f), Color(0.0f, 0.0f));
		nvgRect(a_nvg, boxPos.x() - 6, boxPos.y() - 6, boxSize.x() + 12, boxSize.y() + 12);
		nvgRoundedRect(a_nvg, boxPos.x(), boxPos.y(), boxSize.x(), boxSize.y(), 6.0f);
		nvgPathWinding(a_nvg, NVG_HOLE);
		nvgFillPaint(a_nvg, bgPaint);
		nvgFill(a_nvg);

		nvgBeginPath(a_nvg);
		Vector2i sepBegin = m_digitCols[0]->getAbsPos() - getAbsPos();
		nvgMoveTo(a_nvg, sepBegin.x(), sepBegin.y());
		nvgLineTo(a_nvg, sepBegin.x(), sepBegin.y() + m_digitCols[0]->size().y());
		sepBegin = m_decimalCol->getAbsPos() - getAbsPos();
		nvgMoveTo(a_nvg, sepBegin.x() + m_decimalCol->size().x(), sepBegin.y());
		nvgLineTo(a_nvg, sepBegin.x() + m_decimalCol->size().x(), sepBegin.y() + m_decimalCol->size().y());
		for (int i = 0; i < m_digitCols.size()-1; i++) {
			sepBegin = m_digitCols[i]->getAbsPos() - getAbsPos() + m_digitCols[i]->size().x()*Vector2i::Unit(0);
			nvgMoveTo(a_nvg, sepBegin.x(), sepBegin.y());
			nvgLineTo(a_nvg, sepBegin.x(), sepBegin.y() + m_digitCols[i]->size().y());
		}
		nvgStrokeColor(a_nvg, Color(0.0f, 0.134f));
		nvgStrokeWidth(a_nvg, 1.0f);
		nvgStroke(a_nvg);
	}

	void UIDigitControl::updateValue_() {
		UIParamControl::updateValue_();
		int nDigits = ceil(log10(m_param->getMax())) + (m_param->getPrecision() > 0 ? m_param->getPrecision() + 1 : 0);
		setNumDigits_(nDigits);
		setValue_(m_param->getValueString());
		m_mainCell->setSize(size());

		m_textBox->setRelPos(m_digitBoxes[0]->getAbsPos() - getAbsPos());
		m_textBox->setSize({ m_digitBoxes.back()->getAbsPos().x() - m_digitBoxes.front()->getAbsPos().x(), m_digitBoxes.front()->size().y() });
	}
}