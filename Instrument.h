#pragma once
#include "Circuit.h"
#include <unordered_map>

using std::unordered_map;
namespace syn
{
  class SourceUnit;

  class Instrument :
    public Circuit
  {
  public:
    Instrument() : m_note(-1) {};
    Instrument(const Instrument& instr) :
      m_sourcemap(instr.m_sourcemap),
      m_note(instr.m_note),
      m_primarySrcVec(instr.m_primarySrcVec)
    {}
    virtual ~Instrument() {};

    int addSource(SourceUnit* unit);
    int addSource(SourceUnit * unit, int uid);
    void removePrimarySource(int srcid);
    /**
     * Marks the specified source unit as a primary source
     */
    void setPrimarySource(int srcid);
    /**
     * Same as setPrimarySource but first clears all current marks
     */
    void resetPrimarySource(int srcid);
    bool isPrimarySource(int srcid) const;
    bool removeUnit(int uid) override;
    SourceUnit& getSourceUnit(string name) const { return *reinterpret_cast<SourceUnit*>(m_units.at(m_unitmap.at(name))); };
    SourceUnit& getSourceUnit(int srcid) const { return *reinterpret_cast<SourceUnit*>(m_units.at(srcid)); };
    bool isSourceUnit(int srcid) const { return find(m_sourcemap.begin(), m_sourcemap.end(), srcid) != m_sourcemap.end(); };
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
    virtual Circuit* cloneImpl() const override;
  };
}