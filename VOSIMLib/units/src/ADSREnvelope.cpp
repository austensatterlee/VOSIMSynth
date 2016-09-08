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
CEREAL_REGISTER_TYPE(syn::ADSREnvelope);

namespace syn
{
	ADSREnvelope::ADSREnvelope(const string& name) :
		Unit(name),
		m_phase(0),
		m_currStage(Attack),
		m_initial(0),
		m_target(1),
		m_isActive(false),
		m_pAttack(addParameter_(UnitParameter("atk", 0.0, 1.0, 0.001))),
		m_pDecay(addParameter_(UnitParameter("dec", 0.0, 1.0, 0.1))),
		m_pSustain(addParameter_(UnitParameter("sus", 0.0, 1.0, 0.707))),
		m_pRelease(addParameter_(UnitParameter("rel", 0.0, 1.0, 0.20))),
		m_pTimeScale(addParameter_(UnitParameter("timescale", 1, 10, 1, UnitParameter::Seconds)))
	{
		m_iGate = addInput_("trig");
		addOutput_("out");
	}

	ADSREnvelope::ADSREnvelope(const ADSREnvelope& a_rhs) :
		ADSREnvelope(a_rhs.name()) {}

	void ADSREnvelope::process_() {
		/* Determine phase increment based on current segment */
		double segment_time;
		switch (m_currStage) {
		case Attack:
			segment_time = getParameter(m_pAttack).getDouble();
			m_initial = 0;
			// skip decay segment if its length is zero
			m_target = getParameter(m_pDecay).getDouble() != 0 ? 1.0 : getParameter(m_pSustain).getDouble();
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
		segment_time = segment_time * getParameter(m_pTimeScale).getInt();
		if (!segment_time) { // for zero second segment time, advance phase pointer to next segment
			m_phase += 1;
		}
		else {
			m_phase += 1.0 / (fs() * segment_time);
		}

		/* Handle segment change */
		
		if (m_phase >= 1.0) {
			if (m_currStage == Attack) {
				m_currStage = Decay;
				m_initial = m_target;
				m_phase = 0.0;
			}
			else if (m_currStage == Decay) {
				m_currStage = Sustain;
				m_phase = 1.0;
			}
			else if (m_currStage == Sustain) {
				m_currStage = Sustain;
				m_phase = 0.0;
			}
			else if (m_currStage == Release) {
				m_isActive = false;
				m_phase = 1.0;
			}
		}

		//double output = LERP(m_initial, m_target, m_phase);
		//@todo: this is just a test. it should be done more cleanly, possible in a new nonlinear unit 
		//double tau = -segment_time / -13.8;
		double shape = 0.3; // std::exp(-1.0 / (tau*getFs()));
		double output = LERP<double>(m_initial,m_target,INVLERP<double>(1,shape,pow(shape, m_phase)));
		setOutputChannel_(0, output);

		if ((!m_isActive || m_currStage == Release) && getInputValue(m_iGate) > 0.5) {
			trigger();
		}
		else if (m_currStage != Release && getInputValue(m_iGate) <= 0.5) {
			release();
		}
	}

	void ADSREnvelope::trigger() {
		m_currStage = Attack;
		m_phase = 0;
		m_isActive = true;
	}

	void ADSREnvelope::release() {
		m_currStage = Release;
		m_initial = getOutputValue(0);
		m_target = 0;
		m_phase = 0;
	}

	bool ADSREnvelope::isActive() const {
		return m_isActive;
	}
}