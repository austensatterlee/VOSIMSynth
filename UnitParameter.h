#ifndef __Parameter__
#define __Parameter__

#include "IControl.h"
#include "IParam.h"
#include <string>
#include <vector>
#include <map>

using std::string;
using std::map;
using std::vector;

namespace syn
{
  enum MOD_ACTION
  {
    SET = 0,
    ADD,
    SCALE,
    NUM_MOD_ACTIONS
  };

  enum PARAM_TYPE
  {
    DOUBLE_TYPE = 0,
    INT_TYPE,
    BOOL_TYPE,
    ENUM_TYPE
  };

  struct Connection
  {
    const vector<double>* srcbuffer;
    MOD_ACTION action;
    bool operator==(const Connection& other) const
    {
      return srcbuffer == other.srcbuffer && action == other.action;
    }
  };

  class Unit;
  class UnitParameter
  {
  protected:
    typedef double(*ParamTransformFunc)(double);
    Unit* m_parent;
    string m_name;
    int m_id;
    double m_defaultValue;
    double m_baseValue;
    double m_currValue;
    double m_lastValue;
    double m_min, m_max;
    PARAM_TYPE m_type;
    const IControl* m_controller;
    bool m_isHidden;
    bool m_needsUpdate;
    map<double, string> m_valueNames;
    vector<Connection> m_connections;
    ParamTransformFunc m_transform_func;
  public:
    UnitParameter(Unit* parent, string name, int id, PARAM_TYPE ptype, double min, double max, double defaultValue, bool ishidden = false) :
      m_parent(parent),
      m_name(name),
      m_id(id),
      m_type(ptype),
      m_min(min),
      m_max(max),
      m_defaultValue(defaultValue),
      m_controller(nullptr),
      m_isHidden(ishidden),
      m_transform_func(nullptr),
      m_connections(0)
    {
      UnitParameter::mod(defaultValue, SET);
      m_currValue = m_baseValue;
    }
    UnitParameter(const UnitParameter& other) :
      UnitParameter(other.m_parent, other.m_name, other.m_id, other.m_type, other.m_min, other.m_max, other.m_defaultValue)
    {
      UnitParameter::mod(other.m_baseValue, SET);
      setTransformFunc(other.m_transform_func);
      m_currValue = m_baseValue;
    }
    virtual ~UnitParameter() {};

    bool operator== (const UnitParameter& p) const;
    virtual void mod(double amt, MOD_ACTION action);
    void addValueName(double value, string value_name);
    void initIParam(IParam* iparam);

    operator double()
    {
      if (m_needsUpdate && m_transform_func != nullptr)
      {
        m_currValue = m_transform_func(m_currValue);
      }
      m_needsUpdate = false;
      return m_currValue;
    }
    /**
     * \brief Prepare the parameter for the next sample by resetting to the base value
     */
    void reset()
    {
      m_currValue = m_baseValue;
      m_needsUpdate = true;
    }

    string getName() const { return m_name; };
    string getString() const;
    int getId() const { return m_id; };
    double getBase() const { return m_baseValue; }
    double getMin() const { return m_min; }
    double getMax() const { return m_max; }
    double getDefault() const { return m_defaultValue; }
    void setDefault(double a_default) { mod(a_default,SET); m_defaultValue = a_default; }
    void setMin(double min) { m_min = min; }
    void setMax(double max) { m_max = max; }
    void setIsHidden(bool ishidden) { m_isHidden = ishidden; }
    const IControl* getController() const { return m_controller; }
    bool hasController() const { return m_controller != nullptr; }
    void setTransformFunc(ParamTransformFunc func) { m_transform_func = func; }
    bool isHidden() const { return m_isHidden; }
    void addConnection(const vector<double>* srcbuffer, MOD_ACTION action)
    {
      m_connections.push_back({ srcbuffer,action });
    }
    int numConnections() const { return m_connections.size(); }
    void pull(int bufind)
    {
      for (int i = 0; i < m_connections.size(); i++)
      {
        mod((*m_connections[i].srcbuffer)[bufind], m_connections[i].action);
      }
    }
    bool isDirty()
    {
      bool isdirty = m_currValue != m_lastValue;
      m_lastValue = m_currValue;
      return isdirty;
    }
    void setController(const IControl* controller);
    void unsetController(const IControl* controller);
    PARAM_TYPE getType() const { return m_type; }
    UnitParameter* clone() const
    {
      UnitParameter* other = cloneImpl();
      other->m_name = m_name;
      other->m_baseValue = m_baseValue;
      other->m_currValue = m_currValue;
      other->m_id = m_id;
      other->m_max = m_max;
      other->m_min = m_min;
      other->m_type = m_type;
      other->m_controller = m_controller;
      other->m_transform_func = m_transform_func;
      other->m_lastValue = m_lastValue;
      other->m_defaultValue = m_defaultValue;
      return other;
    }
  private:
    virtual UnitParameter* cloneImpl() const { return new UnitParameter(*this); }
  };
}
#endif // __Parameter__