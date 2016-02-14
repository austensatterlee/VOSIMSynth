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
		Oscillator::process_(a_inputs, a_outputs);
        int num_pulses = getParameter(m_pNumPulses).getInt();
        double pulse_decay = getParameter(m_pPulseDecay).getDouble();

		double output = 0.0;
		double unwrapped_pulse_phase = m_phase * m_pulse_step / m_phase_step;
		double pulse_phase = WRAP(unwrapped_pulse_phase, 1.0);
		int curr_pulse_num = unwrapped_pulse_phase - pulse_phase;
		
		if (curr_pulse_num <= num_pulses) {
			double pulseval = lut_sin.getlinear(pulse_phase);
			
			output = pulseval*abs(pulseval);
			output *= m_curr_pulse_gain * sqrt(m_pulse_step / m_phase_step);

			if (pulse_phase >= 1-m_pulse_step) {
				m_curr_pulse_gain *= 1 - pulse_decay;
			}
		}
		a_outputs.setChannel(0,output * m_gain);
	}

	void VosimOscillator::updatePhaseStep_()
	{
		Oscillator::updatePhaseStep_();
		double pulse_freq;
        int num_pulses = getParameter(m_pNumPulses).getInt();
        double pulse_tune = getParameter(m_pPulseTune).getDouble();
		double min_freq = num_pulses*m_freq;
		double max_freq = 2000;
		if (min_freq > max_freq) {
			pulse_freq = m_freq*(num_pulses);
		}
		else {
			pulse_freq = LERP(min_freq, max_freq, pulse_tune);
		}
		m_pulse_step = pulse_freq / getFs();
	}

	void VosimOscillator::sync_() {
		Oscillator::sync_();
		m_curr_pulse_gain = 1.0;
	}

	void FormantOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
	{
		Oscillator::process_(a_inputs, a_outputs);
		double formant_freq;
		double cos_width;
		if (m_pitch > 95) {
			formant_freq = m_freq;
			cos_width = 1;
		}else {
            double fmt_pitch_norm = getParameter(m_pFmtpitch).getDouble();
			double formant_pitch = LERP(m_pitch, 95, fmt_pitch_norm);
			formant_freq = pitchToFreq(formant_pitch);
			cos_width = 1 + 10 * getParameter(m_pWidth).getDouble();
		}

		double formant_step = formant_freq / getFs();
		double formant_phase = m_phase * formant_step / m_phase_step;
		formant_phase = WRAP(formant_phase, 1.0);
		double cos_phase = CLAMP(m_phase * cos_width, 0, 1);

		double sinval = lut_sin.getlinear(formant_phase + 0.5);
		double cosval = 0.5*(1+lut_sin.getlinear(cos_phase - 0.25));
		double output = sinval*cosval*sqrt(cos_width);
        a_outputs.setChannel(0,output * m_gain);
	}	
}

