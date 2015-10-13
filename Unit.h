#ifndef __UNIT__
#define __UNIT__
#include "DSPMath.h"
#include "UnitParameter.h"
#include "Connection.h"
#include "GallantSignal.h"
#include <unordered_map>
#include <vector>
#include <tuple>

using Gallant::Signal1;
using std::tuple;
using std::unordered_map;
using std::vector;

namespace syn
{
  class Circuit;
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
      return m_params[pname]->get();
    }
    Circuit& getParent() const
    {
      return *parent;
    }
    Signal1<double> m_extOutPort;
  protected:
    typedef unordered_map<string, UnitParameter*> UnitParameterMap;
    friend class Circuit;

    virtual double process() { return 0.0; };
    void addParam(string name, UnitParameter* param);
    void addParams(const vector<string> paramnames);
    UnitParameterMap m_params;
    double m_Fs;
    Circuit* parent;
  private:
    double m_lastOutput;
    void beginProcessing();
  };
}
#endif
