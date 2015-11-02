#ifndef __Connection__
#define __Connection__
#include "UnitParameter.h"
#include "IPlugStructs.h"
#include <string>
#include <vector>
using std::string;
using std::vector;
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
  protected:
    vector<double> m_buf; //<! transfer buffer
    size_t m_maxSize;
    unsigned int m_bufReadInd,m_bufWriteInd;
  public:
    const int m_sourceid;
    const int m_targetid;
    const int m_targetport;
    const MOD_ACTION m_action;

    Connection(int sourceid, int targetid, int targetport, MOD_ACTION action, size_t maxsize=32) :
      m_sourceid(sourceid),
      m_targetid(targetid),
      m_targetport(targetport),
      m_action(action),
      m_buf(32),
      m_bufReadInd(0),
      m_bufWriteInd(0),
      m_maxSize(maxsize)
    {}
    // copy constructor
    Connection(const Connection& c) :
      m_sourceid(c.m_sourceid),
      m_targetid(c.m_targetid),
      m_targetport(c.m_targetport),
      m_action(c.m_action),
      m_buf(c.m_buf),
      m_maxSize(c.m_maxSize),
      m_bufReadInd(c.m_bufReadInd),
      m_bufWriteInd(c.m_bufWriteInd)
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
      m_targetid == c.m_targetid && m_action==c.m_action && m_bufWriteInd==c.m_bufWriteInd &&\
      m_bufReadInd==c.m_bufReadInd;
    }

    virtual double pull()
    {
      if (m_bufReadInd == m_bufWriteInd)
      {
        return 0.0;
      }
      double next = m_buf[m_bufReadInd];
      m_bufReadInd++;
      if (m_bufReadInd >= m_maxSize)
      {
        m_bufReadInd = 0;
      }
      return next;
    }

    virtual void push(double next)
    {
      m_buf[m_bufWriteInd] = next;
      m_bufWriteInd++;
      if (m_bufWriteInd >= m_maxSize)
      {
        m_bufWriteInd = 0;
      }
    }

    virtual void push(const double* next, size_t nsamples)
    {
      if (m_maxSize - m_bufWriteInd < nsamples)
      {
        m_maxSize = m_bufWriteInd + nsamples;
        m_buf.resize(nsamples + m_bufWriteInd);
      }
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

  /**
   * \brief Unlike its parent, MIDIConnection only allows a single message per sample, and so it has no queue.
   */
  class MIDIConnection :public Connection
  {    
    public:
    const IMidiMsg::EControlChangeMsg m_sourceid;
    MIDIConnection(IMidiMsg::EControlChangeMsg cc, int targetid, int targetport, MOD_ACTION action) :
      Connection(cc, targetid, targetport, action, 1),
      m_sourceid(cc)
    {      
      m_buf[0] = 1.0;
    }

    virtual double pull()
    {
      return m_buf[0];
    }

    virtual void push(double next)
    {
      m_buf[0] = next;
    }
  };
}
#endif // __Connection__