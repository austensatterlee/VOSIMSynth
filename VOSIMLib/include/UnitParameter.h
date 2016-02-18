#ifndef __UnitParameter__
#define __UnitParameter__

#include <vector>
#include <map>
#include <cstring>

using std::string;
using std::map;
using std::vector;

#define MAX_PARAM_STR_LEN 32

namespace syn
{
    enum EParamType{
        Null,
        Bool,
        Enum,
        Int,
        Double
    };

	class UnitParameter
	{
	public:
        UnitParameter();
        UnitParameter(const string& a_name, bool a_defaultValue);
        UnitParameter(const string& a_name, int a_min, int a_max, int a_defaultValue);
        UnitParameter(const string& a_name, const vector<string>& a_optionNames);
        UnitParameter(const string& a_name, double a_min, double a_max, double a_defaultValue, int a_displayPrecision=2);


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

        bool setBool(bool a_value);
        bool setInt(int a_value);
        bool setDouble(double a_value);
		bool set(double a_value);
        /**
         * Set the parameter from a number in the range (0,1)
         */
        bool setNorm(double a_norm_value);

        template<typename T>
        T get() const;

        bool getBool() const;
        int getInt() const;
        double getDouble() const;
        /**
        * Get the parameter value as a number in the range (0,1)
        */
        double getNorm() const;
        string getString() const;
	private:
        string m_name;
        double m_value, m_defaultValue;
        double m_min, m_max;
        EParamType m_type;
        int m_displayPrecision;

		struct DisplayText
		{
			int m_value;
			string m_text;
		};

		vector<DisplayText> m_displayTexts;
	};

    template<typename T>
    T UnitParameter::get() const{
        switch(m_type){
            case Bool:
                return (T)getBool();
            case Enum:
            case Int:
                return (T)getInt();
            case Double:
                return (T)getDouble();
            case Null:
            default:
                return (T)NULL;
        }
    }
}
#endif // __UnitParameter__


