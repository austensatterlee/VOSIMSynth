#include "Unit.h"
#include <array>

using namespace std;

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
    u->m_bufind = m_bufind;
    for (int i = 0; i < m_params.size(); i++)
    {
      u->m_params[i]->mod(*m_params[i], SET);
    }
    return u;
  }

  UnitParameter& Unit::addParam(string name, int id, IParam::EParamType ptype, const double min, const double max, const double defaultValue, const bool isHidden)
  {
    m_parammap[name] = id;
    if (m_params.size() <= id)
      m_params.resize(id + 1);
    m_params[id] = new UnitParameter(this, name, id, ptype, min, max, defaultValue, isHidden);
    return *m_params[id];
  }

  UnitParameter& Unit::addParam(string name, IParam::EParamType ptype, const double min, const double max, const double defaultValue, const bool isHidden)
  {
    return addParam(name, m_params.size(), ptype, min, max, defaultValue, isHidden);
  }

  UnitParameter& Unit::addEnumParam(string name, const vector<string> choice_names)
  {
    UnitParameter& param = addParam(name, IParam::kTypeEnum, 0, choice_names.size(), false);
    for (int i = 0; i < choice_names.size(); i++)
    {
      param.SetDisplayText(i, choice_names[i].c_str());
    }
    return param;
  }

  Unit::Unit(string name) :
    m_Fs(44100.0),
    m_output(1, 0.0),
    m_bufind(0),
    m_name(name),
    m_parent(nullptr)
  {}

  Unit::~Unit()
  {
    // delete allocated parameters
    for (int i = 0; i < m_params.size(); i++)
    {
      delete m_params[i];
    }
  }

  void Unit::tick()
  {
    beginProcessing();
    for (int i = 0; i < m_output.size(); i++)
    {
      for (int j = 0; j < m_params.size(); j++)
      {
        m_params[j]->reset();
        if (m_params[j]->numConnections()>0) {
          m_params[j]->pull(i);
        }
      }
      process(i);
      m_bufind = i;
    }
    finishProcessing();
  }

  int Unit::getParamId(string name)
  {
    int paramid;
    try
    {
      paramid = m_parammap.at(name);
    }
    catch (out_of_range exc)
    {
      paramid = -1;
    }
    return paramid;
  }
}