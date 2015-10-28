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
/**
 * \class Unit
 *
 * \brief Units encapsulate a discrete processor with an internal state
 *
 * A unit is composed of N state variables, a single output, and a transition function which produces a new output
 * given the state of the Unit and the last output. Units expose some of their state variables as inputs in the
 * form of numbered parameters (UnitParameter).
 *
 */
  class Unit
  {
    friend class Circuit;
  public:
    Unit(string name);
    virtual ~Unit();
    /*!
     * \brief Runs the unit for the specified number of ticks. The result is accessed via getLastOutputBuffer().
     */
    void tick(size_t nsamples, vector<Connection*>& connections);
    void setFs(const double fs) { m_Fs = fs; };
    double getFs() const { return m_Fs; };
    double getLastOutput() const { return m_output.size() ? m_output.back() : 0; };
    const vector<double>& getLastOutputBuffer() const { return m_output; };
    /*!
     *\brief Modifies the value of the parameter associated with portid.
     *       Primarily used by Circuit to process connections.
     *
     */
    void modifyParameter(int portid, double val, MOD_ACTION action);
    bool hasParameter(string name) { return m_parammap.find(name) != m_parammap.end(); };
    double readParam(string pname) const { return *m_params[m_parammap.at(pname)]; };
    double readParam(int id) const { return *m_params[id]; };
    UnitParameter& getParam(string pname) { return *m_params[m_parammap.at(pname)]; }
    UnitParameter& getParam(int pid) { return *m_params[pid]; }
    vector<string> getParameterNames() const;
    int getParamId(string name);
    Circuit& getParent() const { return *parent; };
    const string getName() const { return m_name; }
    Unit* clone() const;
    Signal1<double> m_extOutPort;
  protected:
    typedef vector<UnitParameter*> ParamVec;
    virtual double process() = 0;
    UnitParameter& addParam(string name, int id, PARAM_TYPE ptype, double min, double max);
    UnitParameter& addParam(string name, PARAM_TYPE ptype, double min, double max);
    ParamVec m_params;
    IDMap m_parammap;
    string m_name;
    Circuit* parent;
    double m_Fs;
    vector<double> m_output;
  private:
    void tickParams(vector<Connection*>& connections);
    virtual Unit* cloneImpl() const = 0;
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
    AccumulatingUnit(string name) : Unit(name),
      m_input(addParam("input", DOUBLE_TYPE, -1, 1)),
      m_gain(addParam("gain", DOUBLE_TYPE, 0, 1))
    {}
    AccumulatingUnit(const AccumulatingUnit& other) : AccumulatingUnit(other.m_name)
    {}
    virtual ~AccumulatingUnit() {};
  protected:
    virtual double process()
    {
      return m_input*m_gain;
    }
  private:
    UnitParameter& m_input;
    UnitParameter& m_gain;
    virtual Unit* cloneImpl() const { return new AccumulatingUnit(*this); };
  };
}
#endif
