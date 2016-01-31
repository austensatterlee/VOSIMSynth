#include "UnitParameter.h"
#include "Unit.h"
#include "DSPMath.h"

syn::UnitParameter::UnitParameter(Unit* parent, string name, int id, IParam::EParamType ptype, double minValue, double maxValue, double defaultValue, double step, double shape, bool canEdit, bool canModulate):
	m_parent(parent),
	m_name(name),
	m_id(id),
	m_canEdit(canEdit),
	m_canModulate(canModulate),
	m_min(minValue),
	m_max(maxValue),
	m_default(defaultValue),
	m_step(step),
	m_shape(shape),
	m_connections(0)
{
	setType(ptype);
	m_offsetValue = m_default;
	m_value[0] = m_value[1] = m_offsetValue;
}

bool syn::UnitParameter::operator==(const UnitParameter& p) const {
	return m_id == p.m_id && m_name == p.m_name && getType() == p.getType() &&
			getMin() == p.getMin() && getMax() == p.getMax();
}

void syn::UnitParameter::mod(double amt, MOD_ACTION action) {
	if (action == SET) {
		m_offsetValue = CLAMP(amt,m_min,m_max);
		m_value[0] = m_value[1] = m_offsetValue;
		m_parent->onParamChange(this);
	} else if (action == SET_NORM) {
		m_offsetValue = FromNormalizedParam(amt, m_min, m_max, m_shape);
		m_offsetValue = CLAMP(m_offsetValue, m_min, m_max);
		m_value[0] = m_value[1] = m_offsetValue;
		m_parent->onParamChange(this);
	}
}

void syn::UnitParameter::mod(const double amt[2], MOD_ACTION action)
{
	 if (action == ADD && !(amt[0] == 0 && amt[1]==0)) {
		 m_value[0] += amt[0];
		 m_value[1] += amt[1];
	 }
	 else if (action == SCALE && !(amt[0] == 1 && amt[1] == 1)) {
		 m_value[0] *= amt[0];
		 m_value[1] *= amt[1];
	 }
}

void syn::UnitParameter::setType(IParam::EParamType new_type) {
	m_type = new_type;
	switch (m_type) {
	case IParam::kTypeBool:
		m_default = (m_default ? 1 : 0);
		setDisplayText(0, "off");
		setDisplayText(1, "on");
		m_max = 2;
	case IParam::kTypeEnum:
	case IParam::kTypeInt:
		m_step = 1.0;
		m_max = floor(m_max);
		m_min = floor(m_min);
	case IParam::kTypeDouble:
		m_displayPrecision = -log10(m_step);
		break;
	default: break;
	}
}

void syn::UnitParameter::setDisplayText(int value, const char* text) {
	int n = m_displayTexts.GetSize();
	m_displayTexts.Resize(n + 1);
	DisplayText* pDT = m_displayTexts.Get() + n;
	pDT->m_value = value;
	strncpy(pDT->m_text, text,MAX_PARAM_STR_LEN);
	if (m_type == IParam::kTypeEnum) {
		m_max = n;
	}
}

void syn::UnitParameter::getDisplayText(char* r_displayText) const {
	double value = static_cast<double>(*this);

	// Look for text that was mapped to the current value
	int n = m_displayTexts.GetSize();
	if (n) {
		DisplayText* pDT = m_displayTexts.Get();
		for (int i = 0; i < n; ++i , ++pDT) {
			if (value == pDT->m_value) {
				// return if we found some
				strncpy(r_displayText, pDT->m_text,MAX_PARAM_STR_LEN);
				return;
			}
		}
	}

	// Otherwise display the numerical value

	if (m_displayPrecision == 0) {
		sprintf(r_displayText, "%d", static_cast<int>(value));
	} else {
		double displayValue = value;
		sprintf(r_displayText, "%.*f", m_displayPrecision, displayValue);
	}
}

void syn::UnitParameter::addConnection(const vector<UnitSample>* srcbuffer, MOD_ACTION action) {
	m_connections.push_back({srcbuffer,action});
}

void syn::UnitParameter::removeConnection(const vector<UnitSample>* a_srcbuffer, MOD_ACTION a_action)
{
	Connection conn = { a_srcbuffer,a_action };
	vector<Connection>::iterator loc = find(m_connections.begin(), m_connections.end(), conn);
	if(loc!=m_connections.end())
		m_connections.erase(loc);
}

void syn::UnitParameter::pull(int bufind) {
	for (int i = 0; i < m_connections.size(); i++) {
		mod((*m_connections[i].srcbuffer)[bufind].output, m_connections[i].action);
	}
	m_parent->onParamChange(this);
}

