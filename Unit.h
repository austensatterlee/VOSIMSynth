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
  class Circuit; // forward decl.

  class Unit
  {
    friend class Circuit;
  public:
    Unit();
    virtual ~Unit();
    double tick();
    void setFs(const double fs) { m_Fs = fs; };
    double getLastOutput() { return m_lastOutput; };
    void modifyParameter(const string pname, const MOD_ACTION action, double val);
    bool hasParameter(string name){ return m_params.find(name)!=m_params.end(); };
    double getParam(string pname) { return m_params[pname]->get(); };
    double getParam(string pname) const { return m_params.at(pname)->get(); };
    vector<string> getParameterNames() const;
    Circuit& getParent() const { return *parent; };
    Unit* clone() const;
    Signal1<double> m_extOutPort;
  protected:
    typedef unordered_map<string, UnitParameter*> UnitParameterMap;
    virtual double process() = 0;
    void addParam(UnitParameter* param);
    void addParams(const vector<string> paramnames);
    UnitParameterMap m_params;
    Circuit* parent;
    double m_Fs;
    double m_lastOutput;
  private:
    virtual Unit* cloneImpl() const = 0;
    void beginProcessing();
    virtual double finishProcessing(double o)
    {
      return o*getParam("gain");
    };
  };

  /*
   * MISC UTILITY UNITS
   */
  class AccumulatingUnit : public Unit
  {
  public:
    AccumulatingUnit() : Unit() {
      addParam(new UnitParameter{"input", 0.0,true});
    }
    virtual ~AccumulatingUnit() {};
  protected:
    virtual double process()
    {
      return getParam("input");
    }
  private:
    virtual Unit* cloneImpl() const { return new AccumulatingUnit(); };
  };
}
#endif
