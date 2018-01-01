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

/**
 * \file MemoryUnit.cpp
 * \brief
 * \details
 * \author Austen Satterlee
 * \date March 6, 2016
 */
#include "vosimlib/units/MemoryUnit.h"
#include "vosimlib/DSPMath.h"

namespace syn
{
    //---------------------
    // NSampleDelay
    //---------------------
    NSampleDelay::NSampleDelay() :
        m_buffer(1),
        m_arraySize(1),
        m_delaySamples(1.0),
        m_curWritePhase(0),
        m_lastOutput(0.0)
    {
        resizeBuffer(1);
    }

    double NSampleDelay::getLastOutput() const
    {
        return m_lastOutput;
    };

    double NSampleDelay::readTap(double a_offset)
    {
        double readPhase = WRAP<double>(m_curWritePhase - a_offset, m_arraySize);
        int rInd1 = WRAP<int>(readPhase, m_arraySize);
        int rInd2 = WRAP<int>(rInd1 + 1, m_arraySize);
        return LERP<double>(m_buffer[rInd1], m_buffer[rInd2], readPhase - rInd1);
    }

    void NSampleDelay::resizeBuffer(double a_delaySamples)
    {
        if (m_delaySamples == a_delaySamples || a_delaySamples <= 0)
            return;
        int requiredBufSize = ceil(a_delaySamples);
        if (requiredBufSize > m_buffer.size())
            m_buffer.resize(requiredBufSize);
        m_arraySize = static_cast<int>(m_buffer.size());
        m_delaySamples = a_delaySamples;
    }

    void NSampleDelay::reset()
    {
        std::fill(&m_buffer.front(), &m_buffer.back(), 0.0);
        m_curWritePhase = 0.0;
        m_lastOutput = 0.0;
    }

    int NSampleDelay::size() const
    {
        return static_cast<int>(m_buffer.size());
    }

    double NSampleDelay::process(double a_input)
    {
        // Read
        m_lastOutput = readTap(m_delaySamples);

        // Write
        silentProcess(a_input);
        return m_lastOutput;
    }

    void NSampleDelay::silentProcess(double a_input)
    {
        m_buffer[static_cast<int>(m_curWritePhase)] = a_input;
        m_curWritePhase = WRAP<double>(m_curWritePhase + 1.0, m_arraySize);
    }

    //---------------------
    // MemoryUnit
    //---------------------
    MemoryUnit::MemoryUnit(const string& a_name) :
        Unit(a_name)
    {
        addInput_("in");
        addOutput_("out");
        m_delay.resizeBuffer(1.0);
        MemoryUnit::reset();
    }

    MemoryUnit::MemoryUnit(const MemoryUnit& a_rhs) :
        MemoryUnit(a_rhs.name()) { }

    void MemoryUnit::reset() { m_delay.reset(); }

    void MemoryUnit::onParamChange_(int a_paramId) { }

    void MemoryUnit::process_()
    {
        BEGIN_PROC_FUNC
        WRITE_OUTPUT(0, m_delay.process(READ_INPUT(0)));
        END_PROC_FUNC
    }

    void MemoryUnit::onNoteOn_()
    {
        reset();
    }

    VariableMemoryUnit::VariableMemoryUnit(const string& a_name) :
        Unit(a_name),
        m_delaySamples(0),
        m_lastOutput(0)
    {
        addInput_(iIn, "in");
        addInput_(iReceive, "recv");
        addInput_(iSizeMod, "delay");
        addOutput_(oOut, "out");
        addOutput_(oSend, "send");
        addParameter_(pBufDelay, UnitParameter("time", 0.00005, 1.0, 0.00005, UnitParameter::EUnitsType::Seconds, 4));
        addParameter_(pBufFreq, UnitParameter("freq", 1.0, 20000.0, 20000.0, UnitParameter::EUnitsType::Freq).setVisible(false));
        addParameter_(pBufBPMFreq, UnitParameter("rate", g_bpmStrs, g_bpmVals, 0, UnitParameter::EUnitsType::BPM).setVisible(false));
        addParameter_(pBufSamples, UnitParameter("samples", 1.0, 48000.0, 1.0, UnitParameter::EUnitsType::Samples).setVisible(false));
        addParameter_(pBufType, UnitParameter("units", {"sec","Hz","BPM","samples"}, {pBufDelay, pBufFreq, pBufBPMFreq, pBufSamples}));
        addParameter_(pDryGain, UnitParameter("dry", 0.0, 1.0, 0.0));
        addParameter_(pWetGain, UnitParameter("wet", 0.0, 1.0, 1.0));
        m_delay.resizeBuffer(48000);
    }

