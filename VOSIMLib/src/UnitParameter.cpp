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
#include "vosimlib/UnitParameter.h"
#include <vosimlib/DSPMath.h>
#include <string>
#include <vosimlib/Unit.h>

namespace syn {
    UnitParameter::UnitParameter()
        :
        m_name(""),
        m_id(-1),
        m_value(0),
        m_defaultValue(0),
        m_min(0),
        m_max(1),
        m_logMin(0),
        m_logRange(0),
        m_isVisible(true),
        m_type(Null),
        m_unitsType(None),
        m_controlType(Bounded),
        m_displayPrecision(0),
        m_parent(nullptr) { }

    UnitParameter::UnitParameter(const string& a_name, bool a_defaultValue)
        :
        UnitParameter(a_name, {"Off","On"}, {0,1}, 0) { m_type = Bool; }

    UnitParameter::UnitParameter(const string& a_name, int a_min, int a_max,
                                 int a_defaultValue, EUnitsType a_unitsType)
        :
        m_name(a_name),
        m_id(-1),
        m_value(a_defaultValue),
        m_defaultValue(a_defaultValue),
        m_min(a_min),
        m_max(a_max),
        m_logMin(0),
        m_logRange(0),
        m_isVisible(true),
        m_type(Int),
        m_unitsType(a_unitsType),
        m_controlType(Bounded),
        m_displayPrecision(0),
        m_parent(nullptr) { }

    UnitParameter::UnitParameter(const string& a_name,
                                 const vector<string>& a_optionNames,
                                 const vector<double>& a_optionValues,
                                 int a_defaultOption, EUnitsType a_unitsType)
        :
        m_name(a_name),
        m_id(-1),
        m_min(0),
        m_max(a_optionNames.size() - 1),
        m_logMin(0),
        m_logRange(0),
        m_isVisible(true),
        m_type(Enum),
        m_unitsType(a_unitsType),
        m_controlType(Bounded),
        m_displayPrecision(0),
        m_parent(nullptr) {
        m_defaultValue = a_defaultOption;
        m_value = m_defaultValue;
        double optionValue;
        for (int i = 0; i < a_optionNames.size(); i++) {
            if (a_optionValues.empty()) { optionValue = i; } else { optionValue = a_optionValues[i]; }
            m_displayTexts.push_back({optionValue, a_optionNames[i]});
        }
    }

    UnitParameter::UnitParameter(const string& a_name, double a_min, double a_max,
                                 double a_defaultValue, EUnitsType a_unitsType, int a_displayPrecision)
        :
        m_name(a_name),
        m_id(-1),
        m_value(a_defaultValue),
        m_defaultValue(a_defaultValue),
        m_min(a_min),
        m_max(a_max),
        m_logMin(log10(a_min)),
        m_logRange(log10(a_max) - log10(a_min)),
        m_isVisible(true),
        m_type(Double),
        m_unitsType(a_unitsType),
        m_controlType(Bounded),
        m_displayPrecision(a_displayPrecision),
        m_parent(nullptr) { }

    void UnitParameter::reset() { m_value = m_defaultValue; }

    const string& UnitParameter::getName() const { return m_name; }

    int UnitParameter::getId() const { return m_id; }

    void UnitParameter::setId(int a_id) { m_id = a_id; }

    Unit* UnitParameter::getParent() const { return m_parent; }

    void UnitParameter::setParent(Unit* a_newParent) { m_parent = a_newParent; }

    UnitParameter::EParamType UnitParameter::getType() const { return m_type; }

    double UnitParameter::getMin() const { return m_min; }

    void UnitParameter::setMin(double a_new_min) {
        m_min = a_new_min;
        m_logMin = log10(a_new_min);
        m_logRange = log10(m_max) - m_logMin;
        set(m_value);
    }

    double UnitParameter::getMax() const { return m_max; }

    void UnitParameter::setMax(double a_new_max) {
        m_max = a_new_max;
        m_logRange = log10(m_max) - m_logMin;
        set(m_value);
    }

    double UnitParameter::getDefaultValue() const { return m_defaultValue; }

    int UnitParameter::getPrecision() const { return m_displayPrecision; }

    bool UnitParameter::setPrecision(int a_precision) {
        if (a_precision >= 0 && m_type == Double && a_precision != m_displayPrecision) {
            m_displayPrecision = a_precision;
            return true;
        }
        return false;
    }

    UnitParameter::EControlType UnitParameter::getControlType() const { return m_controlType; }

    UnitParameter& UnitParameter::setControlType(EControlType a_newControlType) {
        m_controlType = a_newControlType;
        return * this;
    }

    UnitParameter& UnitParameter::setUnitsType(EUnitsType a_newUnitsType) {
        m_unitsType = a_newUnitsType;
        return * this;
    }

    bool UnitParameter::isVisible() const { return m_isVisible; }

    UnitParameter& UnitParameter::setVisible(bool a_visible) {
        m_isVisible = a_visible;
        return * this;
    }

    bool UnitParameter::getBool() const { return m_value > 0.5; }

