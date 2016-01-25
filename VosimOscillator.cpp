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
		m_curr_pulse_gain = vosc.m_curr_pulse_gain;
		m_pulse_step = vosc.m_pulse_step;
		m_pulse_phase = vosc.m_pulse_phase;
		m_unwrapped_pulse_phase = vosc.m_unwrapped_pulse_phase;
	}

	void VosimOscillator::process(int bufind)
	{
		Oscillator::tick_phase();
		double pulse_pitch = LERP(12*log2(m_number*m_freq/440.)+69, 12 * log2(13000 / 440.) + 69, m_ppitch);
		double pulse_freq = pitchToFreq(pulse_pitch);
		m_pulse_step = pulse_freq / m_Fs;
		m_unwrapped_pulse_phase = m_phase / m_phase_step * m_pulse_step;
		if (m_unwrapped_pulse_phase < 1)
		{
			m_curr_pulse_gain = 1.0;
		}
		if (m_unwrapped_pulse_phase >= m_number)
		{
			m_output[bufind] = 0;
		}
		else
		{
			m_last_pulse_phase = m_pulse_phase;
			m_pulse_phase = m_unwrapped_pulse_phase - static_cast<int>(m_unwrapped_pulse_phase);
			if (m_last_pulse_phase > m_pulse_phase)
			{
				m_curr_pulse_gain *= m_decay;
			}
			double tableval = 1 - lut_sin.getresampled(m_pulse_phase + 0.25,1./m_pulse_step);
			m_output[bufind] = m_gain * m_velocity * m_curr_pulse_gain * tableval;
		}
	}

	void VosimOscillator::sync()
	{
		Oscillator::sync();
		m_pulse_phase = 0;
		m_unwrapped_pulse_phase = 0;
	}
}

