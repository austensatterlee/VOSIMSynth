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
    SCALE
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
    double m_base;
    double m_offset = 0;
    double m_scale = 1.0;
    PARAM_TYPE m_type;
    bool m_isHidden;
    bool m_isDirty;
    bool m_wasDirty;
    bool m_clamp;
    string m_name;
    double m_min;
    double m_max;

    void set(double val)
    {
      m_base = val;
    };
    void add(double val)
    {
      m_offset += val;
    };
    void scale(double val)
    {
      m_scale *= val;
    };
  public:
    double m_curr = 0;
    UnitParameter(string name, double base, double min, double max, PARAM_TYPE type=DOUBLE_TYPE, bool m_isHidden = true) :
      m_name(name),
      m_base(base),
      m_min(min),
      m_max(max),
      m_isHidden(m_isHidden),
      m_isDirty(true),
      m_wasDirty(true),
      m_curr(0),
      m_type(type)
    {
      if (m_max <= m_min)
      {
        m_clamp = false;
        m_min = 0;
        m_max = 1;
      }
      else
      {
        m_clamp = true;
      }
    };
    UnitParameter(const UnitParameter& p);
    virtual ~UnitParameter() {};
    bool isDirty() const
    {
      return m_isDirty;
    }
    bool wasDirty() const
    {
      return m_wasDirty;
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
    
    void tick();
    void mod(MOD_ACTION action, double val);
    double getMin() const{return m_min;}
    double getMax() const{return m_max;}

  };
}
#endif // __Parameter__