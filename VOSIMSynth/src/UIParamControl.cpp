#include "UIParamControl.h"
#include "VoiceManager.h"
#include "UILabel.h"
#include <UITextBox.h>
#include <Theme.h>

synui::UIParamControl::UIParamControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId, int a_paramId) :
	UIComponent(a_window),
	m_vm(a_vm),
	m_unitId(a_unitId),
	m_paramId(a_paramId), 
	m_param(nullptr),
	m_nameLabel(nullptr),
	m_valueLabel(nullptr),
	m_unitsLabel(nullptr),
	m_normValue(0.0),
	m_isValueDirty(true)
{
	m_textBox = new UITextBox(m_window);
	m_textBox->setVisible(false);
	m_textBox->setCallback([&](const string& a_str)-> bool {
		setParamFromString(a_str);
		m_textBox->setVisible(false);
		for(auto child : m_children)
			if(child.get()!=m_textBox) child->setVisible(true);
		return true;
	});
	m_textBox->setFontSize(theme()->mTextSliderFontSize);
	addChild(m_textBox);
}

void synui::UIParamControl::setParamValue(double a_val) {
	syn::RTMessage* msg = new syn::RTMessage();
	msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
		double val;
		int unitId, paramId;
		GetArgs(a_data, 0, unitId, paramId, val);
		a_circuit->setInternalParameter(unitId, paramId, val);
	};
	PutArgs(&msg->data, m_unitId, m_paramId, a_val);
	m_vm->queueAction(msg);
	m_isValueDirty = true;
}

void synui::UIParamControl::setParamNorm(double a_normval) {
	syn::RTMessage* msg = new syn::RTMessage();
	msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
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
	msg->data.Put<double>(&a_normval);
	m_vm->queueAction(msg);
	m_isValueDirty = true;
}

void synui::UIParamControl::nudgeParam(double a_logScale, double a_linScale) {
	syn::RTMessage* msg = new syn::RTMessage();
	PutArgs(&msg->data, m_unitId, m_paramId, a_logScale, a_linScale);
	msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
		double logScale, linScale;
		int unitId, paramId;
		GetArgs(a_data, 0, unitId, paramId, logScale, linScale);
		a_circuit->getUnit(unitId).getParameter(paramId).nudge(logScale, linScale);
	};
	m_vm->queueAction(msg);
	m_isValueDirty = true;
}

synui::UIComponent* synui::UIParamControl::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* ret = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (ret) return ret;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {	
		m_textBox->setVisible(true);
		for (auto child : m_children) {
			if(child.get()!=m_textBox) child->setVisible(false);
		}
		m_window->setFocus(m_textBox);
		return m_textBox->onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	}
	return nullptr;
}

void synui::UIParamControl::setParamFromString(const string& a_str) {
	syn::RTMessage* msg = new syn::RTMessage();
	PutArgs(&msg->data, a_str, m_unitId, m_paramId);
	msg->action = [](syn::Circuit* a_circuit, bool a_isLast, ByteChunk* a_data) {
		string valStr;
		int unitId, paramId;
		GetArgs(a_data, 0, valStr, unitId, paramId);
		a_circuit->setInternalParameterFromString(unitId, paramId, valStr);
	};
	m_vm->queueAction(msg);
	m_isValueDirty = true;
}

void synui::UIParamControl::updateValue_() {
	const syn::UnitParameter& param = m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).getParameter(m_paramId);
	m_param = &param;
	string valueStr = param.getValueString();
	if(m_nameLabel)
		m_nameLabel->setText(param.getName());
	if(m_unitsLabel)
		m_unitsLabel->setText(param.getUnitsString());
	if(m_valueLabel)
		m_valueLabel->setText(valueStr);
	if (m_textBox)
		m_textBox->setValue(valueStr);
	m_normValue = param.getNorm();
}

void synui::UIParamControl::draw(NVGcontext* a_nvg) {
	const syn::UnitParameter& param = m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()).getParameter(m_paramId);
	double normValue = m_param->getNorm();
	if (&param != m_param || normValue != m_normValue || m_isValueDirty) {
		updateValue_();
		m_isValueDirty = false;
	}
}
