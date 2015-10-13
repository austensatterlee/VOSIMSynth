#include "Instrument.h"

void Instrument::addSource(string name, SourceUnit* unit)
{
  addUnit(name, unit);
  m_sourceUnits.push_back(unit);
}


void Instrument::noteOn(int pitch, int vel)
{
  for (auto it : m_sourceUnits)
  {
    (*it).noteOn(pitch, vel);
  }
}

void Instrument::noteOff(int pitch, int vel)
{
  for (auto it : m_sourceUnits)
  {
    (*it).noteOff(pitch, vel);
  }
}
