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
   */
  struct Connection
  {
    int m_sourceid;
    int m_targetid;
    int m_targetport;
    MOD_ACTION m_action;

    Connection(int sourceid, int targetid, int targetport, MOD_ACTION action) :
      m_sourceid(sourceid),
      m_targetid(targetid),
      m_targetport(targetport),
      m_action(action)
    {}
    Connection(const Connection& c)
    {
      m_sourceid = c.m_sourceid;
      m_targetid = c.m_targetid;
      m_targetport = c.m_targetport;
      m_action = c.m_action;
    }
    bool operator==(const Connection& c)
    {
      return c.m_sourceid==m_sourceid && c.m_targetid==m_targetid && c.m_targetport == m_targetport && c.m_action==m_action;
    }
  };

  struct MIDIConnection :public Connection
  {
    IMidiMsg::EControlChangeMsg m_sourceid;
    MIDIConnection(IMidiMsg::EControlChangeMsg cc, int targetid, int targetport, MOD_ACTION action) :
      Connection(cc,targetid,targetport,action),
      m_sourceid(cc)
    {}
  };
}
#endif // __Connection__