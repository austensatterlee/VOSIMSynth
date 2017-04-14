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

#include "vosimlib/common_serial.h"
#include "vosimlib/common.h"
#include <vector>
#include <map>

using std::string;
using std::map;
using std::vector;

#define MAX_PARAM_STR_LEN 32

namespace syn
{
    class Unit;

    class VOSIMLIB_API UnitParameter
    {
    public:
        struct DisplayText
        {
            double m_value;
            string m_text;
            
            DisplayText(double value, const string& text) : m_value(value), m_text(text) { }
            DisplayText(const json& j) : m_value(j["value"]), m_text(j["text"].get<string>()) { }

            operator json() const
            {
                json j;
                j["text"] = m_text;
                j["value"] = m_value;
                return j;
            }
        };

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
            Decibal,
            Samples
        };

        enum EControlType
        {
            Bounded,
            Unbounded
        };

    public:
        UnitParameter();
        /**
         * Creates a boolean parameter
         */
        UnitParameter(const string& a_name, bool a_defaultValue);
        /**
         * Creates an integer parameter
         */
        UnitParameter(const string& a_name, int a_min, int a_max, int a_defaultValue, EUnitsType a_unitsType = None);
        /**
         * Creates an enumerated parameter
         */
        UnitParameter(const string& a_name, const vector<string>& a_optionNames, const vector<double>& a_optionValues = {}, int a_defaultOption = 0, EUnitsType a_unitsType = None);
        /**
         * Creates a real-number parameter
         */
        UnitParameter(const string& a_name, double a_min, double a_max, double a_defaultValue, EUnitsType m_unitsType = None, int a_displayPrecision = 2);

        /**
         * Reset the parameter to its default value
         */
        void reset();

        const string& getName() const;
        int getId() const;
        void setId(int a_id);
        Unit* getParent() const;
        void setParent(Unit* a_newParent);

        EParamType getType() const;

        double getMin() const;

        void setMin(double a_new_min);

        double getMax() const;

        void setMax(double a_new_max);

        double getDefaultValue() const;

        int getPrecision() const;
        bool setPrecision(int a_precision);

        EControlType getControlType() const;
        UnitParameter& setControlType(EControlType a_newControlType);

        UnitParameter& setUnitsType(EUnitsType a_newUnitsType);

        UnitParameter& setVisible(bool a_visible);

        bool isVisible() const;

        bool set(double a_value);
        /**
         * Set the parameter from a number in the range (0,1)
         * \return true if the parameter value was changed
         */
        bool setNorm(double a_norm_value);
        bool setFromString(const string& a_str);
        bool nudge(double a_logAmt, double a_linAmt);

        /**
         * Automatically retrieves the parameter value according to its type.
         */
        template <typename T>
        T get() const;

        bool getBool() const;
        int getInt() const;
        double getDouble() const;
        double getEnum() const;
        double getEnum(int a_index) const;

        /**
         * Get the parameter value as a number in the range (0,1)
         */
        double getNorm() const;
        string getValueString() const;
        string getUnitsString() const;
        const vector<DisplayText>& getDisplayTexts() const;

        UnitParameter& load(const json& j);

        operator json() const;

    private:
        string m_name;
        int m_id;
        double m_value, m_defaultValue;
        double m_min, m_max;
        double m_logMin, m_logRange;
        bool m_isVisible;
        EParamType m_type;
        EUnitsType m_unitsType;
        EControlType m_controlType;
        int m_displayPrecision;
        Unit* m_parent;

        vector<DisplayText> m_displayTexts;
    };

    template <typename T>
    T UnitParameter::get() const
    {
        switch (m_type)
        {
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
