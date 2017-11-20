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
#include "vosimlib/units/OscillatorUnit.h"
#include "vosimlib/DSPMath.h"
#include "vosimlib/tables.h"

namespace syn
{
    OscillatorUnit::OscillatorUnit(const string& a_name) :
        Unit(a_name),
        m_basePhase(0),
        m_phase(0),
        m_last_phase(0),
        m_phase_step(0),
        m_period(1),
        m_freq(0.0),
        m_gain(0.0),
        m_bias(0.0),
        m_lastSync(0.0)
    {
        addOutput_(oOut, "out");
        addOutput_(oPhase, "ph");
        addParameter_(pGain, {"gain", 0.0, 1.0, 1.0});
        addParameter_(pPhaseOffset, {"phase", 0.0, 1.0, 0.0});
        addParameter_(pUnipolar, UnitParameter("unipolar", false));
        addInput_(iGainMul, "g[x]", 1.0);
        addInput_(iPhaseAdd, "ph");
        addInput_(iSync, "sync");
    }

    void OscillatorUnit::reset()
    {
        m_basePhase = 0.0;
    }

    void OscillatorUnit::updatePhaseStep_()
    {
        if (m_freq)
        {
            m_period = fs() / m_freq;
            m_phase_step = 1. / m_period;
        }
        else
        {
            m_period = 0.0;
            m_phase_step = 0.0;
        }
    }

    void OscillatorUnit::tickPhase_(double a_phaseOffset)
    {        
        // sync
        if (m_lastSync > 0.5 && READ_INPUT(iSync) < 0.5)
        {
            reset();
        }
        m_lastSync = READ_INPUT(iSync);

        m_basePhase += m_phase_step;
        if (m_basePhase >= 1)
        {
            m_basePhase -= 1;
        }
        m_phase = m_basePhase + a_phaseOffset;
        m_phase = WRAP(m_phase, 1.0);
        m_last_phase = m_phase;
    }

    void OscillatorUnit::process_()
    {
        double phase_offset = param(pPhaseOffset).getDouble() + READ_INPUT(iPhaseAdd);
        m_gain = param(pGain).getDouble() * READ_INPUT(iGainMul);
        m_bias = 0;
        if (param(pUnipolar).getBool())
        {
            // make signal unipolar
            m_gain *= 0.5;
            m_bias = m_gain;
        }
        updatePhaseStep_();
        tickPhase_(phase_offset);

        WRITE_OUTPUT(oPhase, m_phase);
    }

    void TunedOscillatorUnit::onNoteOn_()
    {
        reset();
    }

    void TunedOscillatorUnit::updatePhaseStep_()
    {
        m_freq = pitchToFreq(m_pitch);
        OscillatorUnit::updatePhaseStep_();
    }

    TunedOscillatorUnit::TunedOscillatorUnit(const string& a_name) :
        OscillatorUnit(a_name),
        m_pitch(0)
    {
        addParameter_(pTune, {"semi", -12.0, 12.0, 0.0});
        addParameter_(pOctave, {"oct", -3, 3, 0});
        addInput_(iNote, "pitch");
    }

    TunedOscillatorUnit::TunedOscillatorUnit(const TunedOscillatorUnit& a_rhs) :
        TunedOscillatorUnit(a_rhs.name()) {}

    void TunedOscillatorUnit::process_()
    {
        double tune = param(pTune).getDouble() + READ_INPUT(iNote);
        double oct = param(pOctave).getInt();
        m_pitch = tune + oct * 12;
        OscillatorUnit::process_();
    }

    BasicOscillatorUnit::BasicOscillatorUnit(const string& a_name) :
        TunedOscillatorUnit(a_name)
    {
        addParameter_(pWaveform, UnitParameter("waveform", WAVE_SHAPE_NAMES));
    }

    BasicOscillatorUnit::BasicOscillatorUnit(const BasicOscillatorUnit& a_rhs) : BasicOscillatorUnit(a_rhs.name()) {}

    void BasicOscillatorUnit::process_()
    {
        BEGIN_PROC_FUNC
        TunedOscillatorUnit::process_();
        double output;
        WAVE_SHAPE shape = static_cast<WAVE_SHAPE>(param(pWaveform).getInt());
        switch (shape)
        {
            case SAW_WAVE:
                output = lut_bl_saw_table().getresampled(m_phase, m_period);
                break;
            case SINE_WAVE:
                output = lut_sin_table().plerp(m_phase);
                break;
            case TRI_WAVE:
                output = lut_bl_tri_table().getresampled(m_phase, m_period);
                break;
            default:
            case SQUARE_WAVE:
                output = lut_bl_square_table().getresampled(m_phase, m_period);
                break;
        }
        WRITE_OUTPUT(oOut, m_gain * output + m_bias);
        END_PROC_FUNC
    }

    LFOOscillatorUnit::LFOOscillatorUnit(const string& a_name) :
        OscillatorUnit(a_name)
    {
        addOutput_(oQuadOut, "quad");
        addInput_(iFreqAdd, "freq");
        addInput_(iFreqMul, "freq[x]", 1.0);
        param(pGain).setMax(1e6);
        param(pGain).setControlType(UnitParameter::Unbounded);
        addParameter_(pBPMFreq, UnitParameter("rate", g_bpmStrs, g_bpmVals, 0, UnitParameter::EUnitsType::BPM).setVisible(false));
        addParameter_(pFreq, UnitParameter("freq", 0.01, 100.0, 1.0, UnitParameter::EUnitsType::Freq));
        addParameter_(pWaveform, UnitParameter("waveform", WAVE_SHAPE_NAMES));
        addParameter_(pTempoSync, UnitParameter("tempo sync", false));
    }

    void LFOOscillatorUnit::process_()
    {
        BEGIN_PROC_FUNC
        // determine frequency
        if (param(pTempoSync).getBool())
        {
            m_freq = bpmToFreq(param(pBPMFreq).getEnum(READ_INPUT(iFreqMul) * (param(pBPMFreq).getInt() + READ_INPUT(iFreqAdd))), tempo());
        }
        else
        {
            m_freq = READ_INPUT(iFreqMul) * (param(pFreq).getDouble() + READ_INPUT(iFreqAdd));
        }
        OscillatorUnit::process_();
        double output, quadoutput;
        WAVE_SHAPE shape = static_cast<WAVE_SHAPE>(param(pWaveform).getInt());
        switch (shape)
        {
            case SAW_WAVE:
                output = naive_saw(m_phase);
                quadoutput = naive_saw(m_phase + 0.25);
                break;
            case SINE_WAVE:
                output = lut_sin_table().plerp(m_phase);
                quadoutput = lut_sin_table().plerp(m_phase + 0.25);
                break;
            case TRI_WAVE:
                output = naive_tri(m_phase);
                quadoutput = naive_tri(m_phase + 0.25);
                break;
            default:
            case SQUARE_WAVE:
                output = naive_square(m_phase);
                quadoutput = naive_square(m_phase + 0.25);
                break;
        }
        WRITE_OUTPUT(oOut, m_gain * output + m_bias);
        WRITE_OUTPUT(oQuadOut, m_gain * quadoutput + m_bias);
        END_PROC_FUNC
    }

    void LFOOscillatorUnit::onParamChange_(int a_paramId)
    {
        if (a_paramId == pTempoSync)
        {
            bool useTempoSync = param(pTempoSync).getBool();
            param(pFreq).setVisible(!useTempoSync);
            param(pBPMFreq).setVisible(useTempoSync);
        }
    }
}
