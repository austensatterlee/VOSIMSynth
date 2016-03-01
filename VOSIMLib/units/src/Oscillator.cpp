#include "Oscillator.h"
#include "DSPMath.h"
#include "tables.h"

namespace syn {

    double sampleWaveShape(WAVE_SHAPE shape, double phase, double period)
    {
        double output;
        switch (static_cast<int>(shape)) {
            case SAW_WAVE:
                output = lut_bl_saw.getresampled(phase, period);
                break;
            case NAIVE_SAW_WAVE:
                if (phase < 0.5)
                    output = 2 * phase;
                else
                    output = 2 * phase - 2;
                break;
            case SINE_WAVE:
                output = lut_sin.getlinear(phase);
                break;
            case TRI_WAVE:
                output = phase <= 0.5 ? 4 * phase - 1 : -4 * (phase - 0.5) + 1;
                break;
            case SQUARE_WAVE:
                output = -lut_bl_saw.getresampled(phase + 0.5, period) + lut_bl_saw.getresampled(phase, period);
                break;
            case NAIVE_SQUARE_WAVE:
                output = phase <= 0.5 ? 1 : -1;
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
		if (m_phase < m_last_phase) {
			sync_();
		}
		m_last_phase = m_phase;
    }

	void Oscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		
		double phase_offset = getParameter(m_pPhaseOffset).getDouble() + a_inputs.getValue(m_iPhaseAdd);
		m_gain = getParameter(m_pGain).getDouble() * a_inputs.getValue(m_iGainAdd);

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
		double tune = getParameter(m_pTune).getDouble() + a_inputs.getValue(m_iNote);
		double oct = getParameter(m_pOctave).getInt();
		m_pitch = tune + oct * 12;
		Oscillator::process_(a_inputs, a_outputs);
	}

    void BasicOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
    {
		TunedOscillator::process_(a_inputs, a_outputs);
        double output;
        int shape = getParameter(m_pWaveform).getInt();
        output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period);
        a_outputs.setChannel(m_oOut, m_gain * output);
    }

	void LFOOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		// determine frequency
		if (getParameter(m_pTempoSync).getBool()) {
			m_freq = getTempo()/60.0*0.25*a_inputs.getValue(m_iFreqMul)*(getParameter(m_pFreq).getDouble() + a_inputs.getValue(m_iFreqAdd));
		}else {
			m_freq = a_inputs.getValue(m_iFreqMul)*(getParameter(m_pFreq).getDouble() + a_inputs.getValue(m_iFreqAdd));
		}
		// sync
		if (m_lastSync<0.5 && a_inputs.getValue(m_iSync) >= 0.5) {
			m_basePhase = 0.0;
			sync_();
		}
		m_lastSync = a_inputs.getValue(m_iSync);
		Oscillator::process_(a_inputs, a_outputs);
		double output;
		int shape = getParameter(m_pWaveform).getInt();
		output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period);
		a_outputs.setChannel(m_oOut, m_gain * output);
    }

	void LFOOscillator::onParamChange_(int a_paramId) {
	    if(a_paramId==m_pTempoSync) {
			double normFreq = getParameter(m_pFreq).getNorm();
		    if(getParameter(m_pTempoSync).getBool()) {
				getParameter_(m_pFreq) = UnitParameter("rate", 1, 16, 1);
		    }else {
				getParameter_(m_pFreq) = UnitParameter("freq", 0.0, 20.0, 1.0);
		    }
			setParameterNorm(m_pFreq, normFreq);
	    }
    }
}

