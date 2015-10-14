#include "Instrument.h"
namespace syn
{

  void Instrument::addSource(string name, SourceUnit* unit)
  {
    addUnit(name, unit);
    m_soUnits[name] = unit;
  }


  void Instrument::noteOn(int pitch, int vel)
  {
    m_note = pitch;
    m_isActive = true;
    for (SourceUnitMap::iterator it = m_soUnits.begin(); it != m_soUnits.end(); it++)
    {
      it->second->noteOn(pitch, vel);
    }
  }

  void Instrument::noteOff(int pitch, int vel)
  {
    m_isActive = false;
    for (SourceUnitMap::iterator it = m_soUnits.begin(); it != m_soUnits.end(); it++)
    {
      it->second->noteOff(pitch, vel);
    }
  }

  bool Instrument::isActive() const
  {
    bool result = false;
    for (SourceUnitMap::const_iterator it = m_soUnits.begin(); it != m_soUnits.end() ; it++)
    {
      result |= it->second->isActive();
    }
    return result;
  }

  Instrument* Instrument::clone()
  {
    Instrument* instr = new Instrument();
    // Clone units
    for (auto it = m_soUnits.begin(); it != m_soUnits.end(); it++)
    {
      instr->addSource(it->first, (SourceUnit*)it->second->clone()); // we already cloned this unit above
    }
    instr->setSink(m_sinkName);
    for (auto it = m_units.begin(); it != m_units.end(); it++)
    {
      if(!instr->hasUnit(it->first))
        instr->addUnit(it->first, it->second->clone());
    }
    // Clone connections
    for (auto it = m_forwardConnections.begin(); it != m_forwardConnections.end(); it++)
    {
      list<Connection*>& l = ((*it).second);
      for (Connection* c : l)
      {
        instr->addConnection(c->clone());
      }
    }
    for (auto it = m_midiConnections.begin(); it != m_midiConnections.end(); it++)
    {
      list<MIDIConnection*>& l = ((*it).second);
      for (MIDIConnection* mc : l)
      {
        instr->addMIDIConnection(mc->clone());
      }
    }
    instr->m_isGraphDirty = m_isGraphDirty;
    instr->m_processQueue = m_processQueue;
    return instr;
  }

}
