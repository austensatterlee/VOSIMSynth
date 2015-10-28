#include "Instrument.h"
#include <cassert>
namespace syn
{

  void Instrument::addSource(SourceUnit* unit)
  {
    m_sourcemap.push_back(m_units.size());
    if(m_primarySrcId==-1)
      m_primarySrcId = m_sourcemap.back();
    addUnit(unit);
  }

  void Instrument::setPrimarySource(string name)
  {
    int srcid = getUnitId(name);
    assert(std::find(m_sourcemap.begin(),m_sourcemap.end(),srcid)!=m_sourcemap.end());
    m_primarySrcId = srcid;
  }


  void Instrument::noteOn(int pitch, int vel)
  {
    m_note = pitch;
    for (int i = 0; i < m_sourcemap.size(); i++)
    {
      ((SourceUnit*)m_units[m_sourcemap[i]])->noteOn(pitch, vel);
    }
  }

  void Instrument::noteOff(int pitch, int vel)
  {
    for (int i = 0; i < m_sourcemap.size(); i++)
    {
      ((SourceUnit*)m_units[m_sourcemap[i]])->noteOff(pitch, vel);
    }
  }

  bool Instrument::isActive() const
  {
    return ((SourceUnit*)m_units[m_primarySrcId])->isActive();
  }

  Circuit* Instrument::cloneImpl() const
  {
    Instrument* instr = new Instrument(*this);
    return instr;
  }

}
