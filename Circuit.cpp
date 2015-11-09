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

  Circuit::~Circuit()
  {
    // Delete units
    for (int i = 0; i<m_units.size(); i++)
    {
      delete m_units[i];
      // Delete connections
      for (int j = 0; j < m_forwardConnections[i].size(); j++)
      {
        delete m_forwardConnections[i][j];
      }
    }
    // delete midi connections
    for (MIDIConnectionMap::iterator it = m_midiConnectionMap.begin(); it != m_midiConnectionMap.end(); it++)
    {
      vector<MIDIConnection*>& l = it->second;
      for (MIDIConnection* mc : l)
      {
        delete mc;
      }
    }
  }

  bool Circuit::addUnit(Unit* unit, int uid)
  {
    if (m_unitmap.find(unit->getName()) == m_unitmap.end())
    {
      m_forwardConnections.push_back(vector<Connection*>());
      m_backwardConnections.push_back(vector<Connection*>());
      m_midiConnections.push_back(vector<MIDIConnection*>());
      
      m_unitmap[unit->getName()] = uid;
      while (uid >= m_units.size())
      {
        m_units.push_back({0});
      }
      m_units[uid]=unit;
      unit->parent = this;
      m_isGraphDirty = true;
      return true;
    }
    return false;
  }

  bool Circuit::addUnit(Unit* unit)
  {
    while (m_nextUid < m_units.size() && m_units[m_nextUid])
    {
      m_nextUid++;
    }
    return addUnit(unit, m_nextUid);
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

  double Circuit::readParam(int uid, int param)
  {
    return m_units[uid]->readParam(param);
  }

  vector<tuple<string, string>> Circuit::getParameterNames()
  {
    vector<tuple<string, string>> all_pnames;
    if (m_isGraphDirty)
    {
      refreshProcQueue();
    }
    for (int i=0;i<m_processQueue.size();i++)
    {
      Unit* unit = m_units[m_processQueue[i]];
      vector<string> pnames = unit->getParameterNames();
      for (string pname : pnames)
      {
        tuple<string, string> nametup{ unit->getName(),pname };
        all_pnames.push_back(nametup);
      }
    }
    return all_pnames;
  }

  void Circuit::addConnection(Connection* c)
  {
    if (!hasUnit(c->m_sourceid) || !hasUnit(c->m_targetid)){
      throw std::invalid_argument("Either source or target name not found inside circuit!");
      DBGMSG("Error connecting source (%d) and target (%d) inside circuit!", c->m_sourceid, c->m_targetid);
    }
    vector<Connection*>& fl = m_forwardConnections[c->m_sourceid];
    if (std::find(fl.begin(), fl.end(), c) == fl.end()) // connection not in forward list (and thus not in backward list)
    {
      vector<Connection*>& bl = m_backwardConnections[c->m_targetid];
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
    Connection* c = new Connection(sourceid,targetid,paramid,action);
    addConnection(c);
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
        int sourceid = m_backwardConnections[currUnit][i]->m_sourceid;
        if (closed_set.find(sourceid) == closed_set.end())
        {
          dependencyQueue.push_back(sourceid);
          closed_set.insert(sourceid);
        }
      }
    }
  }

  void Circuit::tick()
  {
    if (m_isGraphDirty)
    {
      refreshProcQueue();
      m_isGraphDirty = false;
    }

    for (int currUnitId : m_processQueue)
    {
      Unit& source = *m_units[currUnitId];
      tickParams(currUnitId);
      source.tick();
      for (Connection* c : m_forwardConnections[currUnitId])
      {
        c->push(source.getLastOutput());
      }
    }
  }



  void Circuit::tickParams(int unitid)
  {
    Unit& unit = getUnit(unitid);
    vector<Connection*>& connections = m_backwardConnections[unitid];
    vector<MIDIConnection*>& midiconnections = m_midiConnections[unitid];
    for (int i = 0; i < midiconnections.size(); i++)
    {
      double amt = midiconnections[i]->pull();
      unit.m_params[midiconnections[i]->m_targetport]->mod(amt, midiconnections[i]->m_action);
    }
    for (int i = 0; i < connections.size(); i++)
    {
      if (!connections[i]->isEmpty())
      {
        double amt = connections[i]->pull();
        unit.m_params[connections[i]->m_targetport]->mod(amt, connections[i]->m_action);
      }
    }
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
        circ->addConnection(new Connection(*m_forwardConnections[i][j]));
      }
    }
    for (MIDIConnectionMap::iterator it = m_midiConnectionMap.begin(); it != m_midiConnectionMap.end(); it++)
    {
      vector<MIDIConnection*>& l = it->second;
      for (MIDIConnection* mc : l)
      {
        circ->addMIDIConnection(new MIDIConnection(*mc));
      }
    }
    circ->m_isGraphDirty = m_isGraphDirty;
    circ->m_processQueue = m_processQueue;
    circ->m_nextUid = m_nextUid;
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

  void Circuit::addMIDIConnection(MIDIConnection* c)
  {
    m_midiConnectionMap[c->m_sourceid].push_back(c);
    m_midiConnections[c->m_targetid].push_back(c);
  }

  void Circuit::addMIDIConnection(IMidiMsg::EControlChangeMsg cc, string targetname, string pname, MOD_ACTION action)
  {
    int targetid = getUnitId(targetname);
    int targetport = m_units[targetid]->getParamId(pname);
    MIDIConnection* c = new MIDIConnection{cc, targetid, targetport, action};
    addMIDIConnection(c);
  }

  void Circuit::sendMIDICC(const IMidiMsg& midimsg)
  {
    IMidiMsg::EControlChangeMsg cc = midimsg.ControlChangeIdx();
    double val = midimsg.ControlChange(cc);
    vector<MIDIConnection*>& connections = m_midiConnectionMap[cc];
    for (int i = 0; i < connections.size(); i++)
    {
      connections[i]->push(val);
    }
  }

  int Circuit::getUnitId(string name)
  {
    if (m_unitmap.find(name) == m_unitmap.end())
    {
      throw std::invalid_argument("Unit ("+name+") not found in circuit.");
      DBGMSG("Unit (%s) not found in circuit.",name);
    }
    return m_unitmap.at(name);
  }
}

