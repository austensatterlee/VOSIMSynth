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
        m_freq = pitchToFreq(m_pitch);
        m_period = getFs() / m_freq;
        m_phase_step = 1. / m_period;
    }

	void Oscillator::onNoteOn_() {
		m_basePhase = 0.0;
		sync_();
    }

	void Oscillator::tickPhase_(double a_phaseOffset)
    {
        m_basePhase += m_phase_step;
		if (m_basePhase >= 1) {
			m_basePhase -= 1;
		}
        m_phase = m_basePhase + a_phaseOffset;
		m_phase = WRAP(m_phase,1.0);
		if (m_phase < m_phase_step) {
			sync_();
		}
    }


	void Oscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		double tune = getParameter(m_pTune).getDouble() + a_inputs.getValue(m_iNote);
		double oct = getParameter(m_pOctave).getInt();
		double phase_offset = getParameter(m_pPhaseOffset).getDouble() + a_inputs.getValue(m_iPhaseOffset);
		m_gain = getParameter(m_pGain).getDouble() * a_inputs.getValue(m_iGain);
		m_pitch = tune + oct * 12;

		updatePhaseStep_();
		tickPhase_(phase_offset);
    }

    void BasicOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
    {
		Oscillator::process_(a_inputs, a_outputs);
        double output;
        int shape = getParameter(m_pWaveform).getInt();
        output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period);
        a_outputs.setChannel(0, m_gain * output);
    }
}

