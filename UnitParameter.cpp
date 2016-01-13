#include "UnitParameter.h"
#include "Unit.h"
syn::UnitParameter::UnitParameter(Unit* parent, string name, int id, IParam::EParamType ptype, double min, double max, double defaultValue, bool ishidden) :
	IParam(),
	m_parent(parent),
	m_name(name),
	m_id(id),
	m_controller(nullptr),
	m_isHidden(ishidden),
	m_connections(0)
{
	const char* fullname = (parent->getName() + "-" + m_name).c_str();
	const char* groupname = parent->getName().c_str();
	const char* label = m_name.c_str();
	switch (ptype)
	{
	case kTypeDouble:
		InitDouble(fullname, defaultValue, min, max, (max - min) / 1.0E2, label, groupname);
		break;
	case kTypeInt:
		InitInt(fullname, defaultValue, min, max, label, groupname);
		break;
	case kTypeEnum:
		InitEnum(fullname, defaultValue, max, label, groupname);
		break;
	case kTypeBool:
		InitBool(name.c_str(), defaultValue, label, groupname);
		break;
	case kTypeNone: break;
	default: break;
	}
	
	mod(defaultValue, SET);
}

bool syn::UnitParameter::operator==(const UnitParameter& p) const
{
  return m_id == p.m_id && m_name == p.m_name && Type() == p.Type() &&\
	GetMin() == p.GetMin() && GetMax() == p.GetMax() && m_controller == p.m_controller;
}

void syn::UnitParameter::mod(double amt, MOD_ACTION action)
{
  if (action == SET)
  {
    m_offsetValue = amt;
    Set(amt);
	m_isDirty = true;
  }
  else if (action == ADD && amt != 0)
  {
    Set(Value() + amt);
	m_isDirty = true;
  }
  else if (action == SCALE && amt != 1)
  {
    Set(Value() * amt);
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
  if (m_controller == controller)
  {
    m_controller = nullptr;
  }
  else
  {
    throw std::logic_error("Only the UnitParameter's assigned controller can disconnect itself.");
  }
}