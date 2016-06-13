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

#ifndef __UNITPARAMETER__
#define __UNITPARAMETER__

#include <vector>
#include <map>

using std::string;
using std::map;
using std::vector;

#define MAX_PARAM_STR_LEN 32

namespace syn
{
	class UnitParameter
	{
	public:
		enum EParamType
		{
			Null,
			Bool,
			Enum,
			Int,
			Double
		};

		enum EUnitsType
		{
			None,
			Freq,
			BPM,
			Semitones,
			Octaves,
			Seconds,
			Decibal
		};

		enum EControlType
		{
			Bounded,
			Unbounded
		};
	public:
		UnitParameter();
		UnitParameter(const string& a_name, bool a_defaultValue);
		UnitParameter(const string& a_name, int a_min, int a_max, int a_defaultValue, EUnitsType a_unitsType = None);
		UnitParameter(const string& a_name, const vector<string>& a_optionNames, const vector<double>& a_optionValues = {}, int a_defaultOption = 0, EUnitsType a_unitsType = None);
		UnitParameter(const string& a_name, double a_min, double a_max, double a_defaultValue, EUnitsType m_unitsType = None, int a_displayPrecision = 2);

		/**
		 * Reset the parameter to its default value
		 */
		void reset();

		const string& getName() const;

		EParamType getType() const;

		double getMin() const;

		void setMin(double a_new_min);

		double getMax() const;

		void setMax(double a_new_max);

		double getDefaultValue() const;

		int getPrecision() const;
		void setPrecision(int a_precision);

		EControlType getControlType() const;
		void setControlType(EControlType a_newControlType);

		bool isVisible() const;

		void setVisible(bool a_visible);

		/**
		 * \return true if the parameter value was changed
		 */
		bool set(double a_value);
		/**
		 * Set the parameter from a number in the range (0,1)
		 * \return true if the parameter value was changed
		 */
		bool setNorm(double a_norm_value);
		bool setFromString(const string& a_str);

		/**
		 * Automatically retrieves the parameter value according to its type.
		 */
		template <typename T>
		T get() const;

		bool getBool() const;
		bool getPrevBool() const;
		int getInt() const;
		int getPrevInt() const;
		double getDouble() const;
		double getPrevDouble() const;
		double getEnum() const;
		double getEnum(int a_index) const;
		double getPrevEnum() const;

		/**
		 * Get the parameter value as a number in the range (0,1)
		 */
		double getNorm() const;
		string getValueString() const;
		string getUnitsString() const;
	private:
		bool _setBool(bool a_value);
		bool _setInt(int a_value);
		bool _setDouble(double a_value);
	private:
		string m_name;
		double m_value, m_prevValue, m_defaultValue;
		double m_min, m_max;
		double m_logMin, m_logRange;
		bool m_isVisible;
		EParamType m_type;
		EUnitsType m_unitsType;
		EControlType m_controlType;
		int m_displayPrecision;

		struct DisplayText
		{
			double m_value;
			string m_text;
		};

		vector<DisplayText> m_displayTexts;
	};

	template <typename T>
	T UnitParameter::get() const {
		switch (m_type) {
		case Bool:
			return static_cast<T>(getBool());
		case Enum:
			return static_cast<T>(getEnum());			
		case Int:
			return static_cast<T>(getInt());
		case Double:
			return static_cast<T>(getDouble());
		case Null:
		default:
			return static_cast<T>(NULL);
		}
	}
};
#endif // __UnitParameter__
