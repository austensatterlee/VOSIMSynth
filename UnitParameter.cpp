#include "UnitParameter.h"

bool syn::UnitParameter::operator==(const UnitParameter& p) const
{
  return m_id == p.m_id && m_name == p.m_name && m_type == p.m_type &&\
         m_min == p.m_min && m_max == p.m_max && m_controller==p.m_controller;
}

void syn::UnitParameter::mod(double amt, MOD_ACTION action)
{
  if (action == SET && amt!=m_baseValue)
  {
    m_baseValue = amt;
    m_currValue = m_baseValue;
    m_isDirty = true;
  }
  else if (action == ADD && amt!=0)
  {
    m_currValue += amt;
    m_isDirty = true;
  }
  else if (action == SCALE && amt!=1)
  {
    m_currValue *= amt;
    m_isDirty = true;
  }
}

void syn::UnitParameter::setController(const IControl* controller)
{
  if (!m_controller)
  {
    m_controller = controller;
  }
  else
  {
    throw std::logic_error("UnitParameter already has a controller.");
  }
}

void syn::UnitParameter::unsetController(const IControl* controller)
{
  if (m_controller = controller)
  {
    m_controller = nullptr;
  }
  else
  {
    throw std::logic_error("Only the UnitParameter's assigned controller can disconnect itself.");
  }
}
