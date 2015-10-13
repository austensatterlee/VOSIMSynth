#pragma once
#include "Circuit.h"
#include "Connection.h"
#include <unordered_map>


using std::unordered_map;
namespace syn
{
  class Instrument :
    public Circuit
  {
  public:
    Instrument() {};
    virtual ~Instrument() {};

    void addSource(string name, SourceUnit* unit);
    SourceUnit* getSourceUnit(string name){return m_sourceUnits[name];};
    void noteOn(int pitch, int vel);
    void noteOff(int pitch, int vel);
  protected:
    typedef unordered_map<string, SourceUnit*> SourceUnitMap;
    SourceUnitMap m_sourceUnits;
  };
}