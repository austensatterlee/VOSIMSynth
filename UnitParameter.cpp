#include "UnitParameter.h"
namespace syn
{
  UnitParameter::UnitParameter(const UnitParameter& p)
  {
    m_base = p.m_base;
    m_bias = p.m_bias;
    m_curr = p.m_curr;
    m_offset = p.m_offset;
    m_scale = p.m_scale;
    m_sidechain = p.m_sidechain;
    m_state = p.m_state;
    m_isRequired = p.m_isRequired;
    m_isDirty = p.m_isDirty;
    m_name = p.m_name;
  }

  void UnitParameter::tick()
  {
    m_curr = m_scale*m_base + m_sidechain*(m_offset)+m_bias;
    if (m_state == ACTIVE)
    {
      m_offset = 0.0;
      m_bias = 0.0;
      m_scale = 1.0;
      m_sidechain = 1.0;
    }
    m_isDirty = false;
  }

  void UnitParameter::tick(PARAM_STATE state)
  {
    PARAM_STATE oldstate = m_state;
    m_state = state;
    tick();
    m_state = oldstate;
  }

  void UnitParameter::mod(MOD_ACTION action, double val)
  {
    if (action == SET)
    {
      set(val);
    }
    else if (action == BIAS)
    {
      bias(val);
    }
    else if (action == ADD)
    {
      add(val);
    }
    else if (action == SCALE)
    {
      scale(val);
    }
    else if (action == SC)
    {
      sc(val);
    }
    m_isDirty = true;
  }
}