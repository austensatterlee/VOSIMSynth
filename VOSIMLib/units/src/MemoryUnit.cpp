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

#include "MemoryUnit.h"
#include "DSPMath.h"

using namespace std;

namespace syn
{
	double NSampleDelay::getPastSample(int a_offset) {
		int bufferReadIndex = WRAP<int>(m_bufferIndex - a_offset, m_bufferSize);
		return m_buffer[bufferReadIndex];
	}

	void NSampleDelay::resizeBuffer(int a_newBufSize) {
		m_buffer.resize(a_newBufSize);
		m_bufferSize = m_buffer.size();
		m_bufferIndex = MIN(m_bufferIndex, m_bufferSize - 1);
	}

	void NSampleDelay::clearBuffer() {
		fill(&m_buffer.front(), &m_buffer.back(), 0.0);
	}

	int NSampleDelay::size() const {
		return m_buffer.size();
	}

	MemoryUnit::MemoryUnit(const string& a_name):
		Unit(a_name),
		m_pBufSize(addParameter_(UnitParameter("samples", 1, 16384, 1))) {
		addInput_("in");
		addOutput_("out");
	}

	MemoryUnit::MemoryUnit(const MemoryUnit& a_rhs):
		MemoryUnit(a_rhs.getName()) {}

	string MemoryUnit::_getClassName() const {
		return "MemoryUnit";
	}

	Unit* MemoryUnit::_clone() const {
		return new MemoryUnit(*this);
	}

	void MemoryUnit::onParamChange_(int a_paramId) {
		if (a_paramId == m_pBufSize) {
			int newBufSize = getParameter(m_pBufSize).getInt();
			m_delay.resizeBuffer(newBufSize);
			if (newBufSize == 1) {
				m_delay.clearBuffer();
			}
		}
	}

	void MemoryUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		double input = a_inputs.getValue(0);
		double output = m_delay.process(input);
		a_outputs.setChannel(0, output);
	}


	double NSampleDelay::process(double a_input) {
		if (isnan(m_buffer[m_bufferIndex]) || isinf(m_buffer[m_bufferIndex])) {
			m_buffer[m_bufferIndex] = 0.0;
		}

		m_bufferIndex++;
		if (m_bufferIndex >= m_bufferSize) {
			m_bufferIndex = 0;
		}

		double output = m_buffer[m_bufferIndex];

		int bufferWriteIndex = WRAP<int>(m_bufferIndex - m_bufferSize, m_bufferSize);
		m_buffer[bufferWriteIndex] = a_input;

		return output;
	}
}
