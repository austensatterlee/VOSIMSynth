#ifndef __Parameter__
#define __Parameter__
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
    bool m_isDirty;
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
    UnitParameter(PARAM_STATE state = ACTIVE) :m_base(0), m_isDirty(false), m_state(state) {};
    UnitParameter(double base, PARAM_STATE state = ACTIVE) :m_base(base), m_isDirty(false), m_state(state) {};
    UnitParameter(const UnitParameter& p);
    virtual ~UnitParameter() {};
    bool isDirty() const
    {
      return m_isDirty;
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