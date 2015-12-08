#include "Circuit.h"
#include <stdexcept>
#include <vector>
#include <unordered_set>

using std::vector;
using std::deque;
using std::unordered_set;
namespace syn
{
  Circuit::~Circuit()
  {
    // Delete units
    for (std::pair<int, Unit*> unitpair : m_units)
    {
      delete unitpair.second;
    }
    m_units.clear();
  }

  int Circuit::addUnit(Unit* unit, int uid)
  {
    if (!hasUnit(unit->getName()))
    {
      m_forwardConnections[uid] = vector<ConnectionMetadata>();
      m_backwardConnections[uid] = vector<ConnectionMetadata>();
      m_unitmap[unit->getName()] = uid;
      m_units[uid] = unit;
      unit->m_parent = this;
      unit->resizeOutputBuffer(m_bufsize);
      unit->setFs(m_Fs);
      m_isGraphDirty = true;
      while (m_units.find(m_nextUid) != m_units.end())
      {
        m_nextUid++;
      }
      return uid;
    }
    return -1;
  }

  int Circuit::addUnit(Unit* unit)
  {
    return addUnit(unit, m_nextUid);
  }

  bool Circuit::removeUnit(int uid)
  {
    if (hasUnit(uid))
    {
      // Surgically remove any mention of this unit from connection metadata
      const vector<ConnectionMetadata>& bwconns = m_backwardConnections[uid];
      for (int i = 0; i < bwconns.size(); i++)
      {
        vector<ConnectionMetadata>& connsto = m_forwardConnections[bwconns[i].srcid];
        for (int j = 0; j < connsto.size(); j++)
        {
          if (connsto[j].targetid == uid)
          {
            connsto.erase(connsto.begin() + j);
            j--;
          }
        }
      }
      const vector<ConnectionMetadata>& fwconns = m_forwardConnections[uid];
      for (int i = 0; i < fwconns.size(); i++)
      {
        vector<ConnectionMetadata>& connsfrom = m_backwardConnections[fwconns[i].targetid];
        for (int j = 0; j < connsfrom.size(); j++)
        {
          if (connsfrom[j].srcid == uid)
          {
            connsfrom.erase(connsfrom.begin() + j);
            j--;
          }
        }
      }

      m_forwardConnections.erase(uid);
      m_backwardConnections.erase(uid);
      m_unitmap.erase(m_units[uid]->getName());
      Unit* unit = m_units[uid];
      m_units.erase(uid);
      delete unit;
      m_isGraphDirty = true;
      if (uid == m_sinkId) m_sinkId = -1;
      m_nextUid = uid;
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

  double Circuit::readParam(int uid, int param)
  {
    return m_units[uid]->readParam(param);
  }

  vector<tuple<string, string>> Circuit::getParameterNames()
  {
    vector<tuple<string, string>> all_pnames;
    if (m_sinkId < 0)
    {
      return all_pnames;
    }
    if (m_isGraphDirty)
    {
      refreshProcQueue();
    }
    for (int i = 0; i < m_processQueue.size(); i++)
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

  bool Circuit::addConnection(ConnectionMetadata c)
  {
    if (!hasUnit(c.srcid) || !hasUnit(c.targetid))
    {
      DBGMSG("Error connecting source (%d) and target (%d) inside circuit!", c.srcid, c.targetid);
      throw std::invalid_argument("Either source or target name not found inside circuit!");
    }
    if (c.targetid == c.srcid)
    {
      return false;
    }
    vector<ConnectionMetadata>& fl = m_forwardConnections[c.srcid];
    if (find(fl.begin(), fl.end(), c) == fl.end()) // connection not in forward list (and thus not in backward list)
    {
      vector<ConnectionMetadata>& bl = m_backwardConnections[c.targetid];
      fl.push_back(c);
      bl.push_back(c);
      m_units[c.targetid]->m_params[c.portid]->addConnection(&(m_units[c.srcid]->getLastOutputBuffer()), c.action);
      m_isGraphDirty = true;
    }
    return true;
  }

  bool Circuit::addConnection(string srcname, string targetname, string pname, MOD_ACTION action)
  {
    int sourceid = getUnitId(srcname);
    int targetid = getUnitId(targetname);
    int paramid = getUnit(targetid).getParamId(pname);
    return addConnection({ sourceid,targetid,paramid,action });
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
        int sourceid = m_backwardConnections[currUnit][i].srcid;
        if (closed_set.find(sourceid) == closed_set.end())
        {
          dependencyQueue.push_front(sourceid);
          closed_set.insert(sourceid);
        }
      }
    }
  }

  void Circuit::tick()
  {
    if (m_sinkId < 0)
    {
      return;
    }
    if (m_isGraphDirty)
    {
      refreshProcQueue();
      m_isGraphDirty = false;
    }

    for (int currUnitId : m_processQueue)
    {
      Unit& source = *m_units[currUnitId];
      source.tick();
    }
  }

  void Circuit::setFs(double fs)
  {
    m_Fs = fs;
    for (std::pair<int, Unit*> unit : m_units)
    {
      unit.second->setFs(fs);
    }
  }

  void Circuit::setBufSize(size_t bufsize)
  {
    m_bufsize = bufsize;
    for (std::pair<int, Unit*> unitpair : m_units)
    {
      unitpair.second->resizeOutputBuffer(bufsize);
    }
  }

  Circuit* Circuit::clone()
  {
    Circuit* circ = (Circuit*)cloneImpl();
    circ->m_bufsize = m_bufsize;

    // Clone units
    for (std::pair<int, Unit*> unitpair : m_units)
    {
      circ->addUnit(unitpair.second->clone(), unitpair.first);
    }
    circ->setSinkId(m_sinkId);
    circ->setFs(m_Fs);

    // Clone connections
    for (std::pair<int, vector<ConnectionMetadata>> connpair : m_forwardConnections)
    {
      for (int j = 0; j < connpair.second.size(); j++)
      {
        circ->addConnection(connpair.second[j]);
      }
    }
    circ->m_isGraphDirty = m_isGraphDirty;
    circ->m_processQueue = m_processQueue;
    circ->m_nextUid = m_nextUid;
    return circ;
  }

  bool Circuit::hasUnit(string name) const
  {
    return m_unitmap.find(name) != m_unitmap.end();
  }

  bool Circuit::hasUnit(int uid) const
  {
    return m_units.find(uid) != m_units.end();
  }

  const vector<ConnectionMetadata>& Circuit::getConnectionsTo(int unitid) const
  {
    if (hasUnit(unitid))
    {
      return m_backwardConnections.at(unitid);
    }
    return{};
  }

  int Circuit::getUnitId(string name)
  {
    if (m_unitmap.find(name) == m_unitmap.end())
    {
      DBGMSG("Unit (%s) not found in circuit.", name);
      throw std::invalid_argument("Unit (" + name + ") not found in circuit.");
    }
    return m_unitmap.at(name);
  }

  int Circuit::getUnitId(Unit* unit)
  {
    int uid = -1;
    for (std::pair<int, Unit*> unitpair : m_units)
    {
      if (unitpair.second == unit) uid = unitpair.first;
    }
    if (uid == -1)
    {
      DBGMSG("Unit not found in circuit.");
      throw std::invalid_argument("Unit not found in circuit.");
    }
    return uid;
  }

  vector<int> Circuit::getUnitIds() const
  {
    vector<int> unitIds;
    for (std::pair<int,Unit*> unitpair : m_units) {
      unitIds.push_back(unitpair.first);
    }
    return unitIds;
  }
}