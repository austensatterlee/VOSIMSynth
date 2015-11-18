#pragma once
#include "Circuit.h"
#include <unordered_map>
#include <functional>
#include <numeric>

using std::unordered_map;
namespace syn
{
  class Instrument :
    public Circuit
  {
  public:
    Instrument() : m_note(-1) {};
    Instrument(const Instrument& instr) :
      m_sourcemap(instr.m_sourcemap),
      m_primarySrcVec(instr.m_primarySrcVec),
      m_note(instr.m_note)
    {}
    virtual ~Instrument() {};

    int addSource(SourceUnit* unit);
    void removePrimarySource(int srcid);
    void addPrimarySource(int srcid);
    virtual bool removeUnit(int uid) override;
    SourceUnit& getSourceUnit(string name) const { return *(SourceUnit*)m_units.at(m_unitmap.at(name)); };
    SourceUnit& getSourceUnit(int srcid) const { return *(SourceUnit*)m_units.at(srcid); };
    bool isSourceUnit(int srcid) const { return std::find(m_sourcemap.begin(),m_sourcemap.end(),srcid)!=m_sourcemap.end(); };
    void noteOn(int note, int vel);
    void noteOff(int note, int vel);
    bool isActive() const;
    int getNote() const { return m_note; }
  protected:
    typedef vector<int> SourceVec;
    SourceVec m_sourcemap;
    int m_note;
    SourceVec m_primarySrcVec;
  private:
    virtual Circuit* cloneImpl() const;
  };
}