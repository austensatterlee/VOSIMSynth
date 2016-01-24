#include "ADSREnvelope.h"
#include "DSPMath.h"


namespace syn
{
	void ADSREnvelope::onSampleRateChange(double newfs) { }

	void ADSREnvelope::process(int bufind) {
		/* Determine phase increment based on current segment */
		double phaseIncrement;
		double initial;
		double target;
		switch (m_currStage) {
		case ATTACK:
			initial = 0.0;
			target = 1.0;
			phaseIncrement = m_attack;
			break;
		case DECAY:
			initial = 1.0;
			target = m_sustain;
			phaseIncrement = m_decay;
			break;
		case SUSTAIN:
			initial = m_sustain;
			target = m_sustain;
			phaseIncrement = 0;
			break;
		case RELEASE:
			initial = m_sustain;
			target = 0.0;
			phaseIncrement = m_release;
			break;
		default:
			throw std::logic_error("Invalid envelope stage");
		}
		m_phase += 1./(phaseIncrement*m_Fs);

		/* Handle segment change */
		if (m_phase >= 1.0) {
			if (m_currStage == ATTACK) {
				m_currStage = DECAY;
			} else if (m_currStage == DECAY) {
				m_currStage = SUSTAIN;
			} else if (m_currStage == SUSTAIN) {
				m_currStage = SUSTAIN;
				m_phase = 1.0;
			} else if (m_currStage == RELEASE) {
				m_isActive = false;
				m_phase = 1.0;
			}
		}
		m_output[bufind] = LERP(initial, target, m_phase);
	}

	void ADSREnvelope::noteOn(int pitch, int vel) {
		m_isActive = true;
		m_currStage = ATTACK;
		m_phase = 0;
	}

	void ADSREnvelope::noteOff(int pitch, int vel) {
		m_currStage = RELEASE;
		m_phase = 0;
	}

	int ADSREnvelope::getSamplesPerPeriod() const {
		double approx = m_attack + m_decay + m_release;
		approx *= m_Fs;
		return int(approx);
	}

	bool ADSREnvelope::isActive() const {
		return m_isActive;
	}

	ADSREnvelope::ADSREnvelope(string name) :
		SourceUnit(name),
		m_attack(addDoubleParam("attack", 0.001, 1.0, 0.001, 1e-3)),
		m_decay(addDoubleParam("decay", 0.001, 1.0, 0.001, 1e-3)),
		m_sustain(addDoubleParam("sustain", 0.0, 1.0, 1.0, 1e-3, 2)),
		m_release(addDoubleParam("release", 0.001, 1.0, 0.001, 1e-3)),
		m_phase(0),
		m_currStage(ATTACK),
		m_isActive(false) { }

	ADSREnvelope::ADSREnvelope(const ADSREnvelope& other) :
		ADSREnvelope(other.m_name) {
		m_phase = other.m_phase;
		m_currStage = other.m_currStage;
		m_isActive = other.m_isActive;
	}

	ADSREnvelope::~ADSREnvelope() { }
}

