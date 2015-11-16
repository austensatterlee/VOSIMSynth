#ifndef __Circuit__
#define __Circuit__

#include "Unit.h"
#include "SourceUnit.h"
#include "UnitParameter.h"
#include <list>
#include <map>
#include <deque>
#include <tuple>


namespace syn
{
  using std::list;
  using std::map;
  using std::deque;
  using std::tuple;

  struct ConnectionMetadata
  {
    int srcid;
    int targetid;
    int portid;
    MOD_ACTION action;
    bool operator==(const ConnectionMetadata& other)
    {
      return other.srcid==srcid && other.targetid==targetid && other.portid==portid && other.action==action;
    }
  };

  /**
  * \class Circuit
  *
  * \brief A collection of Units and Connections.
  *
  * A circuit allows Units to automatically cooperate by managing the signal flow via Connections.
  * The signal flow of a Circuit starts at the Units without incoming connections and flows towards the Unit
  * marked as the sink. The output of the Circuit is the output of the sink.
  *
  */
  class Circuit
  {
  public:
    Circuit() :
      m_nextUid(0),
      m_bufsize(1),
      m_sinkId(-1)
    {}
    virtual ~Circuit();
    Circuit* clone();
    bool addUnit(Unit* unit);
    bool addUnit(Unit* unit, int uid);
    virtual bool removeUnit(int uid);
    void setSinkId(int id);
    void setSinkName(string name);
    /*!
     * \brief Specify a connection from one unit's output to another unit's parameter.
     * \sa Connection
     */
    void addConnection(string srcname, string targetname, string pname, MOD_ACTION action);
    void addConnection(ConnectionMetadata c);
    /**
     * \brief Manually modify a Unit's parameter
     */
    void modifyParameter(int uid, int param, double val, MOD_ACTION action);
    void modifyParameter(string uname, string param, double val, MOD_ACTION action);
    double readParam(int uid, int param);

    vector<tuple<string, string>> getParameterNames();
    Unit& getUnit(string name) { return *m_units[m_unitmap[name]]; };
    Unit& getUnit(int id) { return *m_units[id]; };
    const Unit& getUnit(int id) const { return *m_units.at(id); };
    int getUnitId(string name);
    int getUnitId(Unit* unit);
    /**
     * \brief Generate the requested number of samples. The result can be retrieved using getLastOutputBuffer()
     */
    void tick();
    void setFs(double fs);
    void setBufSize(size_t bufsize);
    size_t getBufSize(){ return m_bufsize; }
    bool hasUnit(string name);
    bool hasUnit(int uid);
    double getLastOutput() const { return m_units[m_sinkId]->getLastOutput(); };
    const vector<double>& getLastOutputBuffer() const { return m_units[m_sinkId]->getLastOutputBuffer(); };
    const vector<ConnectionMetadata>& getConnectionsTo(int unitid);
  protected:
    typedef  vector<Unit*> UnitVec;
    typedef  vector<vector<ConnectionMetadata>> ConnVec;
    UnitVec m_units;
    ConnVec m_forwardConnections;
    ConnVec m_backwardConnections;
    IDMap m_unitmap;
    deque<int> m_processQueue; //!< cache storage for the linearized version of unit dependencies
    bool m_isGraphDirty = true; //!< indicates whether or not the graph's linearization should be recomputed
    int m_sinkId;
    size_t m_bufsize;
  private:
    void refreshProcQueue();
    virtual Circuit* cloneImpl() const { return new Circuit(); };
    int m_nextUid;
  };
};
#endif // __Circuit__