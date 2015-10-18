#include "Instrument.h"
namespace syn
{

  void Instrument::addSource(SourceUnit* unit)
  {
    m_sourcemap.push_back(m_units.size());
    addUnit(unit);
  }


  void Instrument::noteOn(int pitch, int vel)
  {
    m_note = pitch;
    m_isActive = true;
    for (int i = 0; i < m_sourcemap.size(); i++)
    {
      ((SourceUnit*)m_units[m_sourcemap[i]])->noteOn(pitch, vel);
    }
  }

  void Instrument::noteOff(int pitch, int vel)
  {
    m_isActive = false;
    for (int i = 0; i < m_sourcemap.size(); i++)
    {
      ((SourceUnit*)m_units[m_sourcemap[i]])->noteOff(pitch, vel);
    }
  }

  bool Instrument::isActive() const
  {
    bool result = false;
    for (int i=0;i<m_sourcemap.size();i++)
    {
      result |= ((SourceUnit*)m_units[m_sourcemap[i]])->isActive();
    }
    return result;
  }

  Circuit* Instrument::cloneImpl() const
  {
    Instrument* instr = new Instrument();
    instr->m_sourcemap = m_sourcemap;
    instr->m_isActive = m_isActive;
    instr->m_note = m_note;
    return instr;
  }

}
