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
#include "vosimlib/units/ADSREnvelope.h"
#include "vosimlib/DSPMath.h"

namespace syn {
    ADSREnvelope::ADSREnvelope(const string& a_name) :
        Unit(a_name),
        m_currState(Off),
        m_lastOutput(0),
        m_lastGate(false)
    {
        addInput_(iGate, "gate");
        addParameter_(pAtkTime, UnitParameter("atk", 0.0, 1.0, 0.010));
        addParameter_(pDecTime, UnitParameter("dec", 0.0, 1.0, 0.100));
        addParameter_(pSustain, UnitParameter("sus", 0.0, 1.0, 0.707));
        addParameter_(pRelTime, UnitParameter("rel", 0.0, 1.0, 0.200));
        addParameter_(pTimescale, UnitParameter("timescale", 1, 10, 1, UnitParameter::Seconds));
        addParameter_(pUpShape, UnitParameter("rise", 0.001, 1.0, 0.22));
        addParameter_(pDownShape, UnitParameter("fall", 0.001, 1.0, 0.007));
        addParameter_(pLegato, UnitParameter("legato", false));
        addOutput_("out");
    }

    ADSREnvelope::ADSREnvelope(const ADSREnvelope& a_rhs) :
        ADSREnvelope(a_rhs.name()) {}

    void ADSREnvelope::process_() {
        BEGIN_PROC_FUNC

        double output = 0.0;
        switch(m_currState) {
        case Off:
            break;
        case Attack:
            output = m_atkBias + m_lastOutput * m_atkFb;
            if(output>=1.0) {
                output = 1.0;
                m_currState = Decay;
            }
            break;
        case Decay:
            output = m_decBias + m_lastOutput * m_decFb;
            if (output <= m_sustain) {
                output = m_sustain;
                m_currState = Sustain;
            }
            break;
        case Sustain:
            if (m_lastOutput > m_sustain)
                output = m_decBias + m_lastOutput * m_decFb;
            else
                output = m_sustain;
            break;
        case Release:
            output = m_relBias + m_lastOutput * m_relFb;
            if (output <= 0.0) {
                output = 0.0;
                m_currState = Off;
            }
            break;
        case Retrigger:
            output = -1.376e-2 + m_lastOutput * 0.9862;
            if (output <= 0.0) {
                output = 0.0;
                m_currState = Attack;
            }
            break;
        }
 
        m_lastOutput = output;
        WRITE_OUTPUT(0, output);
        
        /* Handle transitions caused by external gate */
        if (isInputConnected(iGate)) {
            bool gate = READ_INPUT(iGate) > 0.5;
            if (gate && !m_lastGate) {
                m_currState = Attack;
            } else if (!gate && m_lastGate) {
                m_currState = Release;
            }
            m_lastGate = gate;
        }

        END_PROC_FUNC
    }

    void ADSREnvelope::onNoteOn_() {
        if (!isInputConnected(iGate)) {
            if (m_currState != Off && !m_legato)
                m_currState = Retrigger;
            else if (m_currState == Off || m_currState == Release)
                m_currState = Attack;
        }
    }

    void ADSREnvelope::onNoteOff_() {
        if (!isInputConnected(iGate)) {
            m_currState = Release;
        }
    }

    void ADSREnvelope::onParamChange_(int a_paramId) {
        auto value = param(a_paramId).get<double>();
        switch(static_cast<Param>(a_paramId)) {
        case pAtkTime:
            setAttackTime_(value);
            break;
        case pDecTime:
            setDecayTime_(value);
            break;
        case pSustain:
            m_sustain = value;
            setDecayTime_(m_decTime);
            break;
        case pRelTime:
            setReleaseTime_(value);
            break;
        case pUpShape:
            setTargetRatioUp_(value);
            setAttackTime_(m_atkTime);
            break;
        case pDownShape:
            setTargetRatioDown_(value);
            setDecayTime_(m_decTime);
            setReleaseTime_(m_relTime);
            break;
        case pTimescale:
            m_timeScale = value;
            setAttackTime_(m_atkTime);
            setDecayTime_(m_decTime);
            setReleaseTime_(m_relTime);
            break;
        case pLegato:
            m_legato = param(a_paramId).get<bool>();
            break;
        }
    }

    void ADSREnvelope::onFsChange_() {
        setAttackTime_(m_atkTime);
        setDecayTime_(m_decTime);
        setReleaseTime_(m_relTime);
    }

    void ADSREnvelope::setAttackTime_(double a_time) {
        m_atkTime = a_time;
        m_atkFb = calcFb_(a_time, m_targetRatioUp);
        m_atkBias = (1.0 + m_targetRatioUp) * (1.0 - m_atkFb);
    }

    void ADSREnvelope::setDecayTime_(double a_time) {
        m_decTime = a_time;
        m_decFb = calcFb_(a_time, m_targetRatioDown);
        m_decBias = (m_sustain - m_targetRatioDown) * (1.0 - m_decFb);
    }
    void ADSREnvelope::setReleaseTime_(double a_time) {
        m_relTime = a_time;
        m_relFb = calcFb_(a_time, m_targetRatioDown);
        m_relBias = -m_targetRatioDown * (1.0 - m_relFb);
    }
    void ADSREnvelope::setTargetRatioUp_(double a_targetRatio) {
        if (a_targetRatio < 0.000000001)
            a_targetRatio = 0.000000001;  // -180 dB
        m_targetRatioUp = a_targetRatio;
    }
    void ADSREnvelope::setTargetRatioDown_(double a_targetRatio) {
        if (a_targetRatio < 0.000000001)
            a_targetRatio = 0.000000001;  // -180 dB
        m_targetRatioDown = a_targetRatio;
    }

    double ADSREnvelope::calcFb_(double a_time, double a_targetRatio) {
        // Original calc: 
        return a_time <= 0 ? 0.0 : exp(-log((1.0 + a_targetRatio) / a_targetRatio) / (a_time * fs() * m_timeScale));
        // Alternative calc:
        // return (a_time <= 0) ? 0.0 : pow(a_targetRatio / (a_targetRatio + 1.), 1. / (a_time * fs() * m_timeScale));
    }

    void ADSREnvelope::reset() {
        m_currState = Off;
        m_lastOutput = 0;
        m_lastGate = false;
    }

    bool ADSREnvelope::isActive() const {
        return m_currState != Off;
    }
}
