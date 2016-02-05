#include "Oscillator.h"
#include "DSPMath.h"
#include "tables.h"

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
		SourceUnit::beginProcessing();
		m_prevFreq = m_freq;
		tick_phase();
	}

	void Oscillator::finishProcessing() {
		m_output[m_bufind][0] = m_output[m_bufind][0]*m_gain;
		m_output[m_bufind][1] = m_output[m_bufind][1]*m_gain;
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
		if (m_phase < m_phase_step) {
			sync();
			updateSyncStatus();
		}
	}

	void BasicOscillator::noteOn(int pitch, int vel) {
		sync();
		m_velocity = DBToAmp(LERP(-30, 0, vel / 127.0));
		m_notepitch = pitch;
		update_step();
	}

	void BasicOscillator::noteOff(int pitch, int vel) {}

	void BasicOscillator::onParamChange(const UnitParameter* param)
	{
		if(param == &m_tune || param == &m_octave)
			update_step();
	}

	void BasicOscillator::update_step() {
		m_pitch = m_notepitch + m_tune + m_octave * 12;
		m_freq = pitchToFreq(m_pitch);
		Oscillator::update_step();
	}

	void BasicOscillator::process(int bufind) {
		double output;
		int shape = m_waveform;
		output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period);
		m_output[bufind][0] = output*m_velocity;
		m_output[bufind][1] = output*m_velocity;
	}

	void LFOOscillator::onTempoChange(double newtempo) {
		m_tempo = newtempo;
	}

	void LFOOscillator::onParamChange(const UnitParameter* param)
	{
		bool useBPM_bool = m_useBPM;
		if (param==&m_useBPM)
		{
			// Switch between rate and frequency parameters depending on the useBPM switch
			if (useBPM_bool) {
				m_freq_param.setCanEdit(false);
				m_freq_param.setCanModulate(false);
				m_rate.setCanEdit(true);
				m_rate.setCanModulate(true);
			}
			else {
				m_freq_param.setCanEdit(true);
				m_freq_param.setCanModulate(true);
				m_rate.setCanEdit(false);
				m_rate.setCanModulate(false);
			}
			update_step();
		}
		else if (useBPM_bool && param == &m_rate) {
			update_step();
		}else if (!useBPM_bool && param == &m_freq_param) {
			update_step();
		}
	}

	void LFOOscillator::process(int bufind) {
		double output;
		int shape = m_waveform;
		output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period);
		m_output[bufind][0] = output;
		m_output[bufind][1] = output;
	}

	void LFOOscillator::update_step() {
		// Update frequency
		if (!m_useBPM) 
		{
			m_freq = m_freq_param;
		}
		else 
		{
			m_freq = 16.0/m_rate*(m_tempo/60.0);
		}
		Oscillator::update_step();
	}
}

