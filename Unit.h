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
    Unit(const Unit& u);
    virtual ~Unit();
    double tick();
    void setFs(const double fs) { m_Fs = fs; };
    double getLastOutput() { return m_lastOutput; };
    void modifyParameter(const string pname, const MOD_ACTION action, double val);
    bool hasParameter(string name){ return m_params.find(name)!=m_params.end(); };
    double getParam(string pname) { return m_params[pname]->get(); };
    double getParam(string pname) const { return m_params.at(pname)->get(); };
    Circuit& getParent() const { return *parent; };
    virtual Unit* clone() const = 0;
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
    void beginProcessing();
    virtual double finishProcessing(double o){
      return o;
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
    virtual Unit* clone() const { return new AccumulatingUnit(); };
  protected:
    virtual double process()
    {
      return getParam("input");
    }
  };
}
#endif