    VariableMemoryUnit::VariableMemoryUnit(const VariableMemoryUnit& a_rhs) :
        VariableMemoryUnit(a_rhs.name()) { }

    void VariableMemoryUnit::reset()
    {
        m_delay.reset();
        m_lastOutput = 0.0;
    }

    void VariableMemoryUnit::onParamChange_(int a_paramId)
    {
        switch (a_paramId)
        {
            case pBufType:
            {
                int newtype = param(pBufType).getInt();
                param(pBufDelay).setVisible(newtype == 0);
                param(pBufFreq).setVisible(newtype == 1);
                param(pBufBPMFreq).setVisible(newtype == 2);
                param(pBufSamples).setVisible(newtype == 3);
                m_lastOutput = 0.0;
                break;
            }
            default:
                break;
        }
    }

    void VariableMemoryUnit::process_()
    {
        BEGIN_PROC_FUNC
        int bufType = param(pBufType).getEnum();
        switch (bufType)
        {
            case pBufDelay:
                m_delaySamples = periodToSamples(param(pBufDelay).getDouble() + READ_INPUT(iSizeMod), fs());
                break;
            case pBufFreq:
                m_delaySamples = freqToSamples(param(pBufFreq).getDouble() + READ_INPUT(iSizeMod), fs());
                break;
            case pBufBPMFreq:
                m_delaySamples = freqToSamples(bpmToFreq(param(pBufBPMFreq).getEnum(param(pBufBPMFreq).getInt() + READ_INPUT(iSizeMod)), tempo()), fs());
                break;
            case pBufSamples:
                m_delaySamples = param(pBufSamples).getDouble() + READ_INPUT(iSizeMod);
                break;
            default:
                break;
        }
        m_delay.resizeBuffer(m_delaySamples);
        double input = READ_INPUT(iIn);
        double receive = READ_INPUT(iReceive);
        double output;
        output = m_delay.process(input + receive);        
        m_lastOutput = output;

        double dryMix = input*param(pDryGain).getDouble();
        double wetMix = output*param(pWetGain).getDouble();
        WRITE_OUTPUT(oOut, wetMix + dryMix);
        WRITE_OUTPUT(oSend, output);
        END_PROC_FUNC
    }

    void VariableMemoryUnit::onNoteOn_()
    {
        reset();
    }

    SampleAndHoldUnit::SampleAndHoldUnit(const string& a_name)
        : Unit(a_name),
          m_currValue(0),
          m_phase(0.0),
          m_step(0.0),
          m_lastSync(0.0) {
        addInput_(iIn, "in");
        addInput_(iSync, "sync");
        addParameter_(pFreq, UnitParameter{"freq", 0.0, 20e3, 0.0, UnitParameter::Freq});
        addOutput_(0, "out");
    }

    SampleAndHoldUnit::SampleAndHoldUnit(const SampleAndHoldUnit& a_rhs)
        : SampleAndHoldUnit(a_rhs.name()) {}

    void SampleAndHoldUnit::reset() {
        m_phase = 0.0;
        m_currValue = 0.0;
    }

    void SampleAndHoldUnit::process_() {
        BEGIN_PROC_FUNC
        m_phase = syn::WRAP(m_phase + m_step, 1.0);
        // sync
        if (m_phase < m_step) {
            m_currValue = READ_INPUT(iIn);
        }
        double trig = READ_INPUT(iSync);
        if (m_lastSync - trig > 0.5) {
            reset();
            m_currValue = READ_INPUT(iIn);
        }
        m_lastSync = READ_INPUT(iSync);
        WRITE_OUTPUT(0, m_currValue);
        END_PROC_FUNC
    }

    void SampleAndHoldUnit::onParamChange_(int a_paramId) {
        switch (a_paramId) {
        case pFreq:
        {
            m_step = param(pFreq).getDouble() / fs();
        }
            break;
        default:
            break;
        }
    }
}
