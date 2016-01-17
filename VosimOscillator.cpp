#include "VosimOscillator.h"

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
		m_pulse_step = m_Step * (m_number + 8 * m_ppitch * m_relativeamt)*m_relativeamt + 1 / m_Fs * (pitchToFreq(m_ppitch * 57 * (1 - m_relativeamt) + 57))*(1- m_relativeamt) ;
		m_unwrapped_pulse_phase = m_phase / m_Step * m_pulse_step;
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
			double tableval = 1 - lut_sin.getlinear(m_pulse_phase + 0.25);
			m_output[bufind] = m_gain * m_velocity * m_curr_pulse_gain * tableval;
		}
	}

	void VosimChoir::onSampleRateChange(double newfs)
	{
		for (int i = 0; i < m_size; i++)
		{
			m_choir[i]->setSampleRate(newfs);
			m_pulsedrifters[i]->setSampleRate(newfs);
		}
	}

	void VosimOscillator::sync()
	{
		Oscillator::sync();
		m_pulse_phase = 0;
		m_unwrapped_pulse_phase = 0;
	}
}

