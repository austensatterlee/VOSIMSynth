#ifndef __Envelope__
#define __Envelope__
#include "SourceUnit.h"
#include <vector>

#define MIN_ENV_PERIOD	0.0001
#define MIN_ENV_AMP 0.01

namespace syn
{
	class Envelope;

	class EnvelopeSegment
	{
	protected:
		Envelope* m_parent;
		int m_period_id;
		int m_target_amp_id;
	public:

		EnvelopeSegment::EnvelopeSegment(Envelope* parent, int pid, int taid) :
			m_parent(parent),
			m_period_id(pid),
			m_target_amp_id(taid),
			prev_amp(0),
			step(0),
			is_increasing(false) { };

		EnvelopeSegment::EnvelopeSegment() : EnvelopeSegment(nullptr, 0, 0) { };

		EnvelopeSegment::EnvelopeSegment(const EnvelopeSegment& other) :
			EnvelopeSegment(other.m_parent, other.m_period_id, other.m_target_amp_id) { };

		EnvelopeSegment& operator=(const EnvelopeSegment& other);
		UnitParameter& target_amp() const;
		UnitParameter& period() const;
		double prev_amp;
		double step;
		bool is_increasing;
	};

	class Envelope : public SourceUnit
	{
	public:
		Envelope(string name, int numSegments);

		Envelope(string name) : Envelope(name, 4) { };

		void noteOn(int pitch, int vel) override;
		void noteOff(int pitch, int vel) override;

		Envelope(const Envelope& env);
		virtual ~Envelope();

		void onParamChange(const UnitParameter* param) override;

		bool isActive() const override
		{
			return !m_isDone;
		};

		int getSamplesPerPeriod() const override;

		int getNumSegments() const
		{
			return m_numSegments;
		};

		void setPeriod(int seg, double period)
		{
			m_segments[seg]->period().mod(period, SET);
		};

		UnitParameter& getPeriod(int seg)
		{
			return m_segments[seg]->period();
		};

		void setTarget(int seg, double target_amp)
		{
			m_segments[seg]->target_amp().mod(target_amp, SET);
		};

		UnitParameter& getTarget(int seg)
		{
			return m_segments[seg]->target_amp();
		};

		double getInitTarget() const
		{
			return m_initPoint;
		};

		void setInitTarget(double x)
		{
			m_initPoint = x;
		};

	protected:
		void updateSegment(const int segment); //!< Updates the EnvelopeSegment to reflect the values in m_params

		void process(int bufind) override;
		void setSegment(int seg);
		void onSampleRateChange(double newfs) override;
	private:
		Unit* cloneImpl() const override
		{
			return new Envelope(*this);
		}

		string getClassName() const override
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
	};
}
#endif // __Envelope__


