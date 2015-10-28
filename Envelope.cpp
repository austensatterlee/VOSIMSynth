#include "Envelope.h"
#include <sstream>
using std::ostringstream;
/******************************
* Envelope methods
*
******************************/
namespace syn
{
  Envelope::Envelope(string name, int numSegments) : SourceUnit(name),
    m_currSegment(0),
    m_isDone(true),
    m_initPoint(0),
    m_numSegments(numSegments),
    m_phase(0),
    m_loopStart(addParam("loopstart",INT_TYPE, 0, numSegments)),
    m_loopEnd(addParam("loopend",INT_TYPE, 0, numSegments))
  {
    // set up a standard ADSR envelope
    ostringstream paramname;
    int i;
    int period,shape,target;
    for (i = 0; i < numSegments; i++)
    {
      paramname.str(""); paramname.clear();
      paramname << "period" << i;
      period = addParam(paramname.str(), DOUBLE_TYPE, MIN_ENV_PERIOD, 10.0).getId();
      getParam(period).mod(0.1,SET);
      paramname.str(""); paramname.clear();
      paramname << "target" << i;
      target = addParam(paramname.str(), DOUBLE_TYPE, -1.0, 1.0).getId();
      if (i == 0 || i == numSegments - 1)
      {
        getParam(target).mod(0.0, SET);
      }
      else
      {
        getParam(target).mod(1.0, SET);
      }
      
      paramname.str(""); paramname.clear();
      paramname << "shape" << i;
      shape = addParam(paramname.str(), DOUBLE_TYPE, MIN_ENV_SHAPE, 1.0).getId();
      m_segments.push_back(new EnvelopeSegment(this, period,target,shape));
      updateSegment(i);
    }
  }

  Envelope::Envelope(const Envelope& env) : Envelope(env.m_name, env.m_numSegments)
  {
    m_currSegment = env.m_currSegment;
    m_initPoint = env.m_initPoint;
    m_isDone = env.m_isDone;
    m_phase = env.m_phase;
  }

  Envelope::~Envelope()
  {
    for (int i = 0; i < m_segments.size(); i++)
    {
      delete m_segments[i];
    }
  }

  void Envelope::setFs(double fs)
  {
    m_Fs = fs;
    for (int i = 0; i < m_numSegments; i++)
    {
      updateSegment(i);
    }
  }

  void Envelope::updateSegment(int segment)
  {
    m_segments[segment]->step = 1. / (m_Fs*(m_segments[segment]->period()));
    if(m_phase==0){
      if (segment == 0)
      {
        m_segments[segment]->prev_amp = m_initPoint;
      }
      else if (segment == m_numSegments - 1)
      {
        m_segments[segment]->prev_amp = getLastOutput();
      }
      else
      {
        m_segments[segment]->prev_amp = getPoint(segment - 1);
      }
    }
    m_segments[segment]->is_increasing = m_segments[segment]->target_amp() > m_segments[segment]->prev_amp;
  }

  double Envelope::process()
  {
    updateSegment(m_currSegment);
    EnvelopeSegment& currseg = *m_segments[m_currSegment];
    bool hasHitSetpoint = (currseg.is_increasing && getLastOutput() >= currseg.target_amp()) || \
                          (!currseg.is_increasing && getLastOutput() <= currseg.target_amp());
    // Advance to a new segment if our time is up or if we're released and we have fully decayed 
    if (m_phase >= 1 || (m_currSegment==m_numSegments-1 && hasHitSetpoint))
    {
      if (m_currSegment + 1 == (int)readParam(1))
      { // check if we have reached a loop point
        setSegment((int)readParam(0));
        m_extSyncPort.Emit();
      }
      else if (m_currSegment < m_numSegments - 2)
      { // check if we have reached the sustain point
        setSegment(m_currSegment+1);
      }
      else if (m_currSegment == m_numSegments - 1)
      { // check if we have fully decayed
        m_isDone = true;
        m_extSyncPort.Emit();
      }
    }
    else
    {
      m_phase += m_segments[m_currSegment]->step;
    }
    double output = LERP(m_segments[m_currSegment]->prev_amp, m_segments[m_currSegment]->target_amp(), m_phase);
    return output;
  }

  void Envelope::setSegment(int seg)
  {
    if (seg < 0)
      seg = 0;
    m_currSegment = seg;
    m_isDone = false;
    m_phase = 0.0;
  }

  void Envelope::noteOn(int pitch, int vel)
  {
    setSegment(0);
  }

  void Envelope::noteOff(int pitch, int vel)
  {
    setSegment(m_numSegments-1);
  }

  int Envelope::getSamplesPerPeriod() const
  {
    double approx = 0;
    for (int i = 0; i < m_numSegments; i++)
    {
      approx += m_segments[i]->period()*m_Fs;
    }
    return (int)approx;
  }

  EnvelopeSegment& EnvelopeSegment::operator=(const EnvelopeSegment& other)
  {
    if (this != &other)
    {
      *this = other;
    }
    return *this;
  }

  EnvelopeSegment::EnvelopeSegment(const EnvelopeSegment& other) :
    EnvelopeSegment(other.m_parent, other.m_period, other.m_target_amp, other.m_shape)
  {

  }

  EnvelopeSegment::EnvelopeSegment() :EnvelopeSegment(nullptr, 0, 0, 0)
  {

  }

  EnvelopeSegment::EnvelopeSegment(Envelope* parent, int pid, int taid, int sid) :
    m_parent(parent),
    m_period(pid),
    m_target_amp(taid),
    m_shape(sid),
    prev_amp(0),
    step(0),
    is_increasing(false)
  {

  }

  UnitParameter& EnvelopeSegment::period()
  {
    if (m_parent)
    {
      return m_parent->getParam(m_period);
    }
  }

  UnitParameter& EnvelopeSegment::period() const
  {
    if (m_parent)
    {
      return m_parent->getParam(m_period);
    }
  }

  UnitParameter& EnvelopeSegment::target_amp()
  {
    if (m_parent)
    {
      return m_parent->getParam(m_target_amp);
    }
  }

  UnitParameter& EnvelopeSegment::target_amp() const
  {
    if (m_parent)
    {
      return m_parent->getParam(m_target_amp);
    }
  }

  UnitParameter& EnvelopeSegment::shape()
  {
    if (m_parent)
    {
      return m_parent->getParam(m_shape);
    }
  }

  UnitParameter& EnvelopeSegment::shape() const
  {
    if (m_parent)
    {
      return m_parent->getParam(m_shape);
    }
  }

}