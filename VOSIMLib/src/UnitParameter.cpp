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

#include "UnitParameter.h"
#include <DSPMath.h>
#include <string>

namespace syn
{
	UnitParameter::UnitParameter() :
		m_name(""),
		m_value(0),
		m_prevValue(0),
		m_defaultValue(0),
		m_min(0),
		m_max(1),
		m_logMin(0),
		m_logRange(0),
		m_isVisible(true),
		m_type(Null),
		m_unitsType(None),
		m_controlType(Bounded),
		m_displayPrecision(0)
	{
	}

	UnitParameter::UnitParameter(const UnitParameter& a_other) :
		m_name(a_other.m_name),
		m_value(a_other.m_value),
		m_prevValue(a_other.m_prevValue),
		m_defaultValue(a_other.m_defaultValue),
		m_min(a_other.m_min),
		m_max(a_other.m_max),
		m_logMin(a_other.m_logMin),
		m_logRange(a_other.m_logRange),
		m_isVisible(true),
		m_type(a_other.m_type),
		m_unitsType(a_other.m_unitsType),
		m_controlType(Bounded),
		m_displayPrecision(a_other.m_displayPrecision),
		m_displayTexts(a_other.m_displayTexts)
	{
	}

	UnitParameter::UnitParameter(const string& a_name, bool a_defaultValue) :
		m_name(a_name),
		m_value(a_defaultValue),
		m_prevValue(a_defaultValue),
		m_defaultValue(a_defaultValue),
		m_min(0),
		m_max(1),
		m_logMin(0),
		m_logRange(0),
		m_isVisible(true),
		m_type(Bool),
		m_unitsType(None),
		m_controlType(Bounded),
		m_displayPrecision(0),
		m_displayTexts({ {0,"Off"},{1,"On"} }) { }

	UnitParameter::UnitParameter(const string& a_name, int a_min, int a_max,
		int a_defaultValue, EUnitsType a_unitsType) :
		m_name(a_name),
		m_value(a_defaultValue),
		m_prevValue(a_defaultValue),
		m_defaultValue(a_defaultValue),
		m_min(a_min),
		m_max(a_max),
		m_logMin(0),
		m_logRange(0),
		m_isVisible(true),
		m_type(Int),
		m_unitsType(a_unitsType),
		m_controlType(Bounded),
		m_displayPrecision(0) { }

	UnitParameter::UnitParameter(const string& a_name,
		const vector<string>& a_optionNames,
		const vector<double>& a_optionValues,
		int a_defaultOption, EUnitsType a_unitsType) :
		m_name(a_name),
		m_min(0),
		m_max(a_optionNames.size() - 1),
		m_logMin(0),
		m_logRange(0),
		m_isVisible(true),
		m_type(Enum),
		m_unitsType(a_unitsType),
		m_controlType(Bounded),
		m_displayPrecision(0)
	{
		m_defaultValue = a_defaultOption;
		m_value = m_prevValue = m_defaultValue;
		double optionValue;
		for (int i = 0; i < a_optionNames.size(); i++) {
			if (a_optionValues.empty()) {
				optionValue = i;
			}
			else {
				optionValue = a_optionValues[i];
			}
			m_displayTexts.push_back({ optionValue, a_optionNames[i] });
		}
	}

	UnitParameter::UnitParameter(const string& a_name, double a_min, double a_max,
		double a_defaultValue, EUnitsType a_unitsType, int a_displayPrecision) :
		m_name(a_name),
		m_value(a_defaultValue),
		m_prevValue(a_defaultValue),
		m_defaultValue(a_defaultValue),
		m_min(a_min),
		m_max(a_max),
		m_logMin(log(a_min)),
		m_logRange(log(a_max) - log(a_min)),
		m_isVisible(true),
		m_type(Double),
		m_unitsType(a_unitsType),
		m_controlType(Bounded),
		m_displayPrecision(a_displayPrecision)
	{ }

	void UnitParameter::reset() {
		m_value = m_defaultValue;
	}

	const string& UnitParameter::getName() const {
		return m_name;
	}

	UnitParameter::EParamType UnitParameter::getType() const {
		return m_type;
	}

	double UnitParameter::getMin() const {
		return m_min;
	}

	void UnitParameter::setMin(double a_new_min) {
		m_min = a_new_min;
		m_logMin = log(a_new_min);
		m_logRange = log(m_max) - m_logMin;
	}

	double UnitParameter::getMax() const {
		return m_max;
	}

	void UnitParameter::setMax(double a_new_max) {
		m_max = a_new_max;
		m_logRange = log(m_max) - m_logMin;
	}

	double UnitParameter::getDefaultValue() const {
		return m_defaultValue;
	}

	int UnitParameter::getPrecision() const {
		return m_displayPrecision;
	}

