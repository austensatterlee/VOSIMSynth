#ifndef __Connection__
#define __Connection__
#include <string>
#include "Parameter.h"
using std::string;
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
  bool operator==(const Connection& mm)
  {
    return (m_targetname == mm.m_targetname && m_paramname == mm.m_paramname && m_action == mm.m_action);
  }
};

#endif // __Connection__