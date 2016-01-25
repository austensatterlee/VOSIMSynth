#ifndef __Parameter__
#define __Parameter__

#include "IControl.h"
#include "IParam.h"
#include "Containers.h"

#include <string>
#include <vector>
#include <map>

using std::string;
using std::map;
using std::vector;

#define MAX_PARAM_STR_LEN 32

namespace syn
{
	enum MOD_ACTION
	{
		SET = 0,
		SET_NORM,
		ADD,
		SCALE,
		NUM_MOD_ACTIONS
	};

	struct Connection
	{
		const vector<double>* srcbuffer;
		MOD_ACTION action;

		bool operator==(const Connection& other) const
		{
			return srcbuffer == other.srcbuffer && action == other.action;
		}
	};

	class Unit;

	class UnitParameter
	{
	public:
		UnitParameter(Unit* parent, string name, int id, IParam::EParamType ptype,
		              double min, double max, double defaultValue, double step,
		              double shape = 1, bool isHidden = false, bool canModulate = true);


		UnitParameter(const UnitParameter& other) :
			UnitParameter(other.m_parent, other.m_name, other.m_id, other.getType(),
			              other.getMin(), other.getMax(), other.getDefault(), other.getStep(),
			              other.getShape(), other.isHidden(), other.canModulate())
		{
			mod(other.m_offsetValue, SET);
		}

		bool operator==(const UnitParameter& p) const;

		virtual ~UnitParameter() {}

		void mod(double amt, MOD_ACTION action);

		operator double() const
		{
			double value = m_value;
			if (m_type != IParam::kTypeDouble) {
				value = m_value<0 ? ceil(m_value) : floor(m_value);
			}
			return value;
		}

		double getNormalized() const
		{
			return ToNormalizedParam(m_value, m_min, m_max, m_shape);
		}

		/// Prepare the parameter for the next sample by resetting to the base value
		void reset()
		{
			mod(m_offsetValue, SET);
		}

		string getName() const
		{
			return m_name;
		}

		void setName(string new_name)
		{
			m_name = new_name;
		}

		int getId() const
		{
			return m_id;
		}

		bool isHidden() const
		{
			return m_isHidden;
		}

		void setHidden(bool a_i)
		{
			m_isHidden = a_i;
		}

		bool canModulate() const
		{
			return m_canModulate;
		}

		void setCanModulate(bool a_i)
		{
			m_canModulate = a_i;
		}

		IParam::EParamType getType() const
		{
			return m_type;
		}

		void setType(IParam::EParamType new_type);

		double getMin() const
		{
			return m_min;
		}

		void setMin(double a_i)
		{
			m_min = a_i;
		}

		double getMax() const
		{
			return m_max;
		}

		void setMax(double a_i)
		{
			m_max = a_i;
		}

		double getDefault() const
		{
			return m_default;
		}

		double getStep() const
		{
			return m_step;
		}

		void setStep(double a_i)
		{
			m_step = a_i;
		}

		double getShape() const
		{
			return m_shape;
		}

		void setShape(double a_i)
		{
			m_shape = a_i;
		}

		void setDisplayText(int value, const char* text);

		void getDisplayText(char* r_displayText, bool normalized) const;

		bool isDirty() const
		{
			return m_isDirty;
		}

		void setClean()
		{
			m_isDirty = false;
		}

		void addConnection(const vector<double>* srcbuffer, MOD_ACTION action);

		int getNumConnections() const
		{
			return m_connections.size();
		}

		void pull(int bufind);

		UnitParameter* clone() const
		{
			UnitParameter* other = new UnitParameter(*this);
			return other;
		}

	protected:
		Unit* m_parent;
		string m_name;
		int m_id;
		double m_value; //<! current value (after modulation)
		double m_offsetValue; //<! current value (before modulation)
		double m_min, m_max;
		double m_step, m_shape;
		double m_default;
		bool m_isHidden, m_isDirty, m_canModulate;
		int m_displayPrecision;
		IParam::EParamType m_type;
		vector<Connection> m_connections;
	private:
		struct DisplayText
		{
			int m_value;
			char m_text[MAX_PARAM_STR_LEN];
		};

		WDL_TypedBuf<DisplayText> m_displayTexts;
	};
}
#endif // __Parameter__


