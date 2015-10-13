#include "Circuit.h"
#include <vector>
#include <deque>

using std::vector;
using std::deque;
namespace syn
{
  Circuit::Circuit()
  {}

  Circuit::Circuit(const Circuit & c)
  {
    throw("Not Implemented");
  }


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
      list<Connection*>* l = &((*it).second);
      while (!l->empty())
      {
        delete l->back();
        l->pop_back();
      }
      delete l;
    }
    m_forwardConnections.clear();
    m_backwardConnections.clear();
  }

  void Circuit::addUnit(string name, Unit* unit)
  {
    m_units[name] = unit;
    unit->parent = this;
  }

  void Circuit::setSink(string name, SinkUnit* unit)
  {
    addUnit(name, unit);
    m_sinkUnit = unit;
    m_sinkName = name;
  }

  void Circuit::modifyParameter(string uname, string pname, MOD_ACTION action, double val)
  {
    m_units[uname]->modifyParameter(pname, action, val);
  }

  void Circuit::addConnection(Connection* connection)
  {
    m_forwardConnections[connection->m_sourcename].push_front(connection);
    m_backwardConnections[connection->m_targetname].push_front(connection);
  }

  double Circuit::tick()
  {
    vector<string> dependencyQueue({ m_sinkName });
    deque<string> processQueue;
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
        dependencyQueue.push_back(sourceName);
      }
    }

    while (!processQueue.empty())
    {
      string currUnitName = processQueue.front();
      processQueue.pop_front();
      m_units[currUnitName]->tick();
      for (auto const &it : m_forwardConnections[currUnitName])
      {
        Unit* target = m_units[it->m_targetname];
        target->modifyParameter(it->m_paramname, it->m_action, m_units[currUnitName]->getLastOutput());
      }
    }

    return m_sinkUnit->getLastOutput();
  }

  void Circuit::setFs(double fs)
  {
    for (auto it : m_units)
    {
      it.second->setFs(fs);
    }
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