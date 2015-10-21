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
    double prev_amp;
    double step;
    bool is_increasing;
  };

  class Envelope : public SourceUnit
  {
  public:
    Envelope(string name, int numSegments);
    Envelope(string name) : Envelope(name, 3) {};
    Envelope(const Envelope& env);
    bool isActive() const { return !m_isDone; };
    void setSegment(int starting_segment); //!< Restarts the envelope beginning at the specified segment
    virtual void noteOn(int pitch, int vel);
    virtual void noteOff(int pitch, int vel);
    int getSamplesPerPeriod() const;
    int getNumSegments() { return m_numSegments; };
    void setPeriod(int seg, double period) { modifyParameter(getPeriodId(seg), period, SET); };
    void setShape(int seg, double shape) { modifyParameter(getShapeId(seg), shape, SET); };
    void setPoint(int seg, double target_amp) { modifyParameter(getPointId(seg), target_amp, SET); };
    int getPeriodId(int seg) { return seg * 3 + 2; };
    int getShapeId(int seg) { return seg * 3 + 3; };
    int getPointId(int seg) { return seg*3 + 4; };
    double getPeriod(int seg) { return readParam(getPeriodId(seg)); };
    double getShape(int seg) { return readParam(getShapeId(seg)); };
    double getPoint(int seg) { return readParam(getPointId(seg)); };
    double getInitPoint() { return m_initPoint; };
  protected:
    void	updateSegment(const int segment); //!< Updates the EnvelopeSegment to reflect the values in m_params 
    virtual double process();
  private:
    virtual Unit* cloneImpl() const { return new Envelope(*this); };
    vector<EnvelopeSegment> m_segments;
    double m_initPoint;
    int m_numSegments;
    int m_currSegment;
    bool	m_isDone;
    double m_RelPoint;
    double m_phase;
  };
}
#endif // __Envelope__