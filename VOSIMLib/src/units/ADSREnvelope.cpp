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

#include "units/ADSREnvelope.h"
#include "DSPMath.h"

namespace syn {
    ADSREnvelope::ADSREnvelope(const string& name) :
        Unit(name),
        m_currStage(Off),
        m_phase(0),
        m_currInitial(0),
        m_currTarget(1) ,
        m_lastOutput(0),
        m_lastGate(0)
    {
        addInput_(iAttack, "atk");
        addInput_(iDecay, "dec");
        addInput_(iSustain, "sus");
        addInput_(iRelease, "rel");
        addInput_(iGate, "trig");
        addParameter_(pAttack, UnitParameter("atk", 0.0, 1.0, 0.001));
        addParameter_(pDecay, UnitParameter("dec", 0.0, 1.0, 0.1));
        addParameter_(pSustain, UnitParameter("sus", 0.0, 1.0, 0.707));
        addParameter_(pRelease, UnitParameter("rel", 0.0, 1.0, 0.20));
        addParameter_(pTimescale, UnitParameter("timescale", 1, 10, 1, UnitParameter::Seconds));
        addOutput_("out");
    }

    ADSREnvelope::ADSREnvelope(const ADSREnvelope& a_rhs) :
        ADSREnvelope(a_rhs.name()) {}

    void ADSREnvelope::process_() {
        BEGIN_PROC_FUNC
        /* Determine phase increment based on current segment */
        double segment_time;
        switch (m_currStage) {
        case Attack:
            segment_time = param(pAttack).getDouble() + READ_INPUT(iAttack);
            m_currTarget = 1.0;
            break;
        case Decay:
            segment_time = param(pDecay).getDouble() + READ_INPUT(iDecay);
            m_currTarget = param(pSustain).getDouble() + READ_INPUT(iSustain);
            break;
        case Sustain:
            segment_time = 2./fs();
            m_currInitial = m_lastOutput;
            m_currTarget = param(pSustain).getDouble() + READ_INPUT(iSustain);
            break;
        case Release:
            // Multiply the segment length by the actual distance the release stage with cover.
            segment_time = (param(pRelease).getDouble() + READ_INPUT(iRelease))*m_currInitial;
            m_currTarget = 0;
            break;
        case Off:
        default:
            segment_time=-1;
            break;
        }

        /* Calculate output */
        double output = 0;
        if (segment_time >= 0) {
            segment_time = segment_time * param(pTimescale).getInt();
            double phase_step = segment_time > 0.0 ? 1./(fs()*segment_time) : 1.0;
            m_phase += phase_step;          
            /* Compute output */
            //double output = LERP(m_initial, m_target, m_phase);
            //@todo: this is just a test. it should be done more cleanly, possible in a new nonlinear unit
            //double tau = -segment_time / -13.8;
            double shape = 0.3; // std::exp(-1.0 / (tau*getFs()));
            output = LERP<double>(m_currInitial, m_currTarget, INVLERP<double>(1, shape, pow(shape, m_phase)));        
        }
 
        WRITE_OUTPUT(0, output);
        m_lastOutput = output;
        
        /* Detect rising and falling gate edges */
        bool rising = isGateRising();
        bool falling = isGateFalling();
        m_lastGate = READ_INPUT(iGate);

        /* Handle transitions caused by external signals (risingEdge, fallingEdge) */
        if (rising) {
            trigger();
        }
        if (falling) {
            switch (m_currStage) {
            case Decay:
            case Sustain:
            case Attack:
                release(output);
                break;
            case Off:
            case Release:
            default:
                break;
            }
        }

        /* Handle transitions caused by an internal signal (phase reset) */
        if (m_phase >= 1.0) {
            m_phase = 0;
            switch (m_currStage) {
            case Attack:
                m_currStage = Decay;
                m_currInitial = m_lastOutput;
                break;
            case Decay:
                m_currStage = Sustain;
                m_currInitial = m_lastOutput;
                break;
            case Release:
                reset();
                break;
            case Off:
            case Sustain:
            default:
                break;
            }
        }
        END_PROC_FUNC
    }

    bool ADSREnvelope::isGateRising() const { return (READ_INPUT(iGate) - m_lastGate) > 0.5; }

    bool ADSREnvelope::isGateFalling() const { return (READ_INPUT(iGate) - m_lastGate) < -0.5; }

    void ADSREnvelope::onNoteOn_() {
        if (!isConnected(iGate)) {
            trigger();
        } else if (isGateRising()) {
            trigger();
        }
    }

    void ADSREnvelope::onNoteOff_() {
        if (!isConnected(iGate)) {
            release(m_lastOutput);
        } else if (isGateFalling()) {
            release(m_lastOutput);
        }
    }

    void ADSREnvelope::trigger() {
        m_currStage = Attack;
        m_phase = 0;
        m_currInitial = 0.0;
        m_currTarget = 1.0;
    }

    void ADSREnvelope::release(double a_releaseValue) {
        m_currStage = Release;
        m_currInitial = a_releaseValue;
        m_phase = 0;
    }

    void ADSREnvelope::reset() {
        m_currStage = Off;
        m_phase = 0;
        m_lastGate = 0;
        m_lastOutput = 0;
    }

    bool ADSREnvelope::isActive() const {
        return m_currStage != Off;
    }
}
