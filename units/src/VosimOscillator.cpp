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
		double tune = getParameter(m_pTune).getDouble() + a_inputs.getValue(1);
		double oct = getParameter(m_pOctave).getInt();
		double gain = getParameter(m_pGain).getDouble() * a_inputs.getValue(0);
        int num_pulses = getParameter(m_pNumPulses).getInt();
        double pulse_decay = getParameter(m_pPulseDecay).getDouble();
		m_pitch = tune + oct * 12;

        update_step();
        tick_phase();
		double output = 0.0;
		
		if (m_curr_pulse_num <= num_pulses) {
			double pulse_phase = m_phase * m_pulse_step / m_phase_step;
			pulse_phase = WRAP(pulse_phase, 1.0);
			double pulseval = lut_sin.getlinear(pulse_phase);
			
			output = pulseval*abs(pulseval);
			output *= m_curr_pulse_gain * sqrt(m_pulse_step / m_phase_step);

			if (pulse_phase <= m_pulse_step) {
				m_curr_pulse_num++;
				m_curr_pulse_gain *= 1 - pulse_decay;
			}
		}
        if (m_phase <= m_phase_step) {
            m_curr_pulse_num=0;
            m_curr_pulse_gain = 1.0;
        }
		a_outputs.setChannel(0,output * gain);
	}

	void VosimOscillator::update_step()
	{
		Oscillator::update_step();
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

	void FormantOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
	{
        double tune = getParameter(m_pTune).getDouble() + a_inputs.getValue(1);
        double oct = getParameter(m_pOctave).getInt();
        double gain = getParameter(m_pGain).getDouble() * a_inputs.getValue(0);
        m_pitch = tune + oct * 12;

        update_step();
        tick_phase();
		double formant_freq;
		double cos_width;
		if (m_pitch > 95) {
			formant_freq = m_freq;
			cos_width = 1;
		}else {
            double fmt_pitch_norm = getParameter(m_pFmtpitch).getDouble();
			double formant_pitch = LERP(m_pitch, 95, fmt_pitch_norm);
			formant_freq = pitchToFreq(formant_pitch);
			cos_width = 1 + 10 * m_pWidth;
		}
		double formant_step = formant_freq / getFs();
		double formant_phase = m_phase * formant_step / m_phase_step;
		formant_phase = WRAP(formant_phase, 1.0);
		double cos_phase = CLAMP(m_phase * cos_width, 0, 1);

		double sinval = lut_sin.getlinear(formant_phase + 0.5);
		double cosval = 0.5*(1+lut_sin.getlinear(cos_phase - 0.25));
		double output = sinval*cosval*sqrt(cos_width);
        a_outputs.setChannel(0,output * gain);
	}	
}

