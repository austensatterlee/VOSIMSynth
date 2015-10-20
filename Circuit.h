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


  class Circuit
  {
  public:
    Circuit();
    virtual ~Circuit();
    bool addUnit(Unit* unit);
    void setSinkId(int id);
    void setSinkName(string name);
    /*!
     * \brief Specify a connection from one unit's output to another unit's parameter.
     * \sa Connection
     */
    void addConnection(Connection& c);
    void addConnection(string srcname, string targetname, string pname, MOD_ACTION action);
    void addMIDIConnection(MIDIConnection& c);
    /*!
     * \brief Modify a Unit's parameter
     * \param uid Unit name/id
     * \param param Parameter name/id
     * \param action A MOD_ACTION specifying how the modification should be applied
     * \param val The value used to perform the actual modification     
     */
    void modifyParameter(int uid, int param, double val, MOD_ACTION action);
    void modifyParameter(string uname, string param, double val, MOD_ACTION action);

    /*!
    * \brief Trigger a CC event that will be routed to internal components as specified by previous calls to ::addMIDIConnection.
    * \sa IMidiMsg
    * \sa addMIDIConnection
    */
    void sendMIDICC(const IMidiMsg& midimsg);
    vector<tuple<string, string>> getParameterNames() const;
    Unit& getUnit(string name) { return *m_units[m_unitmap[name]]; };
    const Unit& getUnit(string name) const { return *m_units.at(m_unitmap.at(name)); };
    Unit& getUnit(int id) { return *m_units[id]; };
    const Unit& getUnit(int id) const { return *m_units.at(id); };
    int getUnitId(string name);
    double tick();
    void setFs(double fs);
    Circuit* clone();
    bool hasUnit(string name);
    bool hasUnit(int uid);
  protected:
    typedef  vector<Unit*> UnitVec;
    typedef  vector<vector<Connection>> ConnVec;
    typedef  unordered_map<IMidiMsg::EControlChangeMsg, vector<MIDIConnection>> MIDIConnectionMap;
    UnitVec m_units;
    ConnVec m_forwardConnections;
    ConnVec m_backwardConnections;
    MIDIConnectionMap m_midiConnections;
    IDMap m_unitmap;
    deque<int> m_processQueue; //!< cache storage for the linearized version of unit dependencies
    bool m_isGraphDirty = true; //!< indicates whether or not the graph's linearization should be recomputed
    int m_sinkId;
  private:
    void refreshProcQueue();
    virtual Circuit* cloneImpl() const { return new Circuit(); };
  };
};
#endif // __Circuit__