#include "Circuit.h"
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <unordered_set>


using std::vector;
using std::deque;
using std::unordered_set;
namespace syn
{
  Circuit::Circuit()
  {}


  Circuit::~Circuit()
  {
    // Delete units
    for (auto it = m_units.begin(); it != m_units.end(); it++)
    {
      delete (*it).second;
    }
    m_units.clear();
    // Delete connections
    for (auto it = m_forwardConnections.begin(); it != m_forwardConnections.end(); it++)
    {
      list<Connection*>& l = (*it).second;
      while (!l.empty())
      {
        delete l.back();
        l.pop_back();
      }
    }
    m_forwardConnections.clear();
    m_backwardConnections.clear();
  }

  bool Circuit::addUnit(string name, Unit* unit)
  {
    if (m_units.find(name) == m_units.end())
    {
      m_units[name] = unit;
      unit->parent = this;
      return true;
    }
    return false;
  }

  void Circuit::setSink(string name)
  {
    m_sinkName = name;
  }

  void Circuit::modifyParameter(string uname, string pname, MOD_ACTION action, double val)
  {
    m_units[uname]->modifyParameter(pname, action, val);
  }

  vector<tuple<string,string>> Circuit::getParameterNames() const
{
    vector<tuple<string,string>> all_pnames;
    for (UnitMap::const_iterator it = m_units.begin(); it != m_units.end(); it++)
    {
      vector<string> pnames = it->second->getParameterNames();
      for (string pname : pnames)
      {
        if(!getUnit(it->first)->m_params[pname]->isRequired()){
          tuple<string,string> nametup{it->first,pname};
          all_pnames.push_back(nametup);
          }
      }
    }
    return all_pnames;
  }

  void Circuit::addConnection(Connection* connection)
  {
    string targetname = connection->m_targetname;
    string sourcename = connection->m_sourcename;
    if (!hasUnit(targetname) || !hasUnit(sourcename))
      throw std::invalid_argument("Either source or target name not found inside circuit!");
    if (!getUnit(targetname)->hasParameter(connection->m_paramname))
      throw std::invalid_argument("Target unit has no parameter with that name");
    list<Connection*>& fl = m_forwardConnections[sourcename];
    list<Connection*>& bl = m_backwardConnections[sourcename];
    if (std::find(fl.begin(), fl.end(), connection) == fl.end()) // connection not in forward list (and thus not in backward list)
    {
      m_forwardConnections[connection->m_sourcename].push_front(connection);
      m_backwardConnections[connection->m_targetname].push_front(connection);
      m_isGraphDirty = true;
    }
  }

  double Circuit::tick()
  {
    if (m_isGraphDirty)
    {
      unordered_set<string> closed_set;
      vector<string> dependencyQueue({ m_sinkName });
      deque<string>& processQueue = m_processQueue;
      processQueue.clear();
      while (!dependencyQueue.empty())
      {
        string currUnitName = dependencyQueue.back();
        dependencyQueue.pop_back();
        processQueue.push_front(currUnitName);
        if (m_backwardConnections.find(currUnitName) == m_backwardConnections.end())
          continue;
        for (list<Connection*>::iterator it = m_backwardConnections[currUnitName].begin(); it != m_backwardConnections[currUnitName].end(); it++)
        {
          string sourceName = (*it)->m_sourcename;
          if (closed_set.find(sourceName) == closed_set.end())
          {
            dependencyQueue.push_back(sourceName);
            closed_set.insert(sourceName);
          }
        }
      }
      m_isGraphDirty = false;
    }

    for (string currUnitName : m_processQueue)
    {
      Unit* source = m_units[currUnitName];
      source->tick();
      for (Connection* c : m_forwardConnections[currUnitName])
      {
        Unit* target = m_units[c->m_targetname];
        target->modifyParameter(c->m_paramname, c->m_action, source->getLastOutput());
      }
    }
    Unit* sink = getUnit(m_sinkName);
    return sink->getLastOutput();
  }

  void Circuit::setFs(double fs)
  {
    for (auto it : m_units)
    {
      it.second->setFs(fs);
    }
  }

  Circuit* Circuit::clone()
  {
    Circuit* circ = new Circuit();
    // Clone units 
    for (auto it = m_units.begin(); it != m_units.end(); it++)
    {
      circ->addUnit(it->first, it->second->clone());
    }
    circ->setSink(m_sinkName);
    // Clone connections
    for (auto it = m_forwardConnections.begin(); it != m_forwardConnections.end(); it++)
    {
      list<Connection*>& l = ((*it).second);
      for (Connection* c : l)
      {
        circ->addConnection(c->clone());
      }
    }
    for (auto it = m_midiConnections.begin(); it != m_midiConnections.end(); it++)
    {
      list<MIDIConnection*>& l = ((*it).second);
      for (MIDIConnection* mc : l)
      {
        circ->addMIDIConnection(mc->clone());
      }
    }
    circ->m_isGraphDirty = m_isGraphDirty;
    circ->m_processQueue = m_processQueue;
    return circ;
  }

  bool Circuit::hasUnit(string name)
  {
    return m_units.find(name) != m_units.end();
  }

  void Circuit::addMIDIConnection(MIDIConnection* midiConnection)
  {
    m_midiConnections[midiConnection->m_ccMessage].push_back(midiConnection);
  }

  void Circuit::sendMIDICC(IMidiMsg *midimsg)
  {
    IMidiMsg::EControlChangeMsg cc = midimsg->ControlChangeIdx();
    double val = midimsg->ControlChange(cc);
    for (auto it : m_midiConnections[cc])
    {
      modifyParameter((*it).m_targetname, (*it).m_paramname, (*it).m_action, val);
    }
  }
}