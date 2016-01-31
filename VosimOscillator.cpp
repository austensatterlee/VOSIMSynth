#include "VosimOscillator.h"
#include "tables.h"
#include "DSPMath.h"

/******************************
* VOSIM methods
*
******************************/
namespace syn
{

	VosimOscillator::VosimOscillator(const VosimOscillator& vosc) :
		VosimOscillator(vosc.m_name)
	{
		m_pulse_step = vosc.m_pulse_step;
		m_curr_pulse_gain = vosc.m_curr_pulse_gain;
		m_curr_pulse_num = vosc.m_curr_pulse_num;
	}

	void VosimOscillator::onParamChange(const UnitParameter* param)
	{
		if(m_prevFreq!=m_freq || param==&m_pulse_tune || param==&m_num_pulses) {
			update_step();
		}
		BasicOscillator::onParamChange(param);		
	}

	void VosimOscillator::process(int bufind)
	{
		double output = 0.0;
		
		if (m_curr_pulse_num <= m_num_pulses) {
			double pulse_phase = m_phase * m_pulse_step / m_phase_step;
			pulse_phase = WRAP(pulse_phase, 1.0);
			double pulseval = lut_sin.getlinear(pulse_phase);
			
			output = pulseval*abs(pulseval);
			output *= m_curr_pulse_gain * sqrt(m_pulse_step / m_phase_step);

			output = m_velocity * output;
			if (1-pulse_phase <= m_pulse_step) {
				m_curr_pulse_num++;
				m_curr_pulse_gain *= 1 - m_pulse_decay;
			}
		}
		m_output[bufind][0] = output;
		m_output[bufind][1] = output;
	}

	void VosimOscillator::sync()
	{
		Oscillator::sync();
		m_curr_pulse_gain = 1.0;
		m_curr_pulse_num = 0;
	}

	void VosimOscillator::update_step()
	{
		BasicOscillator::update_step();
		double pulse_freq;
		double min_freq = m_num_pulses*m_freq;
		double max_freq = 2000;
		if (min_freq > max_freq) {
			pulse_freq = m_freq*(m_num_pulses);
		}
		else {
			pulse_freq = LERP(min_freq, max_freq, m_pulse_tune);
		}
		m_pulse_step = pulse_freq / m_Fs;
	}

	void FormantOscillator::process(int bufind)
	{
		double formant_freq;
		double cos_width;
		if (m_pitch > 95) {
			formant_freq = m_freq;
			cos_width = 1;
		}else {
			double formant_pitch = LERP(m_pitch, 95, m_fmtpitch);
			formant_freq = pitchToFreq(formant_pitch);
			cos_width = 1 + 10 * m_width;
		}
		double formant_step = formant_freq / m_Fs;
		double formant_phase = m_phase * formant_step / m_phase_step;
		formant_phase = WRAP(formant_phase, 1.0);
		double cos_phase = CLAMP(m_phase * cos_width, 0, 1);

		double sinval = lut_sin.getlinear(formant_phase + 0.5);
		double cosval = 0.5*(1+lut_sin.getlinear(cos_phase - 0.25));
		double output = sinval*cosval*sqrt(cos_width);
		output = m_velocity * output;
		m_output[bufind][0] = output;
		m_output[bufind][1] = output;
	}	
}

