#ifndef __UNIT__
#define __UNIT__
#include "DSPMath.h"
#include "Parameter.h"
#include "Connection.h"
#include <map>
#include <list>
#include <tuple>


class Unit
{
public:
  Unit();
  Unit(const Unit& u);
  virtual ~Unit();
  double tick();
  void setFs(const double fs) { m_Fs = fs; };
  double getLastOutput() const { return m_lastOutput; };
  void modifyParameter(const string pname, const MOD_ACTION action, double val);
  double getParam(string pname)
  {
    return m_params[pname].get();
  }
protected:
  virtual double process() {return 0.0;};
  void addParams(const list<tuple<string,Parameter>>);
  void addParams(const list<string> paramnames);
  map<string, Parameter> m_params;
  double m_Fs;
private:
  double m_lastOutput;
  virtual double finishProcessing(double);
  void beginProcessing();
};

#endif
