#ifndef __UNIT__
#define __UNIT__
#include "UnitParameter.h"
#include "DSPMath.h"
#include "GallantSignal.h"
#include <unordered_map>
#include <vector>
#include <tuple>

using Gallant::Signal1;
using std::tuple;
using std::unordered_map;
using std::vector;

typedef  unordered_map<string, int> IDMap; //!< string->integral id translation map 

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
    void tick();
    virtual void setFs(double fs) { m_Fs = fs; };
    double getFs() const { return m_Fs; };
    const vector<double>& getLastOutputBuffer() const { return m_output; };
    double getLastOutput() const { return m_output[m_bufind]; };
    virtual void resizeOutputBuffer(size_t newbufsize){ m_output.resize(newbufsize); }
    /*!
     *\brief Modifies the value of the parameter associated with portid.
     */
    void modifyParameter(int portid, double val, MOD_ACTION action);
    bool hasParameter(string name) { return m_parammap.find(name) != m_parammap.end(); };
    double readParam(string pname) const { return *m_params[m_parammap.at(pname)]; };
    double readParam(int id) const { return *m_params[id]; };
    UnitParameter& getParam(string pname) { return *m_params[m_parammap.at(pname)]; }
    UnitParameter& getParam(int pid) { return *m_params[pid]; }
    vector<string> getParameterNames() const;
    int getParamId(string name);
    Circuit& getParent() const { return *m_parent; };
    string getName() const { return m_name; }
    void setName(string name){ m_name = name; }
    Unit* clone() const;
  protected:
    typedef vector<UnitParameter*> ParamVec;
    ParamVec m_params;
    IDMap m_parammap;
    string m_name;
    Circuit* m_parent;
    double m_Fs;
    vector<double> m_output;
    virtual void process(int bufind) = 0; //<! should add its result to m_output[bufind]
    UnitParameter& addParam(string name, int id, PARAM_TYPE ptype, const double min, const double max, const bool isHidden=false);
    UnitParameter& addParam(string name, PARAM_TYPE ptype, const double min, const double max, const bool isHidden=false);
    UnitParameter& addEnumParam(string name, const vector<string> choice_names);
    
  private:
    int m_bufind;
    virtual Unit* cloneImpl() const = 0;
    virtual void beginProcessing() {};
    virtual void finishProcessing() {}; //<! Allows parent classes to apply common processing to child class outputs.
  };

  /*
   * MISC UTILITY UNITS
   */
  class AccumulatingUnit : public Unit
  {
  public:
    AccumulatingUnit(string name) : Unit(name),
      m_input(addParam("input", DOUBLE_TYPE, -1, 1, true)),
      m_gain(addParam("gain", DOUBLE_TYPE, 0, 1))
    {}
    AccumulatingUnit(const AccumulatingUnit& other) : AccumulatingUnit(other.m_name)
    {}
    virtual ~AccumulatingUnit() {};
  protected:
    virtual void process(int bufind)
    {
      m_output[bufind] = m_input*m_gain;
    }
  private:
    UnitParameter& m_input;
    UnitParameter& m_gain;
    virtual Unit* cloneImpl() const { return new AccumulatingUnit(*this); };
  };
}
#endif
