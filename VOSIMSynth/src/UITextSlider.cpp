#include "UITextSlider.h"

syn::UITextSlider::UITextSlider(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_paramId):
	UIComponent{a_window},
	m_value(0.0),
	m_vm(a_vm),
	m_unitId(a_unitId),
	m_paramId(a_paramId) 
{
	m_paramName = m_vm->getUnit(m_unitId).getParameter(m_paramId).getName();
	m_textBox = new UITextBox(m_window, m_valueStr);
	m_textBox->setVisible(false);
	m_textBox->setCallback([&](const string& a_str)->bool {return setValueFromString(a_str); m_textBox->setVisible(false); });
	m_textBox->setFontSize(theme()->mTextSliderFontSize);
	addChild(m_textBox);
	_updateValue();
}

bool syn::UITextSlider::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
		const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
		m_value = param.getNorm();
		double targetValue = (a_relCursor[0] - m_pos[0]) * (1.0 / size()[0]);
		double error = m_value - targetValue;
		double adjust_speed = 0.5;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
			adjust_speed *= 0.1;
		m_value = m_value - adjust_speed * error;

		ActionMessage* msg = new ActionMessage();
		msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
		{
			double currValue;
			int unitId, paramId;
			int pos = 0;
			pos = a_data->Get<int>(&unitId, pos);
			pos = a_data->Get<int>(&paramId, pos);
			pos = a_data->Get<double>(&currValue, pos);
			a_circuit->setInternalParameterNorm(unitId, paramId, currValue);
		};
		msg->data.Put<int>(&m_unitId);
		msg->data.Put<int>(&m_paramId);
		msg->data.Put<double>(&m_value);
		m_vm->queueAction(msg);

		m_isValueDirty = true;

		return true;
	}
	return false;
}

syn::UIComponent* syn::UITextSlider::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* retval = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (retval) return retval;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
		const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
		m_textBox->setVisible(true);
		m_textBox->setValue(param.getString());
		m_textBox->setSize(size());		
		m_window->setFocus(m_textBox);
		return m_textBox;
	}
	return this;
}

bool syn::UITextSlider::onMouseScroll(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, int a_scrollAmt) {
	a_scrollAmt = CLAMP(a_scrollAmt, -1, 1);

	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);

	double scale;
	if (param.getType() != Double) {
		scale = 1.0;
	}
	else {
		scale = pow(10, -param.getPrecision() + 1);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
			scale *= 10;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
			scale *= 0.1;
		}
	}

	double currValue = param.get<double>() + scale * a_scrollAmt;
	ActionMessage* msg = new ActionMessage();
	msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
	{
		double currValue;
		int unitId, paramId;
		int pos = 0;
		pos = a_data->Get<int>(&unitId, pos);
		pos = a_data->Get<int>(&paramId, pos);
		pos = a_data->Get<double>(&currValue,pos);
		a_circuit->setInternalParameter(unitId, paramId, currValue);
	};
	msg->data.Put<int>(&m_unitId);
	msg->data.Put<int>(&m_paramId);
	msg->data.Put<double>(&currValue);	
	m_vm->queueAction(msg);

	m_isValueDirty = true;

	return true;
}

bool syn::UITextSlider::setValueFromString(const string& a_str) {
	m_textBox->setVisible(false);
	try {
		size_t num_digits;
		double value = std::stod(a_str, &num_digits);

		size_t decimal_pos;
		// adjust parameter precision
		decimal_pos = a_str.find_first_of(".");
		if (decimal_pos != string::npos) {
			int newParamPrecision = static_cast<int>(num_digits) - static_cast<int>(decimal_pos) - 1;

			ActionMessage* msg = new ActionMessage();
			msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
			{
				int newPrecision;
				int unitId, paramId;
				int pos = 0;
				pos = a_data->Get<int>(&unitId, pos);
				pos = a_data->Get<int>(&paramId, pos);
				pos = a_data->Get<int>(&newPrecision, pos);
				a_circuit->setInternalParameterPrecision(unitId,paramId,newPrecision);
			};
			msg->data.Put<int>(&m_unitId);
			msg->data.Put<int>(&m_paramId);
			msg->data.Put<int>(&newParamPrecision);
			m_vm->queueAction(msg);
		}

		ActionMessage* msg = new ActionMessage();
		msg->action = [](Circuit* a_circuit, bool a_isLast, ByteChunk* a_data)
		{
			double currValue;
			int unitId, paramId;
			int pos = 0;
			pos = a_data->Get<int>(&unitId, pos);
			pos = a_data->Get<int>(&paramId, pos);
			pos = a_data->Get<double>(&currValue, pos);
			a_circuit->setInternalParameter(unitId, paramId, currValue);
		};
		msg->data.Put<int>(&m_unitId);
		msg->data.Put<int>(&m_paramId);
		msg->data.Put<double>(&value);
		m_vm->queueAction(msg);

		m_isValueDirty = true;

		return true;
	}
	catch (std::invalid_argument&) {
		return false;
		// do nothing
	}
}

void syn::UITextSlider::draw(NVGcontext* a_nvg) {
	if (!m_textBox->visible()) {
		if(m_isValueDirty) {
			_updateValue();
		}

		nvgBeginPath(a_nvg);
		nvgFillColor(a_nvg, Color(Vector3f{ 0.0f,0.0f,0.0f }));
		nvgRect(a_nvg, 0, 0, size()[0], size()[1]);
		nvgFill(a_nvg);

		nvgFillColor(a_nvg, Color(Vector3f{ 0.4f,0.1f,0.7f }));
		nvgBeginPath(a_nvg);
		float fgWidth = size()[0] * m_value;
		nvgRect(a_nvg, 0, 0, fgWidth, size()[1]);
		nvgFill(a_nvg);

		nvgFillColor(a_nvg, Color(Vector3f{ 1.0f,1.0f,1.0f }));
		nvgFontSize(a_nvg, theme()->mTextSliderFontSize);
		nvgTextAlign(a_nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgText(a_nvg, 0, 0, m_paramName.c_str(), NULL);

		nvgTextAlign(a_nvg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
		nvgText(a_nvg, size()[0], 0, m_valueStr.c_str(), NULL);
	}
}

void syn::UITextSlider::_updateMinSize() {
	int textWidth = 0;
	float bounds[4];
	NVGcontext* nvg = m_window->getContext();
	nvgFontSize(nvg, theme()->mTextSliderFontSize);
	nvgTextAlign(nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	// Calculate size of name text
	nvgTextBounds(nvg, 0, 0, m_paramName.c_str(), NULL, bounds);
	textWidth += 5 + bounds[2] - bounds[0];
	// Calculate size of value text
	nvgTextBounds(nvg, 0, 0, m_valueStr.c_str(), NULL, bounds);
	textWidth += 5 + bounds[2] - bounds[0];

	Vector2i minsize = { textWidth,-1 };

	setMinSize_(minsize.cwiseMax(m_textBox->minSize()));
}

void syn::UITextSlider::_onResize() {
	m_textBox->setSize(size());
}

void syn::UITextSlider::_updateValue() {
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	double value = param.getNorm();
	string valueStr = param.getString();
	if (value == m_value && valueStr == m_valueStr)
		return;
	m_value = value;
	m_valueStr = valueStr;
	m_isValueDirty = false;
	_updateMinSize();
}
