#include "Envelope.h"
/******************************
* Envelope methods
*
******************************/

Envelope::Envelope() :
  m_currSegment(0),
  m_isDone(true),
  m_isRepeating(false),
  m_lastRawOutput(0),
  m_initPoint(0),
  m_numSegments(3)
{
  m_segments = vector<EnvelopeSegment>(m_numSegments);
  // set up a standard ADSR envelope
  m_initPoint = 0.0;
  m_segments[0].target_amp = 1.0;
  m_segments[1].target_amp = 1.0;
  m_segments[2].target_amp = 0.0;
  for (int i = 0; i < m_numSegments; i++)
  {
    setPeriod(i, MIN_ENV_PERIOD, MIN_ENV_SHAPE);
  }
}

void Envelope::setPeriod(int segment, double period, double shape)
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
void Envelope::setShape(int segment, double shape)
{
  if (segment == -1)
  {
    for (int i = 0; i < m_numSegments; i++)
    {
      setPeriod(i, m_segments[i].period, shape);
    }
  }
  else
  {
    setPeriod(segment, m_segments[segment].period, shape);
  }
}

/*
* Sets the end point of the specified segment
*/
void Envelope::setPoint(int segment, double target_amp)
{
  m_segments[segment].target_amp = target_amp;
  if (segment < m_numSegments - 1)
  {
    setPeriod(segment + 1, m_segments[segment + 1].period, m_segments[segment + 1].shape);
  }
  setPeriod(segment, m_segments[segment].period, m_segments[segment].shape);
}

double Envelope::process(const double input)
{
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
        trigger();
      }
      if (m_currSegment == m_numSegments - 1)
      {
        m_isDone = true;
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
    setPeriod(i, m_segments[i].period, m_segments[i].shape);
  }
}

void Envelope::trigger()
{
  m_currSegment = 0;
  m_lastRawOutput = m_initPoint;
  m_isDone = false;
}

void Envelope::release()
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