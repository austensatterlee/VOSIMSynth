#include "Instrument.h"
#include <cassert>
namespace syn
{

  int Instrument::addSource(SourceUnit* unit)
  {
    int uid = addUnit(unit);
    if(uid!=-1){
      m_sourcemap.push_back(uid);
      if (m_primarySrcVec.empty())
        m_primarySrcVec.push_back(m_sourcemap.back());
    }
    return uid;
  }

  void Instrument::addPrimarySource(int srcid)
  {
    assert(std::find(m_sourcemap.begin(), m_sourcemap.end(), srcid) != m_sourcemap.end());
    m_primarySrcVec.push_back(srcid);
  }

  bool Instrument::removeUnit(int uid)
  {
    bool success = Circuit::removeUnit(uid);
    SourceVec::iterator srcmap_it = std::find(m_sourcemap.begin(), m_sourcemap.end(), uid);
    if (srcmap_it != m_sourcemap.end())
    {
      m_sourcemap.erase(srcmap_it);
      SourceVec::iterator primarysrc_it = std::find(m_primarySrcVec.begin(), m_primarySrcVec.end(), uid);
      if (primarysrc_it != m_primarySrcVec.end())
      {
        m_primarySrcVec.erase(primarysrc_it);
      }
    }
    return success;
  }

  void Instrument::removePrimarySource(int srcid)
  {
    SourceVec::iterator it = std::find(m_primarySrcVec.begin(), m_primarySrcVec.end(), srcid);
    if (it == m_primarySrcVec.end())
    {
      DBGMSG("Error: cannot remove source unit as primary source");
      return;
    }
    m_primarySrcVec.erase(it);
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
    if(m_sinkId<0) return false;
    for(int i=0;i<m_primarySrcVec.size();i++){
      if(((SourceUnit*)m_units.at(m_primarySrcVec[i]))->isActive()) return true;
    }
    return false;
  }

  Circuit* Instrument::cloneImpl() const
  {
    Instrument* instr = new Instrument(*this);
    return instr;
  }

}
