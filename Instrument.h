#pragma once
#include "Circuit.h"
#include "Connection.h"
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
    Instrument() : m_note(-1), m_primarySrcId(-1) {};
    Instrument(const Instrument& instr);
    virtual ~Instrument() {};

    void addSource(SourceUnit* unit);
    void setPrimarySource(string name);
    SourceUnit& getSourceUnit(string name) { return *(SourceUnit*)m_units[m_unitmap[name]]; };
    void noteOn(int note, int vel);
    void noteOff(int note, int vel);
    bool isActive() const;
    int getNote() const { return m_note; }
  protected:
    typedef vector<int> SourceVec;
    SourceVec m_sourcemap;
    int m_note;
    int m_primarySrcId;
  private:
    virtual Circuit* cloneImpl() const;
  };
}