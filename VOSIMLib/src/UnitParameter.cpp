#include <stdio.h>
#include "UnitParameter.h"

namespace syn {
    UnitParameter::UnitParameter() :
            m_name(""),
            m_type(Null),
            m_displayPrecision(0)
    {
    }

    UnitParameter::UnitParameter(const string &a_name, bool a_defaultValue) :
            m_name(a_name),
            m_type(Bool),
            m_min(0),
            m_max(1),
            m_defaultValue(a_defaultValue),
            m_value(a_defaultValue),
            m_displayPrecision(0),
            m_displayTexts({{0,"Off"},{1,"On"}})
    {
    }

    UnitParameter::UnitParameter(const string &a_name, int a_min, int a_max,
                                 int a_defaultValue) :
            m_name(a_name),
            m_type(Int),
            m_min(a_min),
            m_max(a_max),
            m_defaultValue(a_defaultValue),
            m_value(a_defaultValue),
            m_displayPrecision(0)
    {
    }

    UnitParameter::UnitParameter(const string &a_name,
                                 const vector<string> &a_optionNames) :
            m_name(a_name),
            m_type(Enum),
            m_min(0),
            m_max(a_optionNames.size()),
            m_defaultValue(2),
            m_value(2),
            m_displayPrecision(0)
    {
        for (int i = 0; i < a_optionNames.size(); i++) {
            m_displayTexts.push_back({i, a_optionNames[i]});
        }
    }

    UnitParameter::UnitParameter(const string &a_name, double a_min, double a_max,
                                 double a_defaultValue) :
            m_name(a_name),
            m_type(Double),
            m_min(a_min),
            m_max(a_max),
            m_defaultValue(a_defaultValue),
            m_value(a_defaultValue),
            m_displayPrecision(3)
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

    bool UnitParameter::getBool() const
    {
        return m_value < 0.5;
    }

    bool UnitParameter::set(bool a_value)
    {
        if(a_value < m_min || a_value > m_max)
            return false;
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

    bool UnitParameter::set(int a_value)
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

    bool UnitParameter::set(double a_value)
    {
        if(a_value < m_min || a_value > m_max)
            return false;
        m_value = a_value;
        return true;
    }

    double UnitParameter::getNorm() const
    {
        return (m_value - m_min)/(m_max - m_min);
    }

    bool UnitParameter::setNorm(double a_norm_value)
    {
        if(a_norm_value < 0 || a_norm_value > 1)
            return false;
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