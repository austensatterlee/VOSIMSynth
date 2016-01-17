#ifndef __Envelope__
#define __Envelope__
#include "SourceUnit.h"
#include <vector>

#define MIN_ENV_PERIOD	0.0001
#define MIN_ENV_SHAPE	0.001

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

    EnvelopeSegment(Envelope* parent, int pid, int taid, int sid) :
      m_parent(parent),
      m_period_id(pid),
      m_target_amp_id(taid),
      m_shape_id(sid),
      prev_amp(0),
      step(0),
      is_increasing(false)
    {
    };

    EnvelopeSegment() : EnvelopeSegment(nullptr, 0, 0, 0)
    {
    };

    EnvelopeSegment(const EnvelopeSegment& other) :
      EnvelopeSegment(other.m_parent, other.m_period_id, other.m_target_amp_id, other.m_shape_id)
    {
    };

    EnvelopeSegment& operator=(const EnvelopeSegment& other);
    UnitParameter& period() const;
    UnitParameter& target_amp() const;
    UnitParameter& shape() const;
    double prev_amp;
    double step;
    bool is_increasing;
  };

  class Envelope : public SourceUnit
  {
  public:
    virtual void noteOn(int pitch, int vel) override;
    virtual void noteOff(int pitch, int vel) override;
    Envelope(string name, int numSegments);

    explicit Envelope(string name) : Envelope(name, 4)
    {
    };

    Envelope(const Envelope& env);
    virtual ~Envelope();

    virtual void setFs(double fs) override;

    virtual bool isActive() const override
    {
      return !m_isDone;
    };

    virtual int getSamplesPerPeriod() const override;

    int getNumSegments() const
    {
      return m_numSegments;
    };

    void setPeriod(int seg, double period)
    {
      m_segments[seg]->period().mod(period, SET);
    };

    void setShape(int seg, double shape)
    {
      m_segments[seg]->shape().mod(shape, SET);
    };

    void setPoint(int seg, double target_amp)
    {
      m_segments[seg]->target_amp().mod(target_amp, SET);
    };

    int getPeriodId(int seg)
    {
      return m_segments[seg]->period().getId();
    };

    int getShapeId(int seg)
    {
      return m_segments[seg]->shape().getId();
    };

    int getPointId(int seg)
    {
      return m_segments[seg]->target_amp().getId();
    };

    double getPeriod(int seg)
    {
      return m_segments[seg]->period();
    };

    double getShape(int seg)
    {
      return m_segments[seg]->shape();
    };

    double getPos(int seg)
    {
      return m_segments[seg]->target_amp();
    };

    double getInitPoint() const
    {
      return m_initPoint;
    };

    void setInitPoint(double x)
    {
      m_initPoint = x;
    };

  protected:
    void updateSegment(const int segment); //!< Updates the EnvelopeSegment to reflect the values in m_params

    virtual void process(int bufind) override;
    void setSegment(int seg);
  private:
    virtual Unit* cloneImpl() const override
    {
      return new Envelope(*this);
    }

    virtual string getClassName() const override
    {
      return "Envelope";
    }

    vector<EnvelopeSegment*> m_segments;
    UnitParameter& m_loopStart;
    UnitParameter& m_loopEnd;
    double m_initPoint;
    int m_numSegments;
    int m_currSegment;
    bool m_isDone;
    double m_RelPoint;
    double m_phase;
    bool m_fsIsDirty;
  };
}
#endif // __Envelope__


