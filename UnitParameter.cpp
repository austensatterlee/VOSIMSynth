#include "UnitParameter.h"
namespace syn
{
  UnitParameter::UnitParameter(const UnitParameter& p)
  {
    m_base = p.m_base;
    m_min = p.m_min;
    m_max = p.m_max;
    m_clamp = p.m_clamp;
    m_type = p.m_type;
    m_curr = p.m_curr;
    m_offset = p.m_offset;
    m_scale = p.m_scale;
    m_isHidden = p.m_isHidden;
    m_isDirty = p.m_isDirty;
    m_name = p.m_name;
  }

  void UnitParameter::tick()
  {
    m_wasDirty = !m_isDirty;
    if (m_isDirty)
    {
      m_curr = (m_scale*(m_base + m_offset));

      if (m_clamp)
      {
        if (m_curr > m_max){
          m_curr = m_max;
        }else if (m_curr < m_min){
          m_curr = m_min;
        }
      }
      m_scale = 1.0;
      m_offset = 0.0;
      m_isDirty = false;      
    }
  }

  void UnitParameter::mod(MOD_ACTION action, double val)
  {
    if (action == SET)
    {
      set(val);
    }
    else if (action == ADD)
    {
      add(val);
    }
    else if (action == SCALE)
    {
      scale(val);
    }
    m_isDirty = true;
  }
}