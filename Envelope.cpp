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
    m_phase(0)
  {
    m_segments = vector<EnvelopeSegment>(m_numSegments);
    // set up a standard ADSR envelope
    ostringstream paramname;
    int i;
    addParam(UnitParameter("loopstart", -1, -1, m_numSegments - 1, INT_TYPE));
    addParam(UnitParameter("loopend", -1, -1, m_numSegments - 1, INT_TYPE));
    for (i = 0; i < numSegments; i++)
    {
      paramname.str(""); paramname.clear();
      paramname << "period" << i;
      addParam(UnitParameter(paramname.str(), 0.5, MIN_ENV_PERIOD, 10.0, DOUBLE_TYPE));
      paramname.str(""); paramname.clear();
      paramname << "shape" << i;
      addParam(UnitParameter(paramname.str(), 1.0, MIN_ENV_SHAPE, 1.0, DOUBLE_TYPE));
      paramname.str(""); paramname.clear();
      paramname << "target" << i;
      if (i == numSegments - 1)
        addParam(UnitParameter(paramname.str(), 0.0, 0.0, 0.0, DOUBLE_TYPE));
      else
        addParam(UnitParameter(paramname.str(), 1.0, 0.0, 0.0, DOUBLE_TYPE));
    }
  }

  Envelope::Envelope(const Envelope& env) : Envelope(env.m_name, env.m_numSegments)
  {
    m_segments = env.m_segments;
    m_currSegment = env.m_currSegment;
    m_initPoint = env.m_initPoint;
    m_isDone = env.m_isDone;
    m_phase = m_phase;
  }

  void Envelope::updateSegment(int segment)
  {
    m_segments[segment].shape = getShape(segment);
    m_segments[segment].target_amp = getPoint(segment);
    if(m_segments[segment].period!=getPeriod(segment)){
      m_segments[segment].period = getPeriod(segment);
      m_segments[segment].step = 1. / (m_Fs*m_segments[segment].period);
    }
    if(m_phase==0){
      if (segment == 0)
      {
        m_segments[segment].prev_amp = m_initPoint;
      }
      else if (segment == m_numSegments - 1)
      {
        m_segments[segment].prev_amp = m_lastOutput;
      }
      else
      {
        m_segments[segment].prev_amp = getPoint(segment - 1);
      }
    }
    m_segments[segment].is_increasing = m_segments[segment].target_amp > m_segments[segment].prev_amp;
  }

  double Envelope::process()
  {
    updateSegment(m_currSegment);
    const EnvelopeSegment& currseg = m_segments[m_currSegment];
    if (m_phase >= 1 || \
        (currseg.is_increasing && m_lastOutput>=currseg.target_amp) || \
        (!currseg.is_increasing && m_lastOutput<=currseg.target_amp))
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
      m_phase += m_segments[m_currSegment].step;
    }
    double output = LERP(m_segments[m_currSegment].prev_amp, m_segments[m_currSegment].target_amp, m_phase);
    if (isinf(output))
    {
      output=0;
    }
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
      approx += m_segments[i].period*m_Fs;
    }
    return (int)approx;
  }
}