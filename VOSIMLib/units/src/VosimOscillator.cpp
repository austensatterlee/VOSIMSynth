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

namespace syn
{
	VosimOscillator::VosimOscillator(string name) :
		TunedOscillator(name),
		m_pulse_step(0.0),
		m_pulse_tune(0),
		m_num_pulses(1),
		m_pPulseTune(addParameter_({ "fp", 0.0, 1.0, 0.0 })),
		m_pNumPulses(addParameter_({ "num", 1, 8, 1 })),
		m_pPulseDecay(addParameter_({ "dec", 0.0, 1.0, 0.0 })) {
		m_iPulseTuneAdd = addInput_("fp");
		m_iPulseTuneMul = addInput_("fp[x]", 1.0);
	}

	VosimOscillator::VosimOscillator(const VosimOscillator& a_rhs) :
		VosimOscillator(a_rhs.getName()) { }

	string VosimOscillator::_getClassName() const {
		return "VosimOscillator";
	}

	Unit* VosimOscillator::_clone() const {
		return new VosimOscillator(*this);
	}

	void VosimOscillator::process_() {
		m_num_pulses = getParameter(m_pNumPulses).getInt();
		m_pulse_tune = CLAMP<double>(getInputValue(m_iPulseTuneMul)*getParameter(m_pPulseTune).getDouble() + getInputValue(m_iPulseTuneAdd), 0, 1);
		TunedOscillator::process_();
		double pulse_decay = getParameter(m_pPulseDecay).getDouble();

		double output = 0.0;

		double unwrapped_pulse_phase = m_phase * m_pulse_step / m_phase_step;

		int curr_pulse_num = static_cast<int>(unwrapped_pulse_phase);

		if (curr_pulse_num < m_num_pulses) {
			double pulse_phase = unwrapped_pulse_phase - curr_pulse_num;
			double pulseval = lut_sin.getlinear(pulse_phase);

			double curr_pulse_gain = 1.0;
			while (curr_pulse_num--) {
				curr_pulse_gain *= (1 - pulse_decay);
			}

			output = pulseval * abs(pulseval);
			output *= curr_pulse_gain * sqrt(m_pulse_step / m_phase_step);
		}
		setOutputChannel_(m_oOut, m_gain * output + m_bias);
	}

	void VosimOscillator::updatePhaseStep_() {
		TunedOscillator::updatePhaseStep_();
		double pulse_freq;
		double min_freq = m_num_pulses * m_freq;
		int MAX_PULSE_FREQ = 4000;
		if (min_freq > MAX_PULSE_FREQ) {
			pulse_freq = min_freq;
		}
		else {
			pulse_freq = LERP<double>(min_freq, MAX_PULSE_FREQ, m_pulse_tune);
		}
		m_pulse_step = pulse_freq / getFs();
	}

	FormantOscillator::FormantOscillator(string name) :
		TunedOscillator(name),
		m_pWidth(addParameter_({ "width", 0.0, 1.0, 0.0 })),
		m_pFmt(addParameter_({ "fmt", 0.0, 1.0, 0.0 })) {
		m_iWidthAdd = addInput_("w");
		m_iWidthMul = addInput_("w[x]", 1.0);
		m_iFmtAdd = addInput_("fmt");
		m_iFmtMul = addInput_("fmt[x]", 1.0);
	}

	FormantOscillator::FormantOscillator(const FormantOscillator& a_rhs) :
		FormantOscillator(a_rhs.getName()) { }

	string FormantOscillator::_getClassName() const {
		return "FormantOscillator";
	}

	Unit* FormantOscillator::_clone() const {
		return new FormantOscillator(*this);
	}

	void FormantOscillator::process_() {
		TunedOscillator::process_();
		double formant_freq;
		double cos_width;
		int MAX_FMT_FREQ = 4000;
		if (m_freq > MAX_FMT_FREQ) {
			formant_freq = m_freq;
			cos_width = 1;
		}
		else {
			double fmt_pitch_norm = CLAMP<double>(getInputValue(m_iFmtMul)*getParameter(m_pFmt).getDouble() + getInputValue(m_iFmtAdd), 0, 1);
			formant_freq = LERP<double>(m_freq, MAX_FMT_FREQ, fmt_pitch_norm);
			cos_width = 1 + 8 * CLAMP<double>(getInputValue(m_iWidthMul)*getParameter(m_pWidth).getDouble() + getInputValue(m_iWidthAdd), 0, 1);
		}

		double formant_step = formant_freq / getFs();
		double formant_phase = m_phase * formant_step / m_phase_step;
		formant_phase = WRAP(formant_phase, 1.0);
		double cos_phase = CLAMP<double>(m_phase * cos_width, 0, 1);

		double sinval = lut_sin.getlinear(formant_phase + 0.5);
		double cosval = 0.5 * (1 + lut_sin.getlinear(cos_phase - 0.25));
		double output = sinval * cosval * sqrt(cos_width);
		setOutputChannel_(m_oOut, m_gain * output + m_bias);
	}
}