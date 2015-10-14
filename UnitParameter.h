#ifndef __Parameter__
#define __Parameter__
#include <string>
using std::string;
namespace syn
{
  enum PARAM_STATE
  {
    FROZEN = 0,
    ACTIVE
  };

  enum MOD_ACTION
  {
    SET = 0,
    BIAS,
    ADD,
    SCALE,
    SC
  };

  class UnitParameter
  {
  protected:
    double m_base;
    double m_offset = 0;
    double m_bias = 0;
    double m_scale = 1.0;
    double m_sidechain = 1.0;
    PARAM_STATE m_state;
    bool m_isRequired;
    bool m_isDirty;
    string m_name;
    void set(double val)
    {
      m_base = val;
    };
    void bias(double val)
    {
      m_bias = val;
    }
    void add(double val)
    {
      m_offset += val;
    };
    void scale(double val)
    {
      m_scale *= val;
    }
    void sc(double val)
    {
      m_sidechain *= val;
    };
  public:
    double m_curr = 0;
    UnitParameter(string name, double base = 0.0, bool isRequired = false, PARAM_STATE state = ACTIVE) :
      m_name(name),
      m_base(base),
      m_isRequired(isRequired),
      m_isDirty(false),
      m_state(state)
    {};
    UnitParameter(const UnitParameter& p);
    virtual ~UnitParameter() {};
    bool isDirty() const
    {
      return m_isDirty;
    }
    bool isRequired() const
    {
      return m_isRequired;
    }
    string getName() const
    {
      return m_name;
    }
    double get() const
    {
      return m_curr;
    }
    void tick(PARAM_STATE state);
    void tick();
    void mod(MOD_ACTION action, double val);
    

  };
}
#endif // __Parameter__