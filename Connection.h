#ifndef __Connection__
#define __Connection__
#include "UnitParameter.h"
#include "IPlugStructs.h"
#include <string>
#include <deque>
using std::string;
using std::deque;
namespace syn
{
  /*!
   * \class Connection
   *
   * \brief A connection targeted at a UnitParameter of some Unit inside a Circuit.
   * 
   * push() data from the source into the queue, pull() data from queue into the target.
   *
   */
  class Connection
  {
  public:
    deque<double> m_buf; //<! transfer buffer
    const int m_sourceid;
    const int m_targetid;
    const int m_targetport;
    const MOD_ACTION m_action;

    Connection(int sourceid, int targetid, int targetport, MOD_ACTION action) :
      m_sourceid(sourceid),
      m_targetid(targetid),
      m_targetport(targetport),
      m_action(action),
      m_buf(0)
    {}
    // copy constructor
    Connection(const Connection& c) :
      m_sourceid(c.m_sourceid),
      m_targetid(c.m_targetid),
      m_targetport(c.m_targetport),
      m_action(c.m_action),
      m_buf(c.m_buf)
    {}
    // copy assignment
    Connection& operator=(const Connection& c)
    {
      if (this == &c)
        return *this;
      Connection tmp(c);
      *this = tmp;
      return *this;
    }
    bool operator==(const Connection& c)
    {
      return m_sourceid==c.m_sourceid && m_targetport == c.m_targetport && \
      m_targetid == c.m_targetid && m_action==c.m_action;
    }

    virtual double pull()
    {
      double next = m_buf.back();
      m_buf.pop_back();
      return next;
    }

    virtual void push(double next)
    {
      m_buf.push_front(next);
    }

    virtual void push(const double* next, size_t nsamples)
    {
      while (nsamples--)
      {
        push(*next);
        next++;
      }
    }

    bool isEmpty () const
    {
      return m_buf.size()==0;
    }
  };

  class MIDIConnection :public Connection
  {    
    public:
    const IMidiMsg::EControlChangeMsg m_sourceid;
    MIDIConnection(IMidiMsg::EControlChangeMsg cc, int targetid, int targetport, MOD_ACTION action) :
      Connection(cc, targetid, targetport, action),
      m_sourceid(cc)
    {      
      m_buf.push_back(0.0);
    }

    virtual double pull()
    {
      double next = m_buf.back();
      return next;
    }

    virtual void push(double next)
    {
      m_buf.clear();
      m_buf.push_front(next);
    }
  };
}
#endif // __Connection__