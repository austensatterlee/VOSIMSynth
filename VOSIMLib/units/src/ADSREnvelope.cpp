#include "ADSREnvelope.h"
#include "DSPMath.h"

namespace syn {


    ADSREnvelope::ADSREnvelope(const string& name) :
            Unit(name),
            m_phase(0),
            m_currStage(Attack),
            m_initial(0),
            m_target(1),
            m_isActive(false),
            m_pAttack(addParameter_(UnitParameter("attack",0.0,1.0,0.01))),
            m_pDecay(addParameter_(UnitParameter("decay",0.0,1.0,0.01))),
            m_pSustain(addParameter_(UnitParameter("sustain",0.0,1.0,1.0))),
            m_pRelease(addParameter_(UnitParameter("release",0.0,1.0,0.01))),
			m_pTimeScale(addParameter_(UnitParameter("timescale",1,10,1)))
    {
		m_iGate = addInput_("gate");
        addOutput_("out");
    }

    ADSREnvelope::ADSREnvelope(const ADSREnvelope& a_rhs) :
            ADSREnvelope(a_rhs.getName())
    {}

    void ADSREnvelope::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
    {
        /* Determine phase increment based on current segment */
        double segment_time;
        switch (m_currStage) {
            case Attack:
                segment_time = getParameter(m_pAttack).getDouble();
                m_initial = 0;
                // skip decay segment if its length is zero
                m_target = getParameter(m_pDecay).getDouble()!=0 ? 1.0 : getParameter(m_pSustain).getDouble();
                break;
            case Decay:
                segment_time = getParameter(m_pDecay).getDouble();
                m_target = getParameter(m_pSustain).getDouble();
                break;
            case Sustain:
                segment_time = 0;
                m_initial = getParameter(m_pSustain).getDouble();
                m_target = getParameter(m_pSustain).getDouble();
                break;
            case Release:
                segment_time = getParameter(m_pRelease).getDouble();
                break;
            default:
                throw std::logic_error("Invalid envelope stage");
        }
		segment_time = segment_time*getParameter(m_pTimeScale).getInt();
        if (!segment_time) { // for zero second segment time, advance phase pointer to next segment
            m_phase += 1;
        } else {
            m_phase += 1.0 / (getFs() * segment_time);
        }


        /* Handle segment change */
        if (m_phase >= 1.0) {
            if (m_currStage == Attack) {
                m_currStage = Decay;
                m_initial = m_target;
                m_phase = 0.0;
            } else if (m_currStage == Decay) {
                m_currStage = Sustain;
                m_phase = 1.0;
            } else if (m_currStage == Sustain) {
                m_currStage = Sustain;
                m_phase = 0.0;
            } else if (m_currStage == Release) {
                m_isActive = false;
                m_phase = 1.0;
            }
        }
        double output = LERP(m_initial, m_target, m_phase);
        a_outputs.setChannel(0,output);

		if ((!m_isActive || m_currStage == Release) && a_inputs.getValue(m_iGate) > 0.0) {
			trigger();
		}
		else if (m_currStage != Release && a_inputs.getValue(m_iGate)<=0.0) {
			release();
		}
    }

    void ADSREnvelope::trigger()
    {
        m_currStage = Attack;
        m_phase = 0;
        m_isActive = true;
    }

    void ADSREnvelope::release()
    {
        m_currStage = Release;
        m_initial = getOutputChannel(0).get();
        m_target = 0;
        m_phase = 0;
    }

    bool ADSREnvelope::isActive() const
    {
        return m_isActive;
    }
}

