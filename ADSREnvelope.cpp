#include "ADSREnvelope.h"


namespace syn
{
	void ADSREnvelope::onSampleRateChange(double newfs)
	{
	}

	void ADSREnvelope::process(int bufind)
	{
		/* Determine phase increment based on current segment */
		double phaseIncrement = 0.0;
		switch (m_currStage)
		{
		case ATTACK:
			phaseIncrement = m_attack;
			break;
		case DECAY:
			phaseIncrement = m_decay;
			break;
		case SUSTAIN:
			phaseIncrement = 0;
			break;
		case RELEASE:
			phaseIncrement = m_release;
			break;
		default:
			throw std::logic_error("Invalid envelope stage");
		}
		m_phase += phaseIncrement;

		/* Handle segment change */
		if (m_phase >= 1.0)
		{
			if (m_currStage == ATTACK)
			{
				m_currStage = DECAY;
			}
			else if (m_currStage == DECAY)
			{
				m_currStage = SUSTAIN;
			}
			else if (m_currStage == RELEASE)
			{
				m_isActive = false;
			}
		}
	}

	void ADSREnvelope::noteOn(int pitch, int vel)
	{
		m_isActive = true;
		m_currStage = ATTACK;
		m_phase = 0;
	}

	void ADSREnvelope::noteOff(int pitch, int vel)
	{
		m_currStage = RELEASE;
	}

	int ADSREnvelope::getSamplesPerPeriod() const
	{
		double approx = m_attack + m_decay + m_release;
		approx *= m_Fs;
		return int(approx);
	}

	bool ADSREnvelope::isActive() const
	{
		return m_isActive;
	}

	ADSREnvelope::ADSREnvelope(string name) :
		SourceUnit(name),
		m_attack(addDoubleParam("attack", 0.001, 1.0, 0.001, 1e-3)),
		m_decay(addDoubleParam("decay", 0.001, 1.0, 0.001, 1e-3)),
		m_sustain(addDoubleParam("sustain", 0.0, 1.0, 1.0, 1e-3, 2)),
		m_release(addDoubleParam("release", 0.001, 1.0, 0.001, 1e-3)),
		m_isActive(false)
	{
	}

	ADSREnvelope::ADSREnvelope(const ADSREnvelope& other) :
		ADSREnvelope(other.m_name)
	{
		m_phase = other.m_phase;
		m_currStage = other.m_currStage;
		m_isActive = other.m_isActive;
	}

	ADSREnvelope::~ADSREnvelope()
	{
	}
}

