#include "UnitParameter.h"
#include "Unit.h"
#include "DSPMath.h"

syn::UnitParameter::UnitParameter(Unit* parent, string name, int id, IParam::EParamType ptype, double minValue, double maxValue, double defaultValue, double step, double shape, bool isHidden, bool canModulate): 
	m_parent(parent),
	m_name(name),
	m_id(id),
	m_type(ptype),
	m_isHidden(isHidden),
	m_canModulate(canModulate),
	m_min(minValue),
	m_max(maxValue),
	m_default(defaultValue),
	m_step(step),
	m_shape(shape),
	m_connections(0)
{
	switch(m_type) {
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
	mod(m_default, SET);
}

bool syn::UnitParameter::operator==(const UnitParameter& p) const {
	return m_id == p.m_id && m_name == p.m_name && getType() == p.getType() &&
		   getMin() == p.getMin() && getMax() == p.getMax();
}

void syn::UnitParameter::mod(double amt, MOD_ACTION action) {
	if (action == SET) {
		m_offsetValue = CLAMP(amt,m_min,m_max);
		m_value = m_offsetValue;
		m_isDirty = true;
	} else if (action == SET_NORM) {
		m_offsetValue = FromNormalizedParam(amt, m_min, m_max, m_shape);
		m_offsetValue = CLAMP(m_offsetValue, m_min, m_max);
		m_value = m_offsetValue;
		m_isDirty = true;
	}	
	else if (action == ADD && amt != 0) {
		m_value += amt;
		m_isDirty = true;
	}
	else if (action == SCALE && amt != 1) {
		m_value *= amt;
		m_isDirty = true;
	}
}

void syn::UnitParameter::setDisplayText(int value, const char* text) {
	int n = m_displayTexts.GetSize();
	m_displayTexts.Resize(n + 1);
	DisplayText* pDT = m_displayTexts.Get() + n;
	pDT->m_value = value;
	strncpy(pDT->m_text,text,MAX_PARAM_STR_LEN);
	if(m_type==IParam::kTypeEnum) {
		m_max = n;
	}
}

void syn::UnitParameter::getDisplayText(char* r_displayText, bool normalized) const {
	int value = m_value;
	if (normalized) value = FromNormalizedParam(value, m_min, m_max, m_shape);

	// Look for text that was mapped to the current value
	int n = m_displayTexts.GetSize();
	if (n)
	{
		DisplayText* pDT = m_displayTexts.Get();
		for (int i = 0; i < n; ++i, ++pDT)
		{
			if (value == pDT->m_value)
			{
				// return if we found some
				strncpy(r_displayText,pDT->m_text,MAX_PARAM_STR_LEN);
				return;
			}
		}
	}

	// Otherwise display the numerical value
	double displayValue = m_value;

	if (m_displayPrecision == 0)
	{
		sprintf(r_displayText, "%d", int(displayValue));
	}
	else
	{
		sprintf(r_displayText, "%.*f", m_displayPrecision, displayValue);
	}
}

void syn::UnitParameter::addConnection(const vector<double>* srcbuffer, MOD_ACTION action) {
	m_connections.push_back({srcbuffer,action});
}

void syn::UnitParameter::pull(int bufind) {
	for (int i = 0; i < m_connections.size(); i++) {
		mod((*m_connections[i].srcbuffer)[bufind], m_connections[i].action);
	}
}

