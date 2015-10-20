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
      delete (*it);
    }
  }

  bool Circuit::addUnit(Unit* unit)
  {

    if (m_unitmap.find(unit->getName()) == m_unitmap.end())
    {
      m_forwardConnections.push_back(vector<Connection>());
      m_backwardConnections.push_back(vector<Connection>());
      m_unitmap[unit->getName()] = m_units.size();
      m_units.push_back(unit);
      unit->parent = this;
      m_isGraphDirty = true;
      return true;
    }
    return false;
  }

  void Circuit::setSinkId(int id)
  {
    m_sinkId = id;
    m_isGraphDirty = true;
  }

  void Circuit::setSinkName(string name)
  {
    m_sinkId = getUnitId(name);
    m_isGraphDirty = true;
  }

  void Circuit::modifyParameter(string uname, string pname, double val, MOD_ACTION action)
  {
    m_units[m_unitmap[uname]]->modifyParameter(m_units[m_unitmap[uname]]->m_parammap[pname], val, action);
  }

  void Circuit::modifyParameter(int uid, int portid, double val, MOD_ACTION action)
  {
    m_units[uid]->modifyParameter(portid, val, action);
  }

  vector<tuple<string, string>> Circuit::getParameterNames() const
  {
    vector<tuple<string, string>> all_pnames;
    int id = 0;
    for (UnitVec::const_iterator it = m_units.begin(); it != m_units.end(); it++)
    {
      vector<string> pnames = (*it)->getParameterNames();
      for (string pname : pnames)
      {
        if (!(*it)->getParam(pname).isHidden())
        {
          tuple<string, string> nametup{ (*it)->getName(),pname };
          all_pnames.push_back(nametup);
        }
      }
      id++;
    }
    return all_pnames;
  }

  void Circuit::addConnection(Connection& c)
  {
    if (!hasUnit(c.m_sourceid) || !hasUnit(c.m_targetid))
      throw std::invalid_argument("Either source or target name not found inside circuit!");
    vector<Connection>& fl = m_forwardConnections[c.m_sourceid];
    if (std::find(fl.begin(), fl.end(), c) == fl.end()) // connection not in forward list (and thus not in backward list)
    {
      vector<Connection>& bl = m_backwardConnections[c.m_targetid];
      fl.push_back(c);
      bl.push_back(c);
      m_isGraphDirty = true;
    }
  }

  void Circuit::addConnection(string srcname, string targetname, string pname, MOD_ACTION action)
  {
    int sourceid = getUnitId(srcname);
    int targetid = getUnitId(targetname);
    int paramid = getUnit(targetid).getParamId(pname);
    Connection c{sourceid,targetid,paramid,action};
    if (!hasUnit(c.m_sourceid) || !hasUnit(c.m_targetid))
      throw std::invalid_argument("Either source or target name not found inside circuit!");
    vector<Connection>& fl = m_forwardConnections[c.m_sourceid];
    if (std::find(fl.begin(), fl.end(), c) == fl.end()) // connection not in forward list (and thus not in backward list)
    {
      vector<Connection>& bl = m_backwardConnections[c.m_targetid];
      fl.push_back(c);
      bl.push_back(c);
      m_isGraphDirty = true;
    }
  }

  void Circuit::refreshProcQueue()
  {
    unordered_set<int> closed_set;
    deque<int> dependencyQueue({ m_sinkId });
    deque<int>& processQueue = m_processQueue;
    processQueue.clear();
    while (!dependencyQueue.empty())
    {
      int currUnit = dependencyQueue.back();
      dependencyQueue.pop_back();
      processQueue.push_front(currUnit);
      if (!m_backwardConnections[currUnit].size())
        continue;
      for (int i = 0; i < m_backwardConnections[currUnit].size(); i++)
      {
        int sourceid = m_backwardConnections[currUnit][i].m_sourceid;
        if (closed_set.find(sourceid) == closed_set.end())
        {
          dependencyQueue.push_back(sourceid);
          closed_set.insert(sourceid);
        }
      }
    }
  }

  double Circuit::tick()
  {

    if (m_isGraphDirty)
    {
      refreshProcQueue();
      m_isGraphDirty = false;
    }

    for (int currUnitId : m_processQueue)
    {
      Unit& source = *m_units[currUnitId];
      source.tick();
      for (Connection& c : m_forwardConnections[currUnitId])
      {
        m_units[c.m_targetid]->modifyParameter(c.m_targetport, source.getLastOutput(), c.m_action);
      }
    }
    return  m_units[m_sinkId]->getLastOutput();
  }


  void Circuit::setFs(double fs)
  {
    for (Unit* unit : m_units)
    {
      unit->setFs(fs);
    }
  }

  Circuit* Circuit::clone()
  {

    Circuit* circ = (Circuit*)cloneImpl();

    // Clone units 
    for (int currUnitId=0;currUnitId<m_units.size();currUnitId++)
    {
      circ->addUnit(m_units[currUnitId]->clone());
    }
    circ->setSinkId(m_sinkId);

    // Clone connections
    for(int i=0; i < m_forwardConnections.size(); i++)
    {
      for (int j = 0; j < m_forwardConnections[i].size(); j++)
      {
        circ->addConnection(Connection(m_forwardConnections[i][j]));
      }
    }
    for (MIDIConnectionMap::iterator it = m_midiConnections.begin(); it != m_midiConnections.end(); it++)
    {
      vector<MIDIConnection>& l = it->second;
      for (MIDIConnection mc : l)
      {
        circ->addMIDIConnection(MIDIConnection(mc));
      }
    }
    circ->m_isGraphDirty = m_isGraphDirty;
    circ->m_processQueue = m_processQueue;
    return circ;
  }

  bool Circuit::hasUnit(string name)
  {
    return m_unitmap.find(name) != m_unitmap.end();
  }

  bool Circuit::hasUnit(int uid)
  {
    return m_units.size() >= uid;
  }

  void Circuit::addMIDIConnection(MIDIConnection& c)
  {
    m_midiConnections[c.m_sourceid].push_back(c);
  }

  void Circuit::sendMIDICC(const IMidiMsg& midimsg)
  {
    IMidiMsg::EControlChangeMsg cc = midimsg.ControlChangeIdx();
    double val = midimsg.ControlChange(cc);
    vector<MIDIConnection>& connections = m_midiConnections[cc];
    for (int i = 0; i < connections.size(); i++)
    {
      modifyParameter(connections[i].m_targetid, connections[i].m_targetport, val, connections[i].m_action);
    }
  }

  int Circuit::getUnitId(string name)
  {
    return m_unitmap.at(name);
  }
}

