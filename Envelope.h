#ifndef __Envelope__
#define __Envelope__
#include "SourceUnit.h"
#include <vector>

#define MIN_ENV_PERIOD	.001
#define MIN_ENV_SHAPE	.0000001
namespace syn
{
  class Envelope;

  class EnvelopeSegment
  {
  protected:
    Envelope* m_parent;
    int m_period_id;
    int m_target_amp_id;
    int m_shape_id;
  public:

    EnvelopeSegment::EnvelopeSegment(Envelope* parent, int pid, int taid, int sid) :
      m_parent(parent),
      m_period_id(pid),
      m_target_amp_id(taid),
      m_shape_id(sid),
      prev_amp(0),
      step(0),
      is_increasing(false)
    {};
    EnvelopeSegment::EnvelopeSegment() :EnvelopeSegment(nullptr, 0, 0, 0)
    {};
    EnvelopeSegment::EnvelopeSegment(const EnvelopeSegment& other) :
      EnvelopeSegment(other.m_parent, other.m_period_id, other.m_target_amp_id, other.m_shape_id)
    {};
    EnvelopeSegment& operator=(const EnvelopeSegment& other);
    UnitParameter& period();
    UnitParameter& period() const;
    UnitParameter& target_amp();
    UnitParameter& target_amp() const;
    UnitParameter& shape();
    UnitParameter& shape() const;
    double prev_amp;
    double step;
    bool is_increasing;
  };

  class Envelope : public SourceUnit
  {
  public:
    Envelope(string name, int numSegments);
    Envelope(string name) : Envelope(name, 4) {};
    Envelope(const Envelope& env);
    virtual ~Envelope();

    void setFs(double fs);
    bool isActive() const { return !m_isDone; };
    void setSegment(int starting_segment); //!< Restarts the envelope beginning at the specified segment
    virtual void noteOn(int pitch, int vel);
    virtual void noteOff(int pitch, int vel);
    int getSamplesPerPeriod() const;
    int getNumSegments() { return m_numSegments; };
    void setPeriod(int seg, double period) { m_segments[seg]->period().mod(period, SET); };
    void setShape(int seg, double shape) { m_segments[seg]->shape().mod(shape, SET); };
    void setPoint(int seg, double target_amp) { m_segments[seg]->target_amp().mod(target_amp, SET); };
    int getPeriodId(int seg) { return m_segments[seg]->period().getId(); };
    int getShapeId(int seg) { return m_segments[seg]->shape().getId(); };
    int getPointId(int seg) { return m_segments[seg]->target_amp().getId(); };
    double getPeriod(int seg) { return m_segments[seg]->period(); };
    double getShape(int seg) { return m_segments[seg]->shape(); };
    double getPoint(int seg) { return m_segments[seg]->target_amp(); };
    double getInitPoint() { return m_initPoint; };
    void setInitPoint(double x) { m_initPoint = x; };

  protected:
    void	updateSegment(const int segment); //!< Updates the EnvelopeSegment to reflect the values in m_params 

    virtual void process();
  private:
    virtual Unit* cloneImpl() const { return new Envelope(*this); };
    vector<EnvelopeSegment*> m_segments;
    UnitParameter& m_loopStart;
    UnitParameter& m_loopEnd;
    double m_initPoint;
    int m_numSegments;
    int m_currSegment;
    bool	m_isDone;
    double m_RelPoint;
    double m_phase;
    bool m_fsIsDirty;
  };
}
#endif // __Envelope__