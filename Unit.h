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

typedef  unordered_map<string, int> IDMap; //!< string<->integral id translation map 

namespace syn
{
  class Circuit; // forward decl.

  class Unit
  {
    friend class Circuit;
  public:
    Unit(string name);
    virtual ~Unit();
    /*!
     * \brief Runs the unit for a single tick. Afterwards, the result is retrievable with getLastOutput().
     */
    double tick();
    void setFs(const double fs) { m_Fs = fs; };
    double getLastOutput() { return m_lastOutput; };
    /*! 
     *\brief Modifies the value of the parameter associated with portid. Primarily used by Circuit to process connections.
     *
     */
    void modifyParameter(int portid, double val, MOD_ACTION action);
    bool hasParameter(string name){ return m_parammap.find(name)!=m_parammap.end(); };
    double readParam(string pname) const { return m_params[m_parammap.at(pname)].get(); };
    double readParam(int id) const { return m_params[id].get(); };
    UnitParameter& getParam(string pname) { return m_params[m_parammap.at(pname)]; }
    UnitParameter& getParam(int pid) { return m_params[pid]; }
    vector<string> getParameterNames() const;
    int getParamId(string name);
    Circuit& getParent() const { return *parent; };
    const string getName() const { return m_name; }
    Unit* clone() const;
    Signal1<double> m_extOutPort;
  protected:
    typedef vector<UnitParameter> ParamVec;
    virtual double process() = 0;
    void addParam(UnitParameter& param);
    ParamVec m_params;
    IDMap m_parammap;
    string m_name;
    Circuit* parent;
    double m_Fs;
    double m_lastOutput;
  private:
    virtual Unit* cloneImpl() const = 0;
    void beginProcessing();
    virtual double finishProcessing(double o)
    {
      return o;
    };
  };

  /*
   * MISC UTILITY UNITS
   */
  class AccumulatingUnit : public Unit
  {
  public:
    AccumulatingUnit(string name) : Unit(name) {
      addParam(UnitParameter{ "input", 0.0, 0.0, 0.0, DOUBLE_TYPE });
      addParam(UnitParameter{ "gain", 1.0, 0.0, 1.0, DOUBLE_TYPE, false });
    }
    virtual ~AccumulatingUnit() {};
  protected:
    virtual double process()
    {
      return readParam(0)*readParam(1);
    }
  private:
    virtual Unit* cloneImpl() const { return new AccumulatingUnit(getName()); };
  };
}
#endif
