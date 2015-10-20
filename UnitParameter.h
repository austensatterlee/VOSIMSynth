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
    bool m_isHidden;
    bool m_isDirty;
    bool m_clamp;
    string m_name;
    double m_min;
    double m_max;

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
    UnitParameter(string name, double base, double min, double max, bool m_isHidden = false, PARAM_STATE state = ACTIVE) :
      m_name(name),
      m_base(base),
      m_min(min),
      m_max(max),
      m_isHidden(m_isHidden),
      m_isDirty(false),
      m_state(state),
      m_curr(0)
    {
      if (m_max <= m_min)
      {
        m_clamp = false;
        m_min = 0;
        m_max = 1;
      }
    };
    UnitParameter(const UnitParameter& p);
    virtual ~UnitParameter() {};
    bool isDirty() const
    {
      return m_isDirty;
    }
    bool isHidden() const
    {
      return m_isHidden;
    }
    double getBase() const
    {
      return m_base;
    }
    string getName() const
    {
      return m_name;
    }
    double get() const
    {
      return m_curr;
    }
    void setHidden(bool ishidden = true)
    {
      m_isHidden = ishidden;
    }
    
    void tick(PARAM_STATE state);
    void tick();
    void mod(MOD_ACTION action, double val);
    double getMin(){return m_min;}
    double getMax(){return m_max;}

  };
}
#endif // __Parameter__