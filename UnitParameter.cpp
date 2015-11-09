#include "UnitParameter.h"
#include "Unit.h"
#include <utility>

bool syn::UnitParameter::operator==(const UnitParameter& p) const
{
  return m_id == p.m_id && m_name == p.m_name && m_type == p.m_type &&\
    m_min == p.m_min && m_max == p.m_max && m_controller == p.m_controller;
}

void syn::UnitParameter::mod(double amt, MOD_ACTION action)
{
  if (action == SET && amt != m_baseValue)
  {
    m_baseValue = amt;
    m_currValue = m_baseValue;
    m_needsUpdate = true;
  }
  else if (action == ADD && amt != 0)
  {
    m_currValue += amt;
    m_needsUpdate = true;
  }
  else if (action == SCALE && amt != 1)
  {
    m_currValue *= amt;
    m_needsUpdate = true;
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

void syn::UnitParameter::addValueName(double value, string value_name)
{
  m_valueNames[value] = value_name;
}

void syn::UnitParameter::initIParam(IParam* iparam)
{
  const char* name = (m_parent->getName() + "-" + m_name).c_str();
  const char* groupname = m_parent->getName().c_str();
  const char* label = m_name.c_str();
  switch (m_type)
  {
  case DOUBLE_TYPE:
    iparam->InitDouble(name, m_baseValue, m_min, m_max, 1e-3, label, groupname);
    break;
  case INT_TYPE:
    iparam->InitInt(name, m_baseValue, m_min, m_max, label, groupname);
    break;
  case ENUM_TYPE:
    iparam->InitEnum(name, m_baseValue, m_max, label, groupname);
    for (std::pair<int, string> p : m_valueNames)
    {
      iparam->SetDisplayText(p.first, p.second.c_str());
    }
    break;
  case BOOL_TYPE:
    iparam->InitBool(name,m_baseValue, label,groupname);
    break;
  }
}