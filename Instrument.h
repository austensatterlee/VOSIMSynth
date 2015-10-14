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

    void addSource(string name, SourceUnit* unit);
    SourceUnit* getSourceUnit(string name) { return m_soUnits[name]; };
    void noteOn(int note, int vel);
    void noteOff(int note, int vel);
    bool isActive() const;
    int getNote() const { return m_note; }
    virtual Instrument* clone();
  protected:
    typedef unordered_map<string, SourceUnit*> SourceUnitMap;
    SourceUnitMap m_soUnits;
    bool m_isActive;
    int m_note;
  };
}