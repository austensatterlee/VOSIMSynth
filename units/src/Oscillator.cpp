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

    void Oscillator::update_step()
    {
        m_freq = pitchToFreq(m_pitch);
        m_period = getFs() / m_freq;
        m_phase_step = 1. / m_period;
    }

    void Oscillator::tick_phase()
    {
        m_basePhase += m_phase_step;
        if (m_basePhase >= 1)
            m_basePhase -= 1;
        m_phase = m_basePhase;
        if (m_phase >= 1)
            m_phase -= 1;
        else if (m_phase < 0)
            m_phase += 1;
    }

    void BasicOscillator::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
    {
        double tune = getParameter(m_pTune).getDouble() + a_inputs.getValue(1);
        double oct = getParameter(m_pOctave).getInt();
        double gain = getParameter(m_pGain).getDouble() * a_inputs.getValue(0);
        m_pitch = tune + oct * 12;

        update_step();
        tick_phase();

        double output;
        int shape = getParameter(m_pWaveform).getInt();
        output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period);
        a_outputs.setChannel(0, gain * output);
    }
}

