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
#include "vosimlib/units/VosimOscillator.h"
#include "vosimlib/tables.h"
#include "vosimlib/DSPMath.h"

namespace syn
{
    VosimOscillator::VosimOscillator(string name) :
        TunedOscillatorUnit(name),
        m_pulse_step(0.0),
        m_pulse_tune(0),
        m_num_pulses(1)
    {
        addParameter_(pPulseTune, {"fp", 0.0, 1.0, 0.0});
        addParameter_(pNumPulses, {"num", 1, 8, 1});
        addParameter_(pPulseDecay, {"dec", 0.0, 1.0, 0.0});
        addInput_(iPulseTuneAdd, "fp");
        addInput_(iPulseTuneMul, "fp[x]", 1.0);
        addInput_(iDecayMul, "dec[x]", 1.0);
    }

    VosimOscillator::VosimOscillator(const VosimOscillator& a_rhs) :
        VosimOscillator(a_rhs.name()) { }

    void VosimOscillator::process_()
    {
        BEGIN_PROC_FUNC
            m_num_pulses = param(pNumPulses).getInt();
            m_pulse_tune = CLAMP<double>(READ_INPUT(iPulseTuneMul) * (param(pPulseTune).getDouble() + READ_INPUT(iPulseTuneAdd)), 0, 1);
            TunedOscillatorUnit::process_();
            double pulse_decay = CLAMP<double>(READ_INPUT(iDecayMul) * param(pPulseDecay).getDouble(), 0, 1);

            double output = 0.0;

            double unwrapped_pulse_phase = m_phase * m_pulse_step / m_phase_step;

            int curr_pulse_num = static_cast<int>(unwrapped_pulse_phase);

            if (curr_pulse_num < m_num_pulses)
            {
                double pulse_phase = unwrapped_pulse_phase - curr_pulse_num;
                double pulseval = lut_sin_table().plerp(pulse_phase);

                double curr_pulse_gain = 1.0;
                while (curr_pulse_num--)
                {
                    curr_pulse_gain *= (1 - pulse_decay);
                }

                output = pulseval * pulseval;
                output *= curr_pulse_gain; // * sqrt(m_pulse_step / m_phase_step);
            }
            WRITE_OUTPUT(oOut, m_gain * output + m_bias);
        END_PROC_FUNC
    }

    void VosimOscillator::updatePhaseStep_()
    {
        TunedOscillatorUnit::updatePhaseStep_();
        double pulse_freq;
        double min_freq = m_num_pulses * m_freq;
        int MAX_PULSE_FREQ = 3000;
        if (min_freq > MAX_PULSE_FREQ)
        {
            pulse_freq = min_freq;
        }
        else
        {
            pulse_freq = pow(2.0, LERP<double>(log2(min_freq), log2(MAX_PULSE_FREQ), m_pulse_tune));
        }
        m_pulse_step = pulse_freq / fs();
    }

    FormantOscillator::FormantOscillator(string name) :
        TunedOscillatorUnit(name)
    {
        addParameter_(pWidth, {"width", 0.0, 1.0, 0.0});
        addParameter_(pFmt, {"fmt", 200.0, 4000.0, 200.0});
        addInput_(iWidthAdd, "w");
        addInput_(iWidthMul, "w[x]", 1.0);
        addInput_(iFmtAdd, "fmt");
        addInput_(iFmtMul, "fmt[x]", 1.0);
    }

    FormantOscillator::FormantOscillator(const FormantOscillator& a_rhs) :
        FormantOscillator(a_rhs.name()) { }

    void FormantOscillator::process_()
    {
        BEGIN_PROC_FUNC
            TunedOscillatorUnit::process_();
            double fmtFreq;
            double width;
            if (m_freq > fs()/8.0)
            {
                fmtFreq = m_freq;
                width = 1;
            }
            else
            {
                fmtFreq = CLAMP<double>(READ_INPUT(iFmtMul) * (param(pFmt).getDouble() + READ_INPUT(iFmtAdd)), param(pFmt).getMin(), param(pFmt).getMax());
                width = 1 + 6 * CLAMP<double>(READ_INPUT(iWidthMul) * (param(pWidth).getDouble() + READ_INPUT(iWidthAdd)), 0, 1);
            }
            
            double cos_phase = CLAMP<double>(m_phase * width, 0, 1);
            double cosval = 0.5 * (1 + lut_sin_table().plerp(cos_phase - 0.25));

            double sinval = lut_sin_table().plerp(m_phase*fmtFreq/m_freq);
            double output = sinval * cosval * sqrt(width);
            WRITE_OUTPUT(oOut, m_gain * output + m_bias);
        END_PROC_FUNC
    }
}
