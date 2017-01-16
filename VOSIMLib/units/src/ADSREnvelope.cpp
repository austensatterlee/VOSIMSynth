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

#include "ADSREnvelope.h"
#include "DSPMath.h"

#include "common.h"

namespace syn
{
    ADSREnvelope::ADSREnvelope(const string& name) :
        Unit(name),
        m_phase(0),
        m_currStage(Attack),
        m_initial(0),
        m_target(1),
        m_isActive(false)        
    {
        addInput_(Input::iAttack, "atk");
        addInput_(Input::iDecay, "dec");
        addInput_(Input::iSustain, "sus");
        addInput_(Input::iRelease, "rel");
        addInput_(Input::iGate, "trig");
        addParameter_(Param::pAttack, UnitParameter("atk", 0.0, 1.0, 0.001));
        addParameter_(Param::pDecay, UnitParameter("dec", 0.0, 1.0, 0.1));
        addParameter_(Param::pSustain, UnitParameter("sus", 0.0, 1.0, 0.707));
        addParameter_(Param::pRelease, UnitParameter("rel", 0.0, 1.0, 0.20));
        addParameter_(Param::pTimescale, UnitParameter("timescale", 1, 10, 1, UnitParameter::Seconds));
        addOutput_("out");
    }

    ADSREnvelope::ADSREnvelope(const ADSREnvelope& a_rhs) :
        ADSREnvelope(a_rhs.name()) {}

    void ADSREnvelope::process_()
    {
        BEGIN_PROC_FUNC
            /* Determine phase increment based on current segment */
            double segment_time;
            switch (m_currStage)
            {
            case Attack:
                segment_time = param(pAttack).getDouble() + READ_INPUT(iAttack);
                m_initial = 0;
                // skip decay segment if its length is zero
                m_target = param(pDecay).getDouble() + READ_INPUT(iDecay) != 0 ? 1.0 : param(pSustain).getDouble() + READ_INPUT(iSustain);
                break;
            case Decay:
                segment_time = param(pDecay).getDouble() + READ_INPUT(iDecay);
                m_target = param(pSustain).getDouble() + READ_INPUT(iSustain);
                break;
            case Sustain:
                segment_time = 0;
                m_initial = param(pSustain).getDouble() + READ_INPUT(iSustain);
                m_target = m_initial;
                break;
            case Release:
                segment_time = param(pRelease).getDouble() + READ_INPUT(iRelease);
                break;
            default:
                throw std::logic_error("Invalid envelope stage");
            }
            segment_time = segment_time * param(pTimescale).getInt();
            if (!segment_time)
            { // for zero second segment time, advance phase pointer to next segment
                m_phase += 1;
            }
            else
            {
                m_phase += 1.0 / (fs() * segment_time);
            }

            /* Handle segment change */

            if (m_phase >= 1.0)
            {
                if (m_currStage == Attack)
                {
                    m_currStage = Decay;
                    m_initial = m_target;
                    m_phase = 0.0;
                }
                else if (m_currStage == Decay)
                {
                    m_currStage = Sustain;
                    m_phase = 1.0;
                }
                else if (m_currStage == Sustain)
                {
                    m_currStage = Sustain;
                    m_phase = 0.0;
                }
                else if (m_currStage == Release)
                {
                    m_isActive = false;
                    m_phase = 1.0;
                }
            }

            //double output = LERP(m_initial, m_target, m_phase);
            //@todo: this is just a test. it should be done more cleanly, possible in a new nonlinear unit
            //double tau = -segment_time / -13.8;
            double shape = 0.3; // std::exp(-1.0 / (tau*getFs()));
            double output = LERP<double>(m_initial, m_target, INVLERP<double>(1, shape, pow(shape, m_phase)));
            WRITE_OUTPUT(0, output);

            if ((!m_isActive || m_currStage == Release) && READ_INPUT(iGate) > 0.5)
            {
                trigger();
            }
            else if (m_currStage != Release && READ_INPUT(iGate) <= 0.5)
            {
                release(READ_OUTPUT(0));
            }
        END_PROC_FUNC
    }

    void ADSREnvelope::trigger()
    {
        m_currStage = Attack;
        m_phase = 0;
        m_isActive = true;
    }

    void ADSREnvelope::release(double a_releaseValue)
    {
        m_currStage = Release;
        m_initial = a_releaseValue;
        m_target = 0;
        m_phase = 0;
    }

    void ADSREnvelope::reset()
    {
        trigger();
        m_isActive=false;
    }

    bool ADSREnvelope::isActive() const
    {
        return m_isActive;
    }
}
