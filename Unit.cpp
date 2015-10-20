#include "Unit.h"
namespace syn
{
	void Unit::modifyParameter(int portid, double val, MOD_ACTION action)
	{
	  m_params[portid].mod(action, val);
	}

  vector<string> Unit::getParameterNames() const
  {
    vector<string> pnames; 
    for (int i = 0; i < m_params.size(); i++)
    {
      pnames.push_back(m_params[i].getName());
    }
    return pnames;
  }

  Unit* Unit::clone() const
  {
    Unit* u = cloneImpl();
    u->m_name = m_name;
    u->m_Fs = m_Fs;
    u->m_lastOutput = m_lastOutput;
    u->m_params = m_params;
    u->m_parammap = m_parammap;
    return u;
  }

  void Unit::addParam(UnitParameter& param)
  {
    m_parammap[param.getName()] = m_params.size();
    m_params.push_back(param);
  }

  Unit::Unit(string name) :
	  m_Fs(44100.0),
	  m_lastOutput(0.0),
    m_name(name)
  {
	}

  Unit::~Unit()
	{
	}

	double Unit::tick()
	{
	  beginProcessing();
    double output = process();
    m_lastOutput = finishProcessing(output);
    m_extOutPort.Emit(m_lastOutput);
    return m_lastOutput;
	}
	
	inline void Unit::beginProcessing()
	{
	  for (int i=0;i<m_params.size();i++)
	  {
	   m_params[i].tick();
	  }
	}

  int Unit::getParamId(string name)
  {
    return m_parammap.at(name);
  }

}