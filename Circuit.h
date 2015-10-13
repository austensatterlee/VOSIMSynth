#ifndef __Circuit__
#define __Circuit__

#include "Connection.h"
#include "Unit.h"
#include "SinkUnit.h"
#include "SourceUnit.h"
#include <list>
#include <map>

using std::list;
using std::map;

class Circuit : public Unit
{
public:
  Circuit();
  Circuit(const Circuit& c);
  virtual ~Circuit();
  void addUnit(string name, Unit* unit);
  void setSink(string name, SinkUnit* unit);
  void addConnection(Connection* connection);
  void modifyParameter(string uname, string pname, MOD_ACTION action, double val);
  void addMIDIConnection(MIDIConnection* CC);
  void sendMIDICC(IMidiMsg *midimsg);
  double tick();
  void setFs(double fs);
protected:
  map<string,Unit*> m_units;
  map<string, list<Connection*>> m_forwardConnections;
  map<string, list<Connection*>> m_backwardConnections;
  map<IMidiMsg::EControlChangeMsg, list<MIDIConnection*>> m_midiConnections;
  string m_sinkName;
  SinkUnit* m_sinkUnit;
};

#endif // __Circuit__