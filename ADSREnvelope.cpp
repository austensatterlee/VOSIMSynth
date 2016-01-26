#include "ADSREnvelope.h"
#include "DSPMath.h"


namespace syn
{
	void ADSREnvelope::onSampleRateChange(double newfs) { }

	void ADSREnvelope::process(int bufind) {
		/* Determine phase increment based on current segment */
		double segment_time;
		switch (m_currStage) {
		case ATTACK:
			segment_time = m_attack;
			m_initial = 0;
			m_target = m_decay>0 ? 1 : static_cast<double>(m_sustain); // skip decay segment if its length is zero
			break;
		case DECAY:
			segment_time = m_decay;
			m_target = m_sustain;
			break;
		case SUSTAIN:
			segment_time = 0;
			m_initial = m_sustain;
			m_target = m_sustain;
			break;
		case RELEASE:
			segment_time = m_release;
			break;
		default:
			throw std::logic_error("Invalid envelope stage");
		}
		if (!segment_time) { // for zero second segment time, advance phase pointer to next segment
			m_phase += 1;
		}else {
			m_phase += 1. / (segment_time*m_Fs);
		}
		

		/* Handle segment change */
		if (m_phase > 1.0) {
			if (m_currStage == ATTACK) {
				m_currStage = DECAY;
				m_initial = getLastOutput();
				m_phase = 0.0;
			} else if (m_currStage == DECAY) {
				m_currStage = SUSTAIN;
				m_phase = 1.0;
			} else if (m_currStage == SUSTAIN) {
				m_currStage = SUSTAIN;
				m_phase = 0.0;
			} else if (m_currStage == RELEASE) {
				m_isActive = false;
				m_phase = 1.0;
			}
		}
		double output = LERP(m_initial, m_target, m_phase);
		m_output[bufind][0] = output;
		m_output[bufind][1] = output;
	}

	void ADSREnvelope::noteOn(int pitch, int vel) {
		m_isActive = true;
		m_currStage = ATTACK;
		m_phase = 0;
	}

	void ADSREnvelope::noteOff(int pitch, int vel) {
		m_currStage = RELEASE;
		m_initial = getLastOutput();
		m_target = 0;
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
		m_attack(addDoubleParam("attack", 0.0, 1.0, 0.001, 1e-3)),
		m_decay(addDoubleParam("decay", 0.0, 1.0, 0.001, 1e-3)),
		m_sustain(addDoubleParam("sustain", 0.0, 1.0, 1.0, 1e-3)),
		m_release(addDoubleParam("release", 0.0, 1.0, 0.001, 1e-3)),
		m_phase(0),
		m_currStage(ATTACK), 
		m_initial(0),
		m_target(1),
		m_isActive(false) { }

	ADSREnvelope::ADSREnvelope(const ADSREnvelope& other) :
		ADSREnvelope(other.m_name) {
		m_phase = other.m_phase;
		m_currStage = other.m_currStage;
		m_isActive = other.m_isActive;
	}

	ADSREnvelope::~ADSREnvelope() { }
}

