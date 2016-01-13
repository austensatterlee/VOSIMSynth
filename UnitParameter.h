#ifndef __Parameter__
#define __Parameter__

#include "IControl.h"
#include "IParam.h"
#include <string>
#include <vector>
#include <map>

using std::string;
using std::map;
using std::vector;

namespace syn
{
	enum MOD_ACTION
	{
		SET = 0,
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

	class UnitParameter : public IParam
	{
	protected:
		Unit* m_parent;
		string m_name;
		int m_id;
		double m_offsetValue;
		bool m_isHidden;
		bool m_isDirty;
		const IControl* m_controller;
		vector<Connection> m_connections;
	public:
		UnitParameter(Unit* parent, string name, int id, IParam::EParamType ptype, double min, double max, double defaultValue, bool ishidden = false);

		UnitParameter(const UnitParameter& other) :
			UnitParameter(other.m_parent, other.m_name, other.m_id, other.Type(), other.GetMin(), other.GetMax(), other.GetDefault(), other.isHidden())
		{
			mod(other.m_offsetValue, SET);
		}

		bool operator==(const UnitParameter& p) const;
		void mod(double amt, MOD_ACTION action);

		operator double() const
		{
			return Value();
		}

		/**
		 * \brief Prepare the parameter for the next sample by resetting to the base value
		 */
		void reset()
		{
			Set(m_offsetValue);
		}

		string getName() const
		{
			return m_name;
		};

		int getId() const
		{
			return m_id;
		};

		double getBase() const
		{
			return m_offsetValue;
		}

		const IControl* getController() const
		{
			return m_controller;
		}

		bool hasController() const
		{
			return m_controller != nullptr;
		}

		bool isHidden() const
		{
			return m_isHidden;
		}

		void setHidden(bool ishidden)
		{
			m_isHidden = ishidden;
		}

		bool isDirty() const
		{
			return m_isDirty;
		}

		void setClean()
		{
			m_isDirty = false;
		}

		void addConnection(const vector<double>* srcbuffer, MOD_ACTION action)
		{
			m_connections.push_back({ srcbuffer,action });
		}

		int numConnections() const
		{
			return m_connections.size();
		}

		void pull(int bufind)
		{
			for (int i = 0; i < m_connections.size(); i++)
			{
				mod((*m_connections[i].srcbuffer)[bufind], m_connections[i].action);
			}
		}

		void setController(const IControl* controller);
		void unsetController(const IControl* controller);

		UnitParameter* clone() const
		{
			UnitParameter* other = cloneImpl();
			return other;
		}

	private:
		virtual UnitParameter* cloneImpl() const
		{
			return new UnitParameter(*this);
		}
	};
}
#endif // __Parameter__


