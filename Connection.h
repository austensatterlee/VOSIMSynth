#ifndef __Connection__
#define __Connection__
#include <string>
#include "UnitParameter.h"
#include "IPlugStructs.h"
using std::string;
namespace syn
{
  /*!
   * \class Connection
   *
   * \brief Represents a connection between the output of one Unit and a parameter of another Unit.
   *
   * \author austen
   * \date October 2015
   */
  struct Connection
  {
    string m_sourcename;
    string m_targetname;
    string m_paramname;
    MOD_ACTION m_action;
    Connection(string sourcename, string targetname, string targetpname, MOD_ACTION action)
    {
      m_sourcename = sourcename;
      m_targetname = targetname;
      m_paramname = targetpname;
      m_action = action;
    }
    Connection* clone()
    {
      return new Connection(m_sourcename,m_targetname,m_paramname,m_action);
    }
    bool operator==(const Connection& mm)
    {
      return (m_targetname == mm.m_targetname && m_paramname == mm.m_paramname && m_action == mm.m_action);
    }
  };

  struct MIDIConnection
  {
    IMidiMsg::EControlChangeMsg m_ccMessage;
    string m_targetname;
    string m_paramname;
    MOD_ACTION m_action;
    MIDIConnection(IMidiMsg::EControlChangeMsg cc, string targetname, string targetpname, MOD_ACTION action)
    {
      m_ccMessage = cc;
      m_targetname = targetname;
      m_paramname = targetpname;
      m_action = action;
    }
    bool operator==(const MIDIConnection& mm)
    {
      return (m_targetname == mm.m_targetname && m_paramname == mm.m_paramname && m_action == mm.m_action);
    }
    MIDIConnection* clone()
    {
      return new MIDIConnection(m_ccMessage, m_targetname, m_paramname, m_action);
    }
  };
}
#endif // __Connection__