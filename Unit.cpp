#include "Unit.h"
void Unit::modifyParameter(const string pname, const MOD_ACTION action, double val)
{
  m_params[pname].mod(action, val);
}

Unit::Unit() :
m_Fs(44100.0),
m_lastOutput(0.0)
{
  
}

Unit::Unit(const Unit& u) :
Unit()
{
  m_params.insert(u.m_params.begin(),u.m_params.end());
  m_Fs = u.m_Fs;
  m_lastOutput = u.m_lastOutput;
}

Unit::~Unit()
{

}


void Unit::addParams(const list<tuple<string, Parameter>> paramtuples)
{
  for (tuple<string,Parameter> ptuple : paramtuples)
  {
    string name = std::get<0>(ptuple);
    Parameter& param = std::get<1>(ptuple);
    m_params[name] = param;
  }
}

void Unit::addParams(const list<string> paramnames)
{
  for (string name : paramnames)
  {
    m_params[name] = Parameter();
  }
}

double Unit::tick()
{
  beginProcessing();
  m_lastOutput = process();
  return finishProcessing(m_lastOutput);
}

inline void Unit::beginProcessing()
{
  for (auto& p : m_params)
  {
    p.second.tick();
  }
}

inline double Unit::finishProcessing(double o)
{
  return o;
}