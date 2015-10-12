#ifndef __Envelope__
#define __Envelope__
#include "SourceUnit.h"
#include <vector>

#define MIN_ENV_PERIOD	.0001
#define MIN_ENV_SHAPE	.000000000001

struct EnvelopeSegment
{
  double period;
  double target_amp;
  double shape;
  double base;
  double mult;
};

class Envelope : public SourceUnit
{
public:
  Envelope();
  virtual void setFs(const double fs);
  const bool isDone() { return m_isDone; };
  void	setPeriod(const int segment, double period, double shape = 0);
  void	setShape(int segment, double shape);
  void	setPoint(int segment, double target_amp);
  virtual void	trigger();
  virtual void	release();
  int   getSamplesPerPeriod() const;
protected:
  double process(const double input);
private:
  vector<EnvelopeSegment> m_segments;
  double m_initPoint;
  int m_numSegments;
  int m_currSegment;
  bool m_isRepeating;
  bool	m_isDone;
  double m_RelPoint;
  double m_lastRawOutput;
};
#endif // __Envelope__