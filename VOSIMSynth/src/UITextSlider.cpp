#include "UITextSlider.h"
#include "VoiceManager.h"
#include "UnitParameter.h"
#include "UICell.h"
#include "UILabel.h"
#include "Theme.h"

syn::UITextSlider::UITextSlider(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_paramId) :
	UIComponent{ a_window },
	m_value(0.0),
	m_vm(a_vm),
	m_unitId(a_unitId),
	m_paramId(a_paramId),
	m_isValueDirty(true)
{
	m_textBox = new UITextBox(m_window);
	m_textBox->setVisible(false);
	m_textBox->setCallback([&](const string& a_str)-> bool {
		setValueFromString(a_str);
		m_textBox->setVisible(false);
		m_row->setVisible(true);
		return true;
	});
	m_textBox->setFontSize(theme()->mTextSliderFontSize);
	addChild(m_textBox);

	m_row = new UIRow(a_window);
	m_row->setChildrenSpacing(0);
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
	m_row->setGreedyChild(m_nameLabel, NVG_ALIGN_LEFT);
	addChild(m_row);

	_updateValue();
}

bool syn::UITextSlider::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
		double currValue = m_param->getNorm();
		double targetValue = (a_relCursor[0]) * (1.0 / size()[0]);
		double error = currValue - targetValue;
		double adjust_speed = 0.5;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
			adjust_speed *= 0.1;
		currValue = currValue - adjust_speed * error;

		RTMessage* msg = new RTMessage();
		msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
			double val;
			int unitId, paramId;
			int pos = 0;
			pos = a_data->Get<int>(&unitId, pos);
			pos = a_data->Get<int>(&paramId, pos);
			pos = a_data->Get<double>(&val, pos);
			a_circuit->setInternalParameterNorm(unitId, paramId, val);
		};
		msg->data.Put<int>(&m_unitId);
		msg->data.Put<int>(&m_paramId);
		msg->data.Put<double>(&currValue);
		m_vm->queueAction(msg);
		m_isValueDirty = true;
		return true;
	}
	return false;
}

syn::UIComponent* syn::UITextSlider::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* retval = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (retval)
		return retval;

	if (a_isDblClick) {
		double newValue = m_param->getDefaultValue();
		RTMessage* msg = new RTMessage();
		msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
			double val;
			int unitId, paramId;
			GetArgs(a_data, 0, unitId, paramId, val);
			a_circuit->setInternalParameter(unitId, paramId, val);
		};
		PutArgs(&msg->data, m_unitId, m_paramId, newValue);
		m_vm->queueAction(msg);
		m_isValueDirty = true;
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

bool syn::UITextSlider::onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) {
	double scale;
	if (m_param->getType() != UnitParameter::Double) {
		scale = 1.0;
	}
	else {
		scale = pow(10, -m_param->getPrecision() + 1);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
			scale *= 10;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
			scale *= 0.1;
		}
	}

	double currValue = m_param->get<double>() + scale * a_scrollAmt;
	RTMessage* msg = new RTMessage();
	PutArgs(&msg->data, m_unitId, m_paramId, currValue);
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
		double currValue;
		int unitId, paramId;
		GetArgs(a_data, 0, unitId, paramId, currValue);
		a_circuit->setInternalParameter(unitId, paramId, currValue);
	};
	m_vm->queueAction(msg);
	m_isValueDirty = true;
	return true;
}

void syn::UITextSlider::setValueFromString(const string& a_str) {
	RTMessage* msg = new RTMessage();
	PutArgs(&msg->data, a_str, m_unitId, m_paramId);
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
		string valStr;
		int unitId, paramId;
		GetArgs(a_data, 0, valStr, unitId, paramId);
		a_circuit->setInternalParameterFromString(unitId, paramId, valStr);
	};
	m_vm->queueAction(msg);
	m_isValueDirty = true;
}

void syn::UITextSlider::draw(NVGcontext* a_nvg) {
	if (!m_textBox->visible()) {
		const UnitParameter& param = m_vm->getUnit(m_unitId,m_vm->getNewestVoiceIndex()).getParameter(m_paramId);
		double value = m_param->getNorm();
		if (&param != m_param || value != m_value || m_isValueDirty) {
			_updateValue();
			m_isValueDirty = false;
		}

		nvgBeginPath(a_nvg);
		nvgFillColor(a_nvg, Color(Vector3f{ 0.4f,0.1f,0.7f }*0.25, 0.7f));
		nvgRect(a_nvg, 0, 0, size()[0], size()[1]);
		nvgFill(a_nvg);

		nvgFillColor(a_nvg, Color(Vector3f{ 0.4f,0.1f,0.7f }));
		nvgBeginPath(a_nvg);
		float fgWidth = size()[0] * m_value;
		nvgRect(a_nvg, 0, 0, fgWidth, size()[1]);
		nvgFill(a_nvg);
	}
}

void syn::UITextSlider::_updateMinSize() {
	setMinSize(m_row->minSize());
}

void syn::UITextSlider::_onResize() {
	m_textBox->setSize(size());
	m_row->setSize(size());
}

void syn::UITextSlider::_updateValue() {
	const UnitParameter& param = m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).getParameter(m_paramId);
	m_param = &param;
	m_nameLabel->setText(param.getName());
	m_unitsLabel->setText(param.getUnitsString());
	m_valueLabel->setText(param.getValueString());
	m_value = param.getNorm();
	m_textBox->setValue(m_valueLabel->text());
	m_textBox->setUnits(m_unitsLabel->text());
	_updateMinSize();
	_onResize();
}