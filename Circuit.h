#ifndef __Circuit__
#define __Circuit__

#include "Unit.h"
#include "Connection.h"
#include "SourceUnit.h"
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
      m_nextUid(0)
    {}
    virtual ~Circuit();
    Circuit* clone();
    bool addUnit(Unit* unit);
    bool addUnit(Unit* unit, int uid);
    void setSinkId(int id);
    void setSinkName(string name);
    /*!
     * \brief Specify a connection from one unit's output to another unit's parameter.
     * \sa Connection
     */
    void addConnection(string srcname, string targetname, string pname, MOD_ACTION action);
    void addConnection(Connection* c);
    void addMIDIConnection(MIDIConnection* c);
    void addMIDIConnection(IMidiMsg::EControlChangeMsg msg, string targetname, string pname, MOD_ACTION action);
    /**
     * \brief Manually modify a Unit's parameter
     */
    void modifyParameter(int uid, int param, double val, MOD_ACTION action);
    void modifyParameter(string uname, string param, double val, MOD_ACTION action);
    double readParam(int uid, int param);

    /**
    * \brief Trigger a CC event that will be routed to internal components as specified by previous calls to ::addMIDIConnection.
    */
    void sendMIDICC(const IMidiMsg& midimsg);

    vector<tuple<string, string>> getParameterNames() const;
    Unit& getUnit(string name) { return *m_units[m_unitmap[name]]; };
    const Unit& getUnit(string name) const { return *m_units.at(m_unitmap.at(name)); };
    Unit& getUnit(int id) { return *m_units[id]; };
    const Unit& getUnit(int id) const { return *m_units.at(id); };
    int getUnitId(string name);
    /**
     * \brief Generate the requested number of samples. The result can be retrieved using getLastOutputBuffer()
     */
    void tick();
    void setFs(double fs);
    bool hasUnit(string name);
    bool hasUnit(int uid);
    double getLastOutput() const { return m_units[m_sinkId]->getLastOutput(); };
  protected:
    typedef  vector<Unit*> UnitVec;
    typedef  vector<vector<Connection*>> ConnVec;
    typedef  unordered_map<IMidiMsg::EControlChangeMsg, vector<MIDIConnection*>> MIDIConnectionMap;
    UnitVec m_units;
    ConnVec m_forwardConnections;
    ConnVec m_backwardConnections;
    vector<vector<MIDIConnection*>> m_midiConnections;
    MIDIConnectionMap m_midiConnectionMap;
    IDMap m_unitmap;
    deque<int> m_processQueue; //!< cache storage for the linearized version of unit dependencies
    bool m_isGraphDirty = true; //!< indicates whether or not the graph's linearization should be recomputed
    int m_sinkId;
    void tickParams(int unitid);
  private:
    void refreshProcQueue();
    virtual Circuit* cloneImpl() const { return new Circuit(); };
    int m_nextUid;
  };
};
#endif // __Circuit__