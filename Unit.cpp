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


void Unit::addParams(const list<string> paramNames)
{
  for (string pname : paramNames)
  {
    m_params[pname] = Parameter();
  }
}

double Unit::tick()
{
  beginProcessing();
  m_lastOutput = process();
  return m_lastOutput;
}

void Unit::beginProcessing()
{
  for (auto it = m_params.begin(); it != m_params.end(); it++)
  {
    it->second.tick();
  }
}