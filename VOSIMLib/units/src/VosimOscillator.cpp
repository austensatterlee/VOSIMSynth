#include "VosimOscillator.h"
#include "tables.h"
#include "DSPMath.h"

/******************************
* VOSIM methods
*
******************************/
namespace syn
{
	void VosimOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
	{
        m_num_pulses = getParameter(m_pNumPulses).getInt();
		m_pulse_tune = CLAMP(getParameter(m_pPulseTune).getDouble() + a_inputs.getValue(m_iPulseTune), 0, 1);
		TunedOscillator::process_(a_inputs, a_outputs);
        double pulse_decay = getParameter(m_pPulseDecay).getDouble();

		double output = 0.0;

		double unwrapped_pulse_phase = m_phase * m_pulse_step / m_phase_step;

		int curr_pulse_num = static_cast<int>(unwrapped_pulse_phase);

		if (curr_pulse_num < m_num_pulses) {
			double pulse_phase = unwrapped_pulse_phase - curr_pulse_num;
			double pulseval = lut_sin.getlinear(pulse_phase);

			double curr_pulse_gain = 1.0;
			while(curr_pulse_num--) {
				curr_pulse_gain *= (1 - pulse_decay);
			}
			
			output = pulseval*abs(pulseval);
			output *= curr_pulse_gain * sqrt(m_pulse_step / m_phase_step);
		}
		a_outputs.setChannel(m_oOut,m_gain*output);
	}

	void VosimOscillator::updatePhaseStep_()
	{
		TunedOscillator::updatePhaseStep_();
		double pulse_freq;
		double min_freq = m_num_pulses*m_freq;
		double max_freq = 2000;
		if (min_freq > max_freq) {
			pulse_freq = m_freq*(m_num_pulses);
		}
		else {
			pulse_freq = LERP(min_freq, max_freq, m_pulse_tune);
		}
		m_pulse_step = pulse_freq / getFs();
	}

	void FormantOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
	{
		TunedOscillator::process_(a_inputs, a_outputs);
		double formant_freq;
		double cos_width;
		if (m_pitch > 95) {
			formant_freq = m_freq;
			cos_width = 1;
		}else {
            double fmt_pitch_norm = CLAMP(getParameter(m_pFmtpitch).getDouble() + a_inputs.getValue(m_iFmtpitch), 0, 1);
			double formant_pitch = LERP(m_pitch, 95, fmt_pitch_norm);
			formant_freq = pitchToFreq(formant_pitch);
			cos_width = 1 + 9 * CLAMP(getParameter(m_pWidth).getDouble() + a_inputs.getValue(m_iWidth), 0, 1);
		}

		double formant_step = formant_freq / getFs();
		double formant_phase = m_phase * formant_step / m_phase_step;
		formant_phase = WRAP(formant_phase, 1.0);
		double cos_phase = CLAMP(m_phase * cos_width, 0, 1);

		double sinval = lut_sin.getlinear(formant_phase + 0.5);
		double cosval = 0.5*(1+lut_sin.getlinear(cos_phase - 0.25));
		double output = sinval*cosval*sqrt(cos_width);
        a_outputs.setChannel(m_oOut,m_gain*output);
	}	
}

