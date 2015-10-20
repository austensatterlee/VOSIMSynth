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
    m_isRepeating(false),
    m_lastRawOutput(0),
    m_initPoint(0),
    m_numSegments(numSegments)
  {
    m_segments = vector<EnvelopeSegment>(m_numSegments);
    // set up a standard ADSR envelope
    m_initPoint = 0.0;
    ostringstream paramname;
    int i;
    for (i = 0; i < numSegments; i++)
    {
      paramname.str(""); paramname.clear();
      paramname << "period" << i;
      addParam(UnitParameter(paramname.str(), 0.5, MIN_ENV_PERIOD, 10.0, true));
      paramname.str(""); paramname.clear();
      paramname << "shape" << i;
      addParam(UnitParameter(paramname.str(), 0.5, MIN_ENV_SHAPE, 1.0, true));
      paramname.str(""); paramname.clear();
      paramname << "target" << i;
      if(i==numSegments-1)
        addParam(UnitParameter(paramname.str(), 0.0, 0.0, 0.0, true));
      else
        addParam(UnitParameter(paramname.str(), 1.0, 0.0, 0.0, true));        
    }
  }

  Envelope::Envelope(const Envelope& env) : Envelope(env.m_name, env.m_numSegments)
  {
    m_segments = env.m_segments;
    m_currSegment = env.m_currSegment;
    m_initPoint = env.m_initPoint;
    m_isDone = env.m_isDone;
    m_isRepeating = env.m_isRepeating;
    m_lastRawOutput = env.m_lastRawOutput;
  }

  void Envelope::refreshPeriod(int segment, double period, double shape)
  {
    /*
    * period - length of segment in seconds
    * shape - higher shape <=> more linear
    *
    */
    shape = shape == 0 ? m_segments[segment].shape : shape;
    m_segments[segment].period = period;
    m_segments[segment].shape = shape;

    period = period + MIN_ENV_PERIOD;
    shape = LERP(dbToAmp(-240), dbToAmp(0), shape) + MIN_ENV_SHAPE;

    m_segments[segment].mult = exp(-log((1 + shape) / shape) / (m_Fs*period));
    double prev_amp = segment > 0 ? m_segments[segment - 1].target_amp : m_initPoint;
    if (m_segments[segment].target_amp > prev_amp)
    {
      m_segments[segment].base = (m_segments[segment].target_amp + shape) * (1 - m_segments[segment].mult);
    }
    else
    {
      m_segments[segment].base = (m_segments[segment].target_amp - shape) * (1 - m_segments[segment].mult);
    }
  }

  /* Sets the shape of the specified segment. Sets the shape of all segments if first argument is -1 */
  void Envelope::refreshShape(int segment, double shape)
  {
    if (segment == -1)
    {
      for (int i = 0; i < m_numSegments; i++)
      {
        refreshPeriod(i, m_segments[i].period, shape);
      }
    }
    else
    {
      refreshPeriod(segment, m_segments[segment].period, shape);
    }
  }

  /*
  * Sets the end point of the specified segment
  */
  void Envelope::refreshPoint(int segment, double target_amp)
  {
    m_segments[segment].target_amp = target_amp;
    if (segment < m_numSegments - 1)
    {
      refreshPeriod(segment + 1, m_segments[segment + 1].period, m_segments[segment + 1].shape);
    }
    refreshPeriod(segment, m_segments[segment].period, m_segments[segment].shape);
  }

  double Envelope::process()
  {
    for (int i = 0; i < m_numSegments; i++)
    {
      int j=i*3;
      if (readParam(j) != m_segments[i].period || readParam(j + 1) != m_segments[i].shape || readParam(j + 2) != m_segments[i].target_amp)
      {
        m_segments[i].period = readParam(j);
        m_segments[i].shape = readParam(j + 1);
        m_segments[i].target_amp = readParam(j + 2);
        refreshPeriod(i,m_segments[i].period,m_segments[i].shape);
      }
    }
    double prev_amp = m_currSegment > 0 ? m_segments[m_currSegment - 1].target_amp : m_initPoint;
    bool isIncreasing = m_segments[m_currSegment].target_amp > prev_amp;
    double output;
    if ((isIncreasing && m_lastRawOutput >= m_segments[m_currSegment].target_amp) || (!isIncreasing && m_lastRawOutput <= m_segments[m_currSegment].target_amp))
    {
      output = m_segments[m_currSegment].target_amp;
      if (m_currSegment < m_numSegments - 2)
      {
        m_currSegment++;
      }
      else
      {
        if (m_isRepeating && m_currSegment == m_numSegments - 2)
        {
          noteOn(0, 0);
        }
        if (m_currSegment == m_numSegments - 1)
        {
          m_isDone = true;
          m_extSyncPort.Emit();
        }
      }
    }
    else
    {
      output = m_segments[m_currSegment].base + m_segments[m_currSegment].mult * m_lastRawOutput;
    }
    m_lastRawOutput = output;
    return output;
  }

  void Envelope::setFs(const double fs)
  {
    m_Fs = fs;
    for (int i = 0; i < m_numSegments; i++)
    {
      refreshPeriod(i, m_segments[i].period, m_segments[i].shape);
    }
  }

  void Envelope::noteOn(int pitch, int vel)
  {
    m_currSegment = 0;
    m_lastRawOutput = m_initPoint;
    m_isDone = false;
  }

  void Envelope::noteOff(int pitch, int vel)
  {
    m_RelPoint = m_lastRawOutput;
    m_currSegment = m_numSegments - 1;
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