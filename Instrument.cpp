#include "Instrument.h"
namespace syn
{
	
	void Instrument::addSource(string name, SourceUnit* unit)
	{
	  addUnit(name, unit);
	  m_sourceUnits[name] = unit;
	}
	
	
	void Instrument::noteOn(int pitch, int vel)
	{
	  for (SourceUnitMap::iterator it = m_sourceUnits.begin() ; it!=m_sourceUnits.end();it++)
	  {
	    it->second->noteOn(pitch, vel);
	  }
	}
	
	void Instrument::noteOff(int pitch, int vel)
  {
    for (SourceUnitMap::iterator it = m_sourceUnits.begin(); it != m_sourceUnits.end(); it++)
    {
      it->second->noteOff(pitch, vel);
    }
	}
}
