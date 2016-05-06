#include "UITextSlider.h"

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
		return true;
	}
	return false;
}

bool syn::UITextSlider::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	return true;
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
	return true;
}

Eigen::Vector2i syn::UITextSlider::calcAutoSize(NVGcontext* a_nvg) const {
	return{ m_autoWidth, size()[1] };
}

void syn::UITextSlider::draw(NVGcontext* a_nvg) {
	int textWidth = 0;
	string paramName = m_vm->getUnit(m_unitId).getParameter(m_paramId).getName();
	string valueStr = m_vm->getUnit(m_unitId).getParameter(m_paramId).getString();
	m_value = m_vm->getUnit(m_unitId).getParameter(m_paramId).getNorm();

	nvgSave(a_nvg);
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
	nvgFontSize(a_nvg, (float)size()[1]);
	nvgTextAlign(a_nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	textWidth += 5 + (int)nvgText(a_nvg, 0, 0, paramName.c_str(), NULL);

	nvgTextAlign(a_nvg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
	float bounds[4] = { 0,0,0,0 };
	nvgTextBounds(a_nvg, 0, 0, valueStr.c_str(), NULL, bounds);
	int textPosX = size()[0];
	nvgText(a_nvg, textPosX, 0, valueStr.c_str(), NULL);
	textWidth += 5 + bounds[2] - bounds[0];

	nvgRestore(a_nvg);

	m_autoWidth = textWidth;
}
