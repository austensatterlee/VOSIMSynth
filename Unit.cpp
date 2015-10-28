#include "Unit.h"
namespace syn
{
  void Unit::modifyParameter(int portid, double val, MOD_ACTION action)
  {
    m_params[portid]->mod(val, action);
  }

  vector<string> Unit::getParameterNames() const
  {
    vector<string> pnames;
    for (int i = 0; i < m_params.size(); i++)
    {
      pnames.push_back(m_params[i]->getName());
    }
    return pnames;
  }

  Unit* Unit::clone() const
  {
    Unit* u = cloneImpl();
    u->m_name = m_name;
    u->m_Fs = m_Fs;
    u->m_output = m_output;
    u->m_parammap = m_parammap;      
    return u;
  }

  UnitParameter& Unit::addParam(string name, int id, PARAM_TYPE ptype,double min, double max)
  {

    m_parammap[name] = id;
    if (m_params.size() <= id)
      m_params.resize(id + 1);
    m_params[id] = new UnitParameter(name, id, ptype, min, max);
    return *m_params[id];
  }

  UnitParameter& Unit::addParam(string name, PARAM_TYPE ptype, double min, double max)
  {
    return addParam(name, m_params.size(), ptype, min, max);
  }

  Unit::Unit(string name) :
    m_Fs(44100.0),
    m_output(0.0),
    m_name(name)
  {}

  Unit::~Unit()
  {
    // delete allocated parameters
    for (int i = 0; i < m_params.size(); i++)
    {
      delete m_params[i];
    }
  }

  void Unit::tick(size_t nsamples, vector<Connection*>& connections)
  {
    m_output.resize(nsamples);
    for (int i = 0; i < nsamples; i++)
    {
      tickParams(connections);
      m_output[i] = process();
      m_output[i] = finishProcessing(m_output[i]);
      m_extOutPort.Emit(m_output[i]);
      for (int i = 0; i < m_params.size(); i++)
      {
        m_params[i]->reset();
      }
    }
  }

  void Unit::tickParams(vector<Connection*>& connections)
  {
    for (int i = 0; i < connections.size(); i++)
    {
      if(!connections[i]->isEmpty()){
        double amt = connections[i]->pull();
        m_params[connections[i]->m_targetport]->mod(amt, connections[i]->m_action);
      }
    }
  }

  int Unit::getParamId(string name)
  {
    return m_parammap.at(name);
  }

}