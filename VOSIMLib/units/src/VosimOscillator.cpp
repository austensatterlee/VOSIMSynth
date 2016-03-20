/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

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
		m_pulse_tune = CLAMP(a_inputs.getValue(m_iPulseTuneMul)*getParameter(m_pPulseTune).getDouble() + a_inputs.getValue(m_iPulseTuneAdd), 0, 1);
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
			output *= curr_pulse_gain;// *sqrt(m_pulse_step / m_phase_step);
		}
		a_outputs.setChannel(m_oOut,m_gain*output);
	}

	void VosimOscillator::updatePhaseStep_()
	{
		TunedOscillator::updatePhaseStep_();
		double pulse_freq;
		double min_freq = m_num_pulses*m_freq;
		int MAX_PULSE_FREQ = 4000;
		if (min_freq > MAX_PULSE_FREQ) {
			pulse_freq = m_freq*(m_num_pulses);
		}
		else {
			pulse_freq = LERP(min_freq, MAX_PULSE_FREQ, m_pulse_tune);
		}
		m_pulse_step = pulse_freq / getFs();
	}

	void FormantOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
	{
		TunedOscillator::process_(a_inputs, a_outputs);
		double formant_freq;
		double cos_width;
		int MAX_FMT_FREQ = 4000;
		if (m_freq > MAX_FMT_FREQ) {
			formant_freq = m_freq;
			cos_width = 1;
		}else {
            double fmt_pitch_norm = CLAMP(a_inputs.getValue(m_iFmtMul)*getParameter(m_pFmt).getDouble() + a_inputs.getValue(m_iFmtAdd), 0, 1);
			formant_freq = LERP(m_freq, MAX_FMT_FREQ, fmt_pitch_norm);
			cos_width = 1 + 8 * CLAMP(a_inputs.getValue(m_iWidthMul)*getParameter(m_pWidth).getDouble() + a_inputs.getValue(m_iWidthAdd), 0, 1);
		}

		double formant_step = formant_freq / getFs();
		double formant_phase = m_phase * formant_step / m_phase_step;
		formant_phase = WRAP(formant_phase, 1.0);
		double cos_phase = CLAMP(m_phase * cos_width, 0, 1);

		double sinval = lut_sin.getlinear(formant_phase + 0.5);
		double cosval = 0.5*(1+lut_sin.getlinear(cos_phase - 0.25));
		double output = sinval*cosval;// *sqrt(cos_width);
        a_outputs.setChannel(m_oOut,m_gain*output);
	}	
}

