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
    Envelope(string name) : Envelope(name, 3) {};
    Envelope(const Envelope& env);
    virtual void setFs(const double fs);
    bool isActive() const { return !m_isDone; };
    virtual void noteOn(int pitch, int vel);
    virtual void noteOff(int pitch, int vel);
    int   getSamplesPerPeriod() const;
    int getNumSegments() { return m_numSegments; };
    void setPeriod(int seg, double period) { modifyParameter(seg * 3, period, SET); };
    void setShape(int seg, double shape) { modifyParameter(seg * 3 + 1, shape, SET); };
    void setPoint(int seg, double target_amp) { modifyParameter(seg * 3 + 2, target_amp, SET); };
    double getPeriod(int seg) { return readParam(seg * 3); };
    double getShape(int seg) { return readParam(seg * 3 + 1); };
    double getPoint(int seg) { return readParam(seg * 3 + 2); };
    double getInitPoint() { return m_initPoint; };
  protected:
    void	refreshPeriod(const int segment, double period, double shape = 0);
    void	refreshShape(int segment, double shape);
    void	refreshPoint(int segment, double target_amp);
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