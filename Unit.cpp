#include "Unit.h"
namespace syn
{
	void Unit::modifyParameter(const string pname, const MOD_ACTION action, double val)
	{
	  m_params[pname]->mod(action, val);
	}
	
	Unit::Unit() :
	  m_Fs(44100.0),
	  m_lastOutput(0.0)
	{
	}
	
	Unit::Unit(const Unit& u) :
	  Unit()
	{
	  for (UnitParameterMap::const_iterator it = u.m_params.cbegin(); it != u.m_params.cend(); it++)
	  {
	    m_params[it->first] = new UnitParameter(*(it->second));
	  }
	  m_Fs = u.m_Fs;
	  m_lastOutput = u.m_lastOutput;
	}
	
	Unit::~Unit()
	{
	  for (UnitParameterMap::iterator it = m_params.begin(); it != m_params.end(); it++)
	  {
	    delete it->second;
	  }
	}
	
	
	void Unit::addParam(UnitParameter* param)
	{
	  m_params[param->getName()] = param;
	}
	
	void Unit::addParams(const vector<string> paramnames)
	{
	  for (string name : paramnames)
	  {
	    m_params[name] = new UnitParameter(name);
	  }
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
	  for (UnitParameterMap::iterator it = m_params.begin(); it != m_params.end(); it++)
	  {
	   it->second->tick();
	  }
	}
}