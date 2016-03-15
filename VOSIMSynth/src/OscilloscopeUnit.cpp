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

#include "OscilloscopeUnit.h"
#include <DSPMath.h>

namespace syn {
	OscilloscopeUnit::OscilloscopeUnit(const string& a_name) :
		Unit(a_name),
		m_bufferIndex(0),
		m_nBuffers(2),
		m_pBufferSize(addParameter_(UnitParameter("buffer size", 2, 16384, 256))),
		m_pNumPeriods(addParameter_(UnitParameter("periods", 1, 16, 1))),
		m_lastPhase(0.0),
		m_lastSync(0),
		m_syncCount(0)
	{
		addInput_("in1");
		addInput_("in2");
		m_iPhase = addInput_("ph");
		m_buffers.resize(m_nBuffers);		
		m_bufferSize = getParameter(m_pBufferSize).getInt();
		for (int i = 0; i < m_nBuffers; i++) {
			m_buffers[i].resize(m_bufferSize);
		}
	}

	void OscilloscopeUnit::onParamChange_(int a_paramId) {
		if(a_paramId==m_pBufferSize) {
			int newBufferSize = getParameter(m_pBufferSize).getInt();
			int oldBufferSize = m_bufferSize;
			m_bufferIndex = MIN(m_bufferIndex,newBufferSize);
			m_bufferSize = newBufferSize;
			if (m_bufferSize > oldBufferSize) {
				for (int i = 0; i < m_nBuffers; i++) {
					m_buffers[i].resize(m_bufferSize);
				}
			}
		}
	}

	void OscilloscopeUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		double phase = a_inputs.getValue(m_iPhase);
		if(1 - (m_lastPhase - phase) < 0.5) {
			_sync();
		}
		m_lastSync++;
		m_lastPhase = phase;
		for (int i = 0; i < m_nBuffers; i++) {
			m_buffers[i][m_bufferIndex] = a_inputs.getValue(i);
		}
		m_bufferIndex = WRAP(m_bufferIndex+1, m_bufferSize);
	}

	void OscilloscopeUnit::_sync() {
		m_syncCount++;
		if(m_syncCount>=getParameter(m_pNumPeriods).getInt()) {
			setParameter(m_pBufferSize, m_lastSync);
			m_bufferIndex = 0;
			m_lastSync = 0;
			m_syncCount = 0;
		}
	}
}
