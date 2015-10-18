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
    Instrument() : m_isActive(false), m_note(-1) {};
    virtual ~Instrument() {};

    void addSource(SourceUnit* unit);
    SourceUnit& getSourceUnit(string name) { return *(SourceUnit*)m_units[m_unitmap[name]]; };
    void noteOn(int note, int vel);
    void noteOff(int note, int vel);
    bool isActive() const;
    int getNote() const { return m_note; }
  protected:
    typedef vector<int> SourceVec;
    SourceVec m_sourcemap;
    bool m_isActive;
    int m_note;
  private:
    virtual Circuit* cloneImpl() const;
  };
}