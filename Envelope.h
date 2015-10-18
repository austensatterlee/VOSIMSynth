#ifndef __Envelope__
#define __Envelope__
#include "SourceUnit.h"
#include <vector>

#define MIN_ENV_PERIOD	.01
#define MIN_ENV_SHAPE	.0000001
namespace syn
{
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
    Envelope(string name, int numSegments);
    Envelope(string name) : Envelope(name,3) {};
    Envelope(const Envelope& env);
    virtual void setFs(const double fs);
    bool isActive() const { return !m_isDone; };
    void	setPeriod(const int segment, double period, double shape = 0);
    void	setShape(int segment, double shape);
    void	setPoint(int segment, double target_amp);
    virtual void noteOn(int pitch, int vel);
    virtual void noteOff(int pitch, int vel);
    int   getSamplesPerPeriod() const;
  protected:
    virtual double process();
  private:
    virtual Unit* cloneImpl() const { return new Envelope(*this); };
    vector<EnvelopeSegment> m_segments;
    double m_initPoint;
    int m_numSegments;
    int m_currSegment;
    bool m_isRepeating;
    bool	m_isDone;
    double m_RelPoint;
    double m_lastRawOutput;
  };
}
#endif // __Envelope__