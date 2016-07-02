#include "UITextSlider.h"
#include "VoiceManager.h"
#include "UnitParameter.h"
#include "UICell.h"
#include "UILabel.h"
#include "Theme.h"
#include "UITextBox.h"

synui::UITextSlider::UITextSlider(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId, int a_paramId) :
	UIParamControl{ a_window, a_vm, a_unitId, a_paramId }
{
	m_textBox = new UITextBox(m_window);
	m_textBox->setVisible(false);
	m_textBox->setCallback([&](const string& a_str)-> bool {
		setParamFromString(a_str);
		m_textBox->setVisible(false);
		m_row->setVisible(true);
		return true;
	});
	m_textBox->setFontSize(theme()->mTextSliderFontSize);
	addChild(m_textBox);

	m_row = new UIRow(a_window);
	m_row->setChildrenSpacing(0);
	m_row->setSelfMinSizePolicy(UICell::SNONE);
	m_nameLabel = new UILabel(a_window);
	m_nameLabel->setFontSize(theme()->mTextSliderFontSize);
	m_nameLabel->setAlignment(NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
	m_valueLabel = new UILabel(a_window);
	m_valueLabel->setFontColor(Color{ 1.0f,1.0f,0.80f,1.0f });
	m_valueLabel->setAlignment(NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT);
	m_valueLabel->setFontSize(theme()->mTextSliderFontSize);
	m_unitsLabel = new UILabel(a_window);
	m_unitsLabel->setFontSize(theme()->mTextSliderFontSize);
	m_unitsLabel->setFontColor(Color{ 1.0f,1.0f,0.8f,0.6f });
	m_unitsLabel->setAlignment(NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT);
	m_row->addChild(m_nameLabel);
	m_row->addChild(m_valueLabel);
	m_row->addChild(m_unitsLabel);
	m_row->setGreedyChild(m_nameLabel);
	addChild(m_row);

	UITextSlider::updateValue_();
}

bool synui::UITextSlider::onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
		double normval = m_param->getNorm();
		double targetValue = (a_relCursor.localCoord(this)[0]) * (1.0 / size()[0]);
		double error = normval - targetValue;
		double adjust_speed = 0.5;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
			adjust_speed *= 0.1;
		normval = normval - adjust_speed * error;

		setParamNorm(normval);
		return true;
	}
	return false;
}

synui::UIComponent* synui::UITextSlider::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* retval = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (retval)
		return retval;

	if (a_isDblClick) {
		setParamValue(m_param->getDefaultValue());
		return nullptr;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
		m_textBox->setVisible(true);
		m_row->setVisible(false);
		m_window->setFocus(m_textBox);
		return m_textBox->onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	}
	return this;
}

bool synui::UITextSlider::onMouseScroll(const UICoord& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) {
	double linScale = a_scrollAmt;
	double logScale;
	if (m_param->getType() == syn::UnitParameter::Double)
		logScale = 1;
	else
		logScale = 0;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
		logScale+=1.0;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
		logScale -= 1.0;
	}
	
	nudgeParam(logScale, linScale);
	
	return true;
}

void synui::UITextSlider::draw(NVGcontext* a_nvg) {
	if (!m_textBox->visible()) {
		UIParamControl::draw(a_nvg);
		nvgBeginPath(a_nvg);
		nvgFillColor(a_nvg, Color(Vector3f{ 0.4f,0.1f,0.7f }*0.25, 0.7f));
		nvgRect(a_nvg, 0, 0, size()[0], size()[1]);
		nvgFill(a_nvg);

		nvgFillColor(a_nvg, Color(Vector3f{ 0.4f,0.1f,0.7f }));
		nvgBeginPath(a_nvg);
		float fgWidth = size()[0] * m_normValue;
		nvgRect(a_nvg, 0, 0, fgWidth, size()[1]);
		nvgFill(a_nvg);
	}
}

void synui::UITextSlider::_updateMinSize() {
	setMinSize(m_row->minSize().cwiseMax(m_textBox->minSize()));
}

void synui::UITextSlider::_onResize() {
	m_textBox->setSize(size());
	m_row->setSize(size());
}

void synui::UITextSlider::updateValue_() {
	UIParamControl::updateValue_();
	m_textBox->setValue(m_valueLabel->text());
	m_textBox->setUnits(m_unitsLabel->text());
	_updateMinSize();
	_onResize();
}