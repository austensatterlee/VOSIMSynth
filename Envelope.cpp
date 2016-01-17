#include "Envelope.h"
#include <sstream>
#include "tables.h"
using std::ostringstream;

/******************************
* Envelope methods
*
******************************/
namespace syn
{
	Envelope::Envelope(string name, int numSegments) :
		SourceUnit(name),
		m_loopStart(addParam("loopstart", IParam::kTypeInt, 0, numSegments, 0)),
		m_loopEnd(addParam("loopend", IParam::kTypeInt, 0, numSegments, 0)),
		m_initPoint(0),
		m_numSegments(numSegments),
		m_currSegment(0),
		m_isDone(true),
		m_phase(0)
	{
		// set up a standard ADSR envelope with intermediate segments
		ostringstream paramname;
		int i;
		int period, target;
		for (i = 0; i < numSegments; i++)
		{
			paramname.str("");
			paramname.clear();
			paramname << "period" << i;
			period = addParam(paramname.str(), IParam::kTypeDouble, MIN_ENV_PERIOD, 2.0, MIN_ENV_PERIOD).getId();
			getParam(period).mod(MIN_ENV_PERIOD, SET);
			paramname.str("");
			paramname.clear();
			paramname << "target" << i;
			if (i == numSegments - 1)
			{
				target = addParam(paramname.str(), IParam::kTypeDouble, 0.0, 1.0, 0).getId();
			}
			else
			{
				target = addParam(paramname.str(), IParam::kTypeDouble, 0.0, 1.0, 1).getId();
			}

			m_segments.push_back(new EnvelopeSegment(this, period, target));
			updateSegment(i);
		}
	}

	Envelope::Envelope(const Envelope& env) : Envelope(env.m_name, env.m_numSegments)
	{
		m_currSegment = env.m_currSegment;
		m_initPoint = env.m_initPoint;
		m_isDone = env.m_isDone;
		m_phase = env.m_phase;
	}

	Envelope::~Envelope()
	{
		for (int i = 0; i < m_segments.size(); i++)
		{
			delete m_segments[i];
		}
	}

	void Envelope::updateSegment(int segment)
	{
		if (m_segments[segment]->period().isDirty() || m_fsIsDirty)
		{
			m_segments[segment]->step = 1. / (m_Fs * (m_segments[segment]->period()));
			m_segments[segment]->period().setClean();
		}
		if (m_phase == 0)
		{
			if (segment == 0)
			{
				m_segments[segment]->prev_amp = m_initPoint;
			}
			else if (segment == m_numSegments - 1)
			{
				m_segments[segment]->prev_amp = getLastOutput();
			}
			else
			{
				m_segments[segment]->prev_amp = getTarget(segment - 1);
			}
		}
		m_segments[segment]->is_increasing = m_segments[segment]->target_amp() > m_segments[segment]->prev_amp;
	}

	void Envelope::process(int bufind)
	{
		updateSegment(m_currSegment);
		EnvelopeSegment& currseg = *m_segments[m_currSegment];
		bool hasHitSetpoint = (currseg.is_increasing && getLastOutput() >= currseg.target_amp()) ||
			(!currseg.is_increasing && getLastOutput() <= currseg.target_amp());
		// Advance to a new segment if our time is up or if we're released and we have fully decayed
		if (m_phase >= 1 || (m_currSegment == m_numSegments - 1 && hasHitSetpoint))
		{
			if (m_currSegment + 1 == int(readParam(1)))
			{ // check if we have reached a loop point
				setSegment(int(readParam(0)));
				m_isSynced = true;
			}
			else if (m_currSegment < m_numSegments - 2)
			{ // advance if we have not reached the sustain point
				setSegment(m_currSegment + 1);
			}
			else if (m_currSegment == m_numSegments - 1)
			{ // check if we have fully decayed
				m_isDone = true;
			}

			if(m_currSegment == m_numSegments-2)
			{
				m_isSynced = true;
			}
		}
		else
		{
			m_phase += m_segments[m_currSegment]->step;
		}
		const EnvelopeSegment& currSeg = *m_segments[m_currSegment];
		double MIN_ENV_DB = AmpToDB(MIN_ENV_AMP);
		double prevdb = LERP(MIN_ENV_DB,0,currSeg.prev_amp);
		double targetdb = LERP(MIN_ENV_DB, 0, currSeg.target_amp());
		double output = DBToAmp(LERP(prevdb,targetdb, m_phase));
		
		m_output[bufind] = output;
	}

	void Envelope::setSegment(int seg)
	{
		if (seg < 0)
			seg = 0;
		m_currSegment = seg;
		m_isDone = false;
		m_phase = 0.0;
	}

	void Envelope::onSampleRateChange(double newfs)
	{
		m_Fs = newfs;
		m_fsIsDirty = true;
		for (int i = 0; i < m_numSegments; i++)
		{
			updateSegment(i);
		}
		m_fsIsDirty = false;
	}

	void Envelope::onBufferSizeChange(size_t newbuffersize)
	{
	}

	void Envelope::onTempoChange(double newtempo)
	{
	}

	void Envelope::noteOn(int pitch, int vel)
	{
		setSegment(0);
	}

	void Envelope::noteOff(int pitch, int vel)
	{
		setSegment(m_numSegments - 1);
	}

	int Envelope::getSamplesPerPeriod() const
	{
		double approx = 0;
		for (int i = 0; i < m_numSegments - 1; i++)
		{
			approx += m_segments[i]->period() * m_Fs;
		}
		return int(approx);
	}

	EnvelopeSegment& EnvelopeSegment::operator=(const EnvelopeSegment& other)
	{
		if (this != &other)
		{
			*this = other;
		}
		return *this;
	}

	UnitParameter& EnvelopeSegment::period() const
	{
		if (m_parent)
		{
			return m_parent->getParam(m_period_id);
		}
		throw;
	}

	UnitParameter& EnvelopeSegment::target_amp() const
	{
		if (m_parent)
		{
			return m_parent->getParam(m_target_amp_id);
		}
		throw;
	}
}

