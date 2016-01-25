#include "Oscillator.h"
#include "DSPMath.h"

namespace syn
{

	double sampleWaveShape(WAVE_SHAPE shape, double phase, double period) {
		double output;
		switch (static_cast<int>(shape)) {
		case SAW_WAVE:
			output = lut_bl_saw.getresampled(phase, period);
			break;
		case NAIVE_SAW_WAVE:
			if (phase < 0.5)
				output = 2 * phase;
			else
				output = 2 * phase - 2;
			break;
		case SINE_WAVE:
			output = lut_sin.getresampled(phase, period);
			break;
		case NAIVE_SINE_WAVE:
			output = lut_sin.getlinear(phase);
			break;
		case TRI_WAVE:
			output = phase <= 0.5 ? 4 * phase - 1 : -4 * (phase - 0.5) + 1;
			break;
		case SQUARE_WAVE:
			output = -lut_bl_saw.getresampled(phase + 0.5, period) + lut_bl_saw.getresampled(phase, period);
			break;
		case NAIVE_SQUARE_WAVE:
			output = phase <= 0.5 ? 1 : -1;
			break;
		default:
			output = 0;
			break;
		}
		return output;
	}

	/******************************
	* Oscillator methods
	*
	******************************/

	void Oscillator::onSampleRateChange(double newfs) {		
		m_Fs = newfs;
		update_step();
	}

	void Oscillator::beginProcessing() {
		update_step();
		tick_phase();
	}

	void Oscillator::finishProcessing() {
		m_output[m_bufind] *= m_gain;
	}

	void Oscillator::update_step() {
		m_period = m_Fs / m_freq;
		m_phase_step = 1. / m_period;
	}

	/**
		 * \brief
		 * \todo Maybe let Unit inheritors implement a function that gets called only when some parameters have changed
		 */
	void Oscillator::tick_phase() {
		m_basePhase += m_phase_step;
		if (m_basePhase >= 1)
			m_basePhase -= 1;
		m_phase = m_basePhase + m_phase_shift;
		if (m_phase >= 1)
			m_phase -= 1;
		else if (m_phase < 0)
			m_phase += 1;
		if (m_phase < m_phase_step)
			updateSyncStatus();
	}

	void BasicOscillator::noteOn(int pitch, int vel) {
		sync();
		m_velocity = DBToAmp(LERP(-30, 0, vel / 127.0));
		m_pitch = pitch;
		m_isPitchDirty = true;
	}

	void BasicOscillator::noteOff(int pitch, int vel) {}

	void BasicOscillator::update_step() {
		if(m_tune.isDirty() || m_octave.isDirty() || m_isPitchDirty) {
			m_freq = pitchToFreq(m_pitch + m_tune + m_octave * 12);
			m_tune.setClean();
			m_octave.setClean();
			m_isPitchDirty = false;
		}
		Oscillator::update_step();
	}

	void BasicOscillator::process(int bufind) {
		double output;
		int shape = m_waveform;
		output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period);
		m_output[bufind] = output*m_velocity;
	}

	void LFOOscillator::onTempoChange(double newtempo) {
		m_tempo = newtempo;
	}

	void LFOOscillator::process(int bufind) {
		double output;
		int shape = m_waveform;
		output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period);
		m_output[bufind] = output;
	}

	void LFOOscillator::update_step() {
		// Switch between rate and frequency parameters depending on the useBPM switch
		if(m_useBPM.isDirty()) {
			if(m_useBPM) {
				m_freq_param.setHidden(true);
				m_freq_param.setCanModulate(false);
				m_rate.setHidden(false);
				m_rate.setCanModulate(true);
			}else {
				m_freq_param.setHidden(false);
				m_freq_param.setCanModulate(true);
				m_rate.setHidden(true);
				m_rate.setCanModulate(false);
			}
		}
		// Update frequency
		if (m_freq_param.isDirty() && !m_useBPM) {
			m_freq = m_freq_param;
		}
		else if(m_rate.isDirty() && m_useBPM) {
			m_freq = m_rate / 8.0*m_tempo;
		}
		Oscillator::update_step();
	}
}

