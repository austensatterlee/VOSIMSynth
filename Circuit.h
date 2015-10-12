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

class Circuit
{
public:
  Circuit();
  Circuit(const Circuit& c);
  virtual ~Circuit();
  void addUnit(string name, Unit* unit);
  void setSink(string name, SinkUnit* unit);
  void setSource(string name, SourceUnit* unit);
  void modUnitParam(string uname, string pname, MOD_ACTION action, double val);
  void addConnection(Connection* connection);
  void trigger();
  void release();
  double tick();
  void setFs(double fs);
protected:
  map<string,Unit*> m_units;
  map<string, list<Connection*>> m_forwardConnections;
  map<string, list<Connection*>> m_backwardConnections;
  string m_sinkName;
  string m_sourceName;
  SinkUnit* m_sinkUnit;
  SourceUnit* m_sourceUnit;
};

#endif // __Circuit__