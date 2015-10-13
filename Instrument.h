#pragma once
#include "Circuit.h"
#include "Connection.h"
#include <vector>


using std::vector;

class Instrument :
  public Circuit
{
public:
  Instrument(){};
  virtual ~Instrument(){};

  void addSource(string name, SourceUnit* unit);
  void noteOn(int pitch, int vel);
  void noteOff(int pitch, int vel);
protected:
  vector<SourceUnit*> m_sourceUnits;
};