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
    }
  }

  bool Circuit::addUnit(Unit* unit, int uid)
  {
    if (m_unitmap.find(unit->getName()) == m_unitmap.end())
    {
      m_forwardConnections.push_back(vector<ConnectionMetadata>());
      m_backwardConnections.push_back(vector<ConnectionMetadata>());
      
      m_unitmap[unit->getName()] = uid;
      while (uid >= m_units.size())
      {
        m_units.push_back({0});
      }
      m_units[uid]=unit;
      unit->m_parent = this;
      unit->resizeOutputBuffer(m_bufsize);
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

  void Circuit::addConnection(ConnectionMetadata c)
  {
    if (!hasUnit(c.srcid) || !hasUnit(c.targetid)){
      throw std::invalid_argument("Either source or target name not found inside circuit!");
      DBGMSG("Error connecting source (%d) and target (%d) inside circuit!", c.srcid, c.targetid);
    }
    vector<ConnectionMetadata>& fl = m_forwardConnections[c.srcid];
    if (std::find(fl.begin(), fl.end(), c) == fl.end()) // connection not in forward list (and thus not in backward list)
    {
      vector<ConnectionMetadata>& bl = m_backwardConnections[c.targetid];
      fl.push_back(c);
      bl.push_back(c);
      m_units[c.targetid]->m_params[c.portid]->addConnection(&(m_units[c.srcid]->getLastOutputBuffer()),c.action);
      m_isGraphDirty = true;
    }
  }

  void Circuit::addConnection(string srcname, string targetname, string pname, MOD_ACTION action)
  {
    int sourceid = getUnitId(srcname);
    int targetid = getUnitId(targetname);
    int paramid = getUnit(targetid).getParamId(pname);
    addConnection({sourceid,targetid,paramid,action});
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
    for (Unit* unit : m_units)
    {
      unit->setFs(fs);
    }
  }

  void Circuit::setBufSize(size_t bufsize)
  {
    m_bufsize = bufsize;
    for (int i = 0; i < m_units.size(); i++)
    {
      m_units[i]->resizeOutputBuffer(bufsize);
    }
  }

  Circuit* Circuit::clone()
  {
    Circuit* circ = (Circuit*)cloneImpl();
    circ->m_bufsize = m_bufsize;

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
        circ->addConnection(m_forwardConnections[i][j]);
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

