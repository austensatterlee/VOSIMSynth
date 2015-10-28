#ifndef __Parameter__
#define __Parameter__

#include <string>
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

  public:
    UnitParameter(string name, int id, PARAM_TYPE ptype, double min, double max) :
      m_name(name),
      m_id(id),
      m_type(ptype),
      m_min(min),
      m_max(max)
    {
      mod(0.5*(m_max+m_min),SET);
    }
    UnitParameter() : UnitParameter("uninitialized", -1, DOUBLE_TYPE, 0, 0)
    {}
    UnitParameter(const UnitParameter& other) :
      UnitParameter(other.m_name, other.m_id, other.m_type, other.m_min, other.m_max)
    {
      mod(other.m_baseValue, SET);
    }
    virtual ~UnitParameter() {};

    bool operator== (const UnitParameter& p) const
    {
      return m_id == p.m_id && m_name == p.m_name && m_type == p.m_type && m_min == p.m_min && m_max == p.m_max;
    }
    virtual void mod(double amt, MOD_ACTION action)
    {
      if (action == SET)
      {
        m_baseValue = amt;
        m_currValue = m_baseValue;
      }
      else if (action == ADD)
      {
        m_currValue += amt;
      }
      else if (action == SCALE)
      {
        m_currValue *= amt;
      }
    }
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
    double getBase() { return m_baseValue; }
    double getMin() { return m_min; }
    double getMax() { return m_max; }
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
      return other;
    }
  private:
    virtual UnitParameter* cloneImpl() const { return new UnitParameter(*this); }
  };
}
#endif // __Parameter__