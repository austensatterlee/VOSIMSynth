#include "UIParamControl.h"
#include "VoiceManager.h"
#include "UILabel.h"

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
	if(m_nameLabel)
		m_nameLabel->setText(param.getName());
	if(m_unitsLabel)
		m_unitsLabel->setText(param.getUnitsString());
	if(m_valueLabel)
		m_valueLabel->setText(param.getValueString());
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
