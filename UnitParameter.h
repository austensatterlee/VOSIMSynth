#ifndef __Parameter__
#define __Parameter__

#include <string>
#include "IControl.h"

using std::string;
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
    BOOL_TYPE
  };

  class UnitParameter
  {
  protected:
    string m_name;
    int m_id;
    double m_baseValue;
    double m_currValue;
    double m_min, m_max;
    PARAM_TYPE m_type;
    const IControl* m_controller;
    bool m_isHidden;
  public:
    UnitParameter(string name, int id, PARAM_TYPE ptype, double min, double max, bool isHidden=false) :
      m_name(name),
      m_id(id),
      m_type(ptype),
      m_min(min),
      m_max(max),
      m_controller(nullptr),
      m_isHidden(isHidden)
    {
      mod(0.5*(m_max + m_min), SET);
    }
    UnitParameter() : UnitParameter("uninitialized", -1, DOUBLE_TYPE, 0, 0)
    {}
    UnitParameter(const UnitParameter& other) :
      UnitParameter(other.m_name, other.m_id, other.m_type, other.m_min, other.m_max)
    {
      mod(other.m_baseValue, SET);
    }
    virtual ~UnitParameter() {};

    bool operator== (const UnitParameter& p) const;
    virtual void mod(double amt, MOD_ACTION action);
    operator double() const { return m_currValue; }
    /**
     * \brief Prepare the parameter for the next sample by resetting to the base value
     */
    void reset()
    {
      m_currValue = m_baseValue;
    }

    const string getName() const { return m_name; };
    const int getId() const { return m_id; };
    double getBase() const { return m_baseValue; }
    double getMin() const { return m_min; }
    double getMax() const { return m_max; }
    const IControl* getController() const { return m_controller; }
    bool hasController() const { return m_controller!=nullptr; }
    bool isHidden() const {return m_isHidden; }
    void setController(const IControl* controller);
    void unsetController(const IControl* controller);
    PARAM_TYPE getType() const {return m_type; }
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
      return other;
    }
  private:
    virtual UnitParameter* cloneImpl() const { return new UnitParameter(*this); }
  };
}
#endif // __Parameter__