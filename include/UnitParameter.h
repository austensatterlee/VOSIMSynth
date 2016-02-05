#ifndef __Parameter__
#define __Parameter__

#include "IControl.h"
#include "IParam.h"
#include "Containers.h"

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

	// forward declaration
	struct UnitSample;

	struct Connection
	{
		const vector<UnitSample>* srcbuffer;
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
		              double shape = 1, bool canEdit = false, bool canModulate = true);


		UnitParameter(const UnitParameter& other) :
			UnitParameter(other.m_parent, other.m_name, other.m_id, other.getType(),
			              other.getMin(), other.getMax(), other.getDefault(), other.getStep(),
			              other.getShape(), other.canEdit(), other.canModulate())
		{
			m_offsetValue = other.m_offsetValue;
			m_value[0] = m_value[1] = m_offsetValue;
		}

		bool operator==(const UnitParameter& p) const;

		virtual ~UnitParameter() {}

		/// used to modulate both channels simultaneously
		void mod(double amt, MOD_ACTION action);
		/// used to modulate both channels independently
		void mod(const double amt[2], MOD_ACTION action);

		operator double() const
		{
			double value = 0.5*(m_value[0] + m_value[1]);
			if (m_type != IParam::kTypeDouble) {
				int intvalue = static_cast<int>(value);
				value = value - intvalue >= 0.5 ? intvalue + 1 : intvalue;
			}
			return value;
		}

		explicit operator bool() const
		{
			return m_value[0] > 0 || m_value[1] > 0;
		}

		double operator[](int a_ind) const
		{
			return m_value[a_ind];
		}

		double getNormalized() const
		{
			return ToNormalizedParam((m_value[0]+m_value[1])*0.5, m_min, m_max, m_shape);
		}

		/// Prepare the parameter for the next sample by resetting to the base value
		void reset()
		{
			m_value[0] = m_value[1] = m_offsetValue;
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

		bool canEdit() const
		{
			return m_canEdit;
		}

		void setCanEdit(bool a_i)
		{
			m_canEdit = a_i;
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

		void getDisplayText(char* r_displayText) const;

		void addConnection(const vector<UnitSample>* srcbuffer, MOD_ACTION action);
		void removeConnection(const vector<UnitSample>* a_srcbuffer, MOD_ACTION a_action);

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
		double m_value[2]; //<! current value (after modulation)
		double m_offsetValue; //<! current value (before modulation)
		double m_min, m_max;
		double m_step, m_shape;
		double m_default;
		bool m_canEdit, m_canModulate;
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


