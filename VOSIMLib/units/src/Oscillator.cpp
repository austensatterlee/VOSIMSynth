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

#include "Oscillator.h"
#include "DSPMath.h"
#include "tables.h"

namespace syn {

    double sampleWaveShape(WAVE_SHAPE shape, double phase, double period, bool useNaive)
    {
        double output;
        switch (static_cast<int>(shape)) {
            case SAW_WAVE:
				if (useNaive) {
					if (phase < 0.5)
						output = 2 * phase;
					else
						output = 2 * phase - 2;
				}else{
					output = lut_bl_saw.getresampled(phase, period);
				}
                break;
            case SINE_WAVE:
                output = lut_sin.getlinear(phase);
                break;
            case TRI_WAVE:
                output = phase <= 0.5 ? 4 * phase - 1 : -4 * (phase - 0.5) + 1;
                break;
            case SQUARE_WAVE:
				if (useNaive) 
					output = phase <= 0.5 ? 1 : -1;
				else
					output = -lut_bl_saw.getresampled(phase + 0.5, period) + lut_bl_saw.getresampled(phase, period);
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
	
	void Oscillator::updatePhaseStep_()
	{
		if (m_freq) {
			m_period = getFs() / m_freq;
			m_phase_step = 1. / m_period;
		} else {
			m_period = 0.0;
			m_phase_step = 0.0;
		}
	}

	void Oscillator::tickPhase_(double a_phaseOffset)
    {
        m_basePhase += m_phase_step;
		if (m_basePhase >= 1) {
			m_basePhase -= 1;
		}
        m_phase = m_basePhase + a_phaseOffset;
		m_phase = WRAP(m_phase,1.0);
		if (1-(m_last_phase-m_phase) < 0.5) {
			sync_();
		}
		m_last_phase = m_phase;
    }

	void Oscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		
		double phase_offset = getParameter(m_pPhaseOffset).getDouble() + a_inputs.getValue(m_iPhaseAdd);
		m_gain = getParameter(m_pGain).getDouble() * a_inputs.getValue(m_iGainMul);
		m_bias = 0;
		if(getParameter(m_pUnipolar).getBool()) { 
			// make signal unipolar
			m_gain *= 0.5;
			m_bias = m_gain;
		}
		updatePhaseStep_();
		tickPhase_(phase_offset);

		a_outputs.setChannel(m_oPhase, m_phase);
    }

	void TunedOscillator::onNoteOn_() {
		m_basePhase = 0.0;
		sync_();
	}

	void TunedOscillator::updatePhaseStep_()
	{
		m_freq = pitchToFreq(m_pitch);
		Oscillator::updatePhaseStep_();
	}

	void TunedOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		double tune = getParameter(m_pTune).getDouble() + 128*a_inputs.getValue(m_iNote);
		double oct = getParameter(m_pOctave).getInt();
		m_pitch = tune + oct * 12;
		Oscillator::process_(a_inputs, a_outputs);
	}

    void BasicOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
    {
		TunedOscillator::process_(a_inputs, a_outputs);
        double output;
        int shape = getParameter(m_pWaveform).getInt();
        output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period, false);
        a_outputs.setChannel(m_oOut, m_gain*output + m_bias);
    }

	void LFOOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		// determine frequency
		if (getParameter(m_pTempoSync).getBool()) {
			m_freq = getTempo()/60.0*0.0625*a_inputs.getValue(m_iFreqMul)*(getParameter(m_pFreq).getDouble() + a_inputs.getValue(m_iFreqAdd));
		}else {
			m_freq = a_inputs.getValue(m_iFreqMul)*(getParameter(m_pFreq).getDouble() + a_inputs.getValue(m_iFreqAdd));
		}
		// sync
		if (m_lastSync<=0.0 && a_inputs.getValue(m_iSync) > 0.0) {
			m_basePhase = 0.0;
			sync_();
		}
		m_lastSync = a_inputs.getValue(m_iSync);
		Oscillator::process_(a_inputs, a_outputs);
		double output;
		int shape = getParameter(m_pWaveform).getInt();
		output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period, true);
		a_outputs.setChannel(m_oOut, m_gain*output + m_bias);
		///  Output quad output
		/// \todo <experimentation>
		double quadoutput;
		quadoutput = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase + 0.25, m_period, true);
		a_outputs.setChannel(m_oQuadOut, m_gain*quadoutput + m_bias);
    }

	void LFOOscillator::onParamChange_(int a_paramId) {
	    if(a_paramId==m_pTempoSync) {
			double normFreq = getParameter(m_pFreq).getNorm();
		    if(getParameter(m_pTempoSync).getBool()) {
				getParameter_(m_pFreq) = m_syncedFreqParam;
		    }else {
				getParameter_(m_pFreq) = m_linFreqParam;
		    }
			setParameterNorm(m_pFreq, normFreq);
	    }
    }
}

