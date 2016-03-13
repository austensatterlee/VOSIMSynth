/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "UnitParameter.h"
#include <DSPMath.h>

namespace syn {
    UnitParameter::UnitParameter() :
            m_name(""),
		    m_value(0), 
			m_prevValue(0), 
			m_defaultValue(0),
		    m_min(0),
		    m_max(1),
            m_type(Null),
            m_displayPrecision(0)
    {
    }

    UnitParameter::UnitParameter(const string &a_name, bool a_defaultValue) :
            m_name(a_name),
            m_value(a_defaultValue),
			m_prevValue(a_defaultValue),
            m_defaultValue(a_defaultValue),
            m_min(0),
            m_max(1),
            m_type(Bool),
            m_displayPrecision(0),
            m_displayTexts({{0,"Off"},{1,"On"}})
    {
    }

    UnitParameter::UnitParameter(const string &a_name, int a_min, int a_max,
                                 int a_defaultValue) :
            m_name(a_name),
            m_value(a_defaultValue),
			m_prevValue(a_defaultValue),
            m_defaultValue(a_defaultValue),
            m_min(a_min),
            m_max(a_max),
            m_type(Int),
            m_displayPrecision(0)
    {
    }

    UnitParameter::UnitParameter(const string &a_name,
                                 const vector<string> &a_optionNames) :
            m_name(a_name),
            m_min(0),
            m_max(a_optionNames.size()-1),
            m_type(Enum),
            m_displayPrecision(0)
    {
		m_defaultValue = 0;
		m_value = m_prevValue = m_defaultValue;
        for (int i = 0; i < a_optionNames.size(); i++) {
            m_displayTexts.push_back({i, a_optionNames[i]});
        }
    }

    UnitParameter::UnitParameter(const string &a_name, double a_min, double a_max,
                                 double a_defaultValue, int a_displayPrecision) :
            m_name(a_name),
            m_value(a_defaultValue),
			m_prevValue(a_defaultValue),
            m_defaultValue(a_defaultValue),
            m_min(a_min),
            m_max(a_max),
            m_type(Double),
            m_displayPrecision(a_displayPrecision)
    {

    }

    void UnitParameter::reset()
    {
        m_value = m_defaultValue;
    }

    const string &UnitParameter::getName() const
    {
        return m_name;
    }

    EParamType UnitParameter::getType() const
    {
        return m_type;
    }

    double UnitParameter::getMin() const
    {
        return m_min;
    }

    void UnitParameter::setMin(double a_new_min)
    {
        m_min = a_new_min;
    }

    double UnitParameter::getMax() const
    {
        return m_max;
    }

    void UnitParameter::setMax(double a_new_max)
    {
        m_max = a_new_max;
    }

    double UnitParameter::getDefaultValue() const
    {
        return m_defaultValue;
    }

	int UnitParameter::getPrecision() const {
		return m_displayPrecision;
    }

	void UnitParameter::setPrecision(int a_precision) {
		if (a_precision >= 0 && m_type==Double)
			m_displayPrecision = a_precision;
    }

	bool UnitParameter::getBool() const
    {
        return m_value > 0.5;
    }

	bool UnitParameter::getPrevBool() const {
		return m_prevValue > 0.5;
    }

	bool UnitParameter::_setBool(bool a_value)
    {
        m_value = static_cast<double>(a_value);
        return true;
    }

    int UnitParameter::getInt() const
    {
        int intvalue = static_cast<int>(m_value);
        // round upwards
		if (m_value > 0)
			intvalue = (m_value - intvalue >= 0.5) ? intvalue + 1 : intvalue;
		else
			intvalue = (intvalue - m_value >= 0.5) ? intvalue - 1 : intvalue;
        return intvalue;
    }

	int UnitParameter::getPrevInt() const {
		int intvalue = static_cast<int>(m_prevValue);
		// round upwards
		if (m_prevValue > 0)
			intvalue = (m_prevValue - intvalue >= 0.5) ? intvalue + 1 : intvalue;
		else
			intvalue = (intvalue - m_value >= 0.5) ? intvalue - 1 : intvalue;
		return intvalue;
    }

	bool UnitParameter::_setInt(int a_value)
    {
        if(a_value < m_min || a_value > m_max)
            return false;
        m_value = static_cast<double>(a_value);
        return true;
    }

    double UnitParameter::getDouble() const
    {
        return m_value;
    }

	double UnitParameter::getPrevDouble() const {
		return m_prevValue;
    }

	bool UnitParameter::_setDouble(double a_value)
    {
        if(a_value < m_min || a_value > m_max)
            return false;
        m_value = a_value;
        return true;
    }

	bool UnitParameter::set(double a_value) {
		m_prevValue = m_value;
	    switch(m_type) {
	    case Bool:
			return _setBool(a_value);
	    case Enum:
	    case Int:
			return _setInt(a_value);
	    case Double: 
			return _setDouble(a_value);
		case Null:
	    default: 
			return false;
	    }
	}

    double UnitParameter::getNorm() const
    {
        return (m_value - m_min)/(m_max - m_min);
    }

    bool UnitParameter::setNorm(double a_norm_value)
    {
		m_prevValue = m_value;
		a_norm_value = CLAMP(a_norm_value, 0, 1);
        m_value = a_norm_value*(m_max - m_min) + m_min;
        return true;
    }

    string UnitParameter::getString() const
    {
        int idx = getInt();
        // Search for a matching display text
        for(int i=0;i<m_displayTexts.size();i++){
            if (idx == m_displayTexts[i].m_value){
                return m_displayTexts[i].m_text;
            }
        }

        // If no display text is found, return numeral
        char displaytext[MAX_PARAM_STR_LEN];
        if (m_displayPrecision == 0) {
            snprintf(displaytext, MAX_PARAM_STR_LEN, "%d", idx);
        } else {
            double displayValue = getDouble();
            snprintf(displaytext, MAX_PARAM_STR_LEN, "%.*f", m_displayPrecision, displayValue);
        }
        return string(displaytext);
    }
}