	void UnitParameter::setPrecision(int a_precision) {
		if (a_precision >= 0 && m_type == Double)
			m_displayPrecision = a_precision;
	}

	UnitParameter::EControlType UnitParameter::getControlType() const
	{
		return m_controlType;
	}

	void UnitParameter::setControlType(EControlType a_newControlType)
	{
		m_controlType = a_newControlType;
	}

	bool UnitParameter::isVisible() const
	{
		return m_isVisible;
	}

	void UnitParameter::setVisible(bool a_visible)
	{
		m_isVisible = a_visible;
	}

	bool UnitParameter::getBool() const {
		return m_value > 0.5;
	}

	bool UnitParameter::getPrevBool() const {
		return m_prevValue > 0.5;
	}

	bool UnitParameter::_setBool(bool a_value) {
		if (getBool() == a_value)
			return false;
		m_value = static_cast<double>(a_value);
		return true;
	}

	int UnitParameter::getInt() const {
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
			intvalue = (intvalue - m_prevValue >= 0.5) ? intvalue - 1 : intvalue;
		return intvalue;
	}

	bool UnitParameter::_setInt(int a_value) {
		if (a_value < m_min || a_value > m_max)
			return false;
		if (getInt() == a_value)
			return false;
		m_value = static_cast<double>(a_value);
		return true;
	}

	double UnitParameter::getDouble() const {
		return m_value;
	}

	double UnitParameter::getPrevDouble() const {
		return m_prevValue;
	}

	double UnitParameter::getEnum() const {
		int idx = getInt();
		return m_displayTexts[idx].m_value;
	}

	double UnitParameter::getEnum(int a_index) const
	{
		a_index = CLAMP(a_index, 0, m_displayTexts.size());
		return m_displayTexts[a_index].m_value;
	}

	double UnitParameter::getPrevEnum() const {
		int idx = getPrevInt();
		return m_displayTexts[idx].m_value;
	}

	bool UnitParameter::_setDouble(double a_value) {
		if (a_value < m_min || a_value > m_max)
			return false;
		if (m_value == a_value)
			return false;
		m_value = a_value;
		return true;
	}

	bool UnitParameter::set(double a_value) {
		m_prevValue = a_value;
		switch (m_type) {
		case Bool:
			return _setBool(a_value >= 0.5);
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

	double UnitParameter::getNorm() const {
		switch (m_unitsType) {
		case Freq:
		case Decibal:
			return (log(m_value) - m_logMin) / m_logRange;
		case None:
		default:
			return (m_value - m_min) / (m_max - m_min);
		}
	}

	bool UnitParameter::setNorm(double a_norm_value) {
		double value = CLAMP(a_norm_value, 0, 1);
		switch (m_unitsType) {
		case Freq:
		case Decibal:
			value = exp(value*m_logRange + m_logMin);
			break;
		case None:
		default:
			value = value * (m_max - m_min) + m_min;
			break;
		}
		return set(value);
	}

	bool UnitParameter::setFromString(const string& a_str) {
		if (getType() != Enum) {
			try {
				size_t num_digits;
				double value = stod(a_str, &num_digits);

				size_t decimal_pos;
				// adjust parameter precision
				decimal_pos = a_str.find_first_of(".");
				if (decimal_pos != string::npos) {
					int newParamPrecision = static_cast<int>(num_digits) - static_cast<int>(decimal_pos) - 1;
					setPrecision(newParamPrecision);
				}
				set(value);
				return true;
			}
			catch (std::invalid_argument&) {
				// Try to match string to an enum option
				int i = 0;
				for (const DisplayText& disp : m_displayTexts) {
					if (disp.m_text == a_str) {
						_setInt(i);
						return true;
					}
					i++;
				}
			}
		}
		else
		{
			// Try to match string to an enum option
			int i = 0;
			for (const DisplayText& disp : m_displayTexts) {
				if (disp.m_text == a_str) {
					_setInt(i);
					return true;
				}
				i++;
			}
		}
		return false;
	}

	string UnitParameter::getValueString() const {
		int idx = getInt();
		// Check for a matching display text
		if (idx >= 0 && idx < m_displayTexts.size()) {
			return m_displayTexts[idx].m_text;
		}

		// If no display text is found, return numeral

		char displaytext[MAX_PARAM_STR_LEN];
		if (getType() != Double) {
			snprintf(displaytext, MAX_PARAM_STR_LEN, "%d", idx);
		}
		else {
			double displayValue = getDouble();
			// if precision is set, print out that many digits
			if (getPrecision() > 0) {
				snprintf(displaytext, MAX_PARAM_STR_LEN, "%.*f", m_displayPrecision, displayValue);
			}
			else {
				snprintf(displaytext, MAX_PARAM_STR_LEN, "%g", displayValue);
			}
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
		case None:
		default:
			units = "";
			break;
		}
		return units;
	}
}