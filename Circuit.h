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
    bool addUnit(string name, Unit* unit);
    void setSink(string name);
    /*!
     * \brief Specify a connection from one unit's output to another unit's parameter.
     * \sa Connection
     */
    void addConnection(Connection* connection);
    void addMIDIConnection(MIDIConnection* CC);
    /*!
     * \brief Modify a Unit's parameter
     * \param uname Unit name
     * \param pname Parameter name
     * \param action A MOD_ACTION specifying how the modification should be applied
     * \param val The value used to perform the actual modification     *
     */
    void modifyParameter(string uname, string pname, MOD_ACTION action, double val);
    /*!
     * \brief Trigger a CC event that will be routed to internal components as specified by previous calls to ::addMIDIConnection.
     * \sa IMidiMsg
     * \sa addMIDIConnection
     */
    vector<tuple<string,string>> getParameterNames() const;
    void sendMIDICC(IMidiMsg *midimsg);
    Unit* getUnit(string name) { return m_units[name]; };
    Unit* getUnit(string name) const { return m_units.at(name); };
    double tick();
    void setFs(double fs);
    virtual Circuit* clone();
    bool hasUnit(string name);
  protected:
    typedef  map<string, Unit*> UnitMap;
    typedef  map<string, list<Connection*>> ConnectionMap;
    typedef  map<IMidiMsg::EControlChangeMsg, list<MIDIConnection*>> MIDIConnectionMap;
    UnitMap m_units;
    ConnectionMap m_forwardConnections;
    ConnectionMap m_backwardConnections;
    MIDIConnectionMap m_midiConnections;
    deque<string> m_processQueue; //!< cache storage for the linearized version of unit dependencies
    string m_sinkName;
    bool m_isGraphDirty = true; //!< indicates whether or not the graph's linearization should be recomputed
  };
};
#endif // __Circuit__