    int UnitParameter::getInt() const {
        int intvalue = static_cast<int>(m_value);
        // round upwards
        if (m_value > 0)
            intvalue = (m_value - intvalue >= 0.5) ? intvalue + 1 : intvalue;
        else
            intvalue = (m_value - intvalue <= -0.5) ? intvalue - 1 : intvalue;
        return intvalue;
    }

    double UnitParameter::getDouble() const { return m_value; }

    double UnitParameter::getEnum() const {
        int idx = getInt();
        return m_displayTexts[idx].m_value;
    }

    double UnitParameter::getEnum(int a_index) const {
        a_index = CLAMP<int>(a_index, 0, static_cast<int>(m_displayTexts.size() - 1));
        return m_displayTexts[a_index].m_value;
    }

    bool UnitParameter::set(double a_value) {
        a_value = CLAMP<double>(a_value, m_min, m_max);
        if (m_value != a_value) {
            m_value = a_value;
            if (m_parent) { m_parent->notifyParameterChanged(m_id); }
            return true;
        }
        return false;
    }

    double UnitParameter::getNorm() const {
        return getNorm(m_value);
    }

    double UnitParameter::getNorm(double a_value) const {
        switch (m_unitsType) {
        case Freq:
        case Decibal:
            if(m_logMin<=0)
                return (a_value - m_min) / (m_max - m_min);
            else
                return (log10(a_value) - m_logMin) / m_logRange;
        case None:
        default:
            return (a_value - m_min) / (m_max - m_min);
        }
    }

    bool UnitParameter::setNorm(double a_norm_value) {
        double value = CLAMP<double>(a_norm_value, 0, 1);
        switch (m_unitsType) {
            case Freq:
            case Decibal:
                if(m_logMin<=0)
                    value = value * (m_max - m_min) + m_min;
                else
                    value = pow(10.0, value * m_logRange + m_logMin);
                break;
            case None:
            default:
                value = value * (m_max - m_min) + m_min;
                break;
        }
        return set(value);
    }

    bool UnitParameter::setFromString(const string& a_str) {
        if (getType() != Enum && getType() != Bool ) {
            try {
                size_t num_digits;
                double value = stod(a_str, &num_digits);

                // adjust parameter precision
                size_t decimal_pos = a_str.find_first_of(".");
                if (decimal_pos != string::npos) {
                    int newParamPrecision = static_cast<int>(num_digits) - static_cast<int>(decimal_pos) - 1;
                    setPrecision(newParamPrecision);
                } else { setPrecision(0); }
                return set(value);
            } catch (std::invalid_argument&) {
                // Try to match string to an enum option
                int i = 0;
                for (const DisplayText& disp : m_displayTexts) {
                    if (disp.m_text == a_str) { return set(i); }
                    i++;
                }
            }
        } else {
            // Try to match string to an enum option
            int i = 0;
            for (const DisplayText& disp : m_displayTexts) {
                if (disp.m_text == a_str) { return set(i); }
                i++;
            }
        }
        return false;
    }

    bool UnitParameter::nudge(double a_logAmt, double a_linAmt) {
        double shiftamt = pow(10.0, -m_displayPrecision + a_logAmt) * a_linAmt;
        if (m_type == Enum)
            return set(getInt() + shiftamt);
        return set(get<double>() + shiftamt);
    }

    string UnitParameter::getValueString() const {
        int idx = getInt();
        // Check for a matching display text
        if (idx >= 0 && idx < m_displayTexts.size()) { return m_displayTexts[idx].m_text; }

        // If no display text is found, return numeral

        char displaytext[MAX_PARAM_STR_LEN];
        if (getType() != Double) { snprintf(displaytext, MAX_PARAM_STR_LEN, "%d", idx); } else {
            double displayValue = getDouble();
            // if precision is set, print out that many digits
            if (getPrecision() > 0) { snprintf(displaytext, MAX_PARAM_STR_LEN, "%.*f", m_displayPrecision, displayValue); } else { snprintf(displaytext, MAX_PARAM_STR_LEN, "%g", displayValue); }
        }
        return string(displaytext);
    }

    string UnitParameter::getUnitsString() const {
        string units;
        switch (m_unitsType) {
            case Freq:
                units = "Hz";
                break;
            case BPM:
                units = "BPM";
                break;
            case Semitones:
                units = "semi";
                break;
            case Octaves:
                units = "oct";
                break;
            case Seconds:
                units = "s";
                break;
            case Decibal:
                units = "dB";
                break;
            case Samples:
                units = "smpls";
                break;
            case None:
            default:
                units = "";
                break;
        }
        return units;
    }

    const vector<UnitParameter::DisplayText>& UnitParameter::getDisplayTexts() const { return m_displayTexts; }

    UnitParameter& UnitParameter::load(const json& j) {
        setFromString(j["value"].get<string>());
        m_type = static_cast<EParamType>(j["data_type"].get<int>());
        m_displayTexts.clear();
        for (const json& k : j["display_texts"]) { m_displayTexts.push_back(DisplayText{k}); }
        m_min = j["min"];
        m_max = j["max"];
        return *this;
    }

    UnitParameter::operator json() const {
        json j;
        j["value"] = getValueString();
        j["data_type"] = m_type;
        j["display_texts"] = m_displayTexts;
        j["min"] = m_min;
        j["max"] = m_max;
        return j;
    }
}
