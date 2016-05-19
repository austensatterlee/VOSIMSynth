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
		if (m_bufferSize == a_newBufSize)
			return;
		if(a_newBufSize>m_buffer.size())
			m_buffer.resize(a_newBufSize);
		m_bufferSize = a_newBufSize;
		m_bufferIndex = MIN(m_bufferIndex, m_bufferSize - 1);
	}

	void NSampleDelay::clearBuffer() {
		fill(&m_buffer.front(), &m_buffer.back(), 0.0);
	}

	int NSampleDelay::size() const {
		return m_buffer.size();
	}

	MemoryUnit::MemoryUnit(const string& a_name):
		Unit(a_name)
	{
		m_delay.resizeBuffer(16384);

		addInput_("in");
		m_iSize = addInput_("size");
		addOutput_("out");
		m_pBufSize = addParameter_({ "samples", 1, 16384, 1 });
		m_pBufDelay = addParameter_({ "delay", 0.001, 1.0, 0.001, UnitParameter::EUnitsType::Seconds});
		m_pBufFreq = addParameter_({ "freq", 1.0, 10000.0, 1.0, UnitParameter::EUnitsType::Freq });
		m_pBufBPMFreq = addParameter_({ "rate", g_bpmStrs, g_bpmVals, 0, UnitParameter::EUnitsType::BPM });
		getParameter_(m_pBufDelay).setVisible(false);
		getParameter_(m_pBufFreq).setVisible(false);
		getParameter_(m_pBufBPMFreq).setVisible(false);
		m_pBufType = addParameter_(UnitParameter{ "units",{"samples","seconds","Hz","BPM"} });
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
		if(a_paramId == m_pBufType)
		{
			int newtype = getParameter(m_pBufType).getInt();
			getParameter_(m_pBufSize).setVisible(newtype==0);
			getParameter_(m_pBufDelay).setVisible(newtype==1);
			getParameter_(m_pBufFreq).setVisible(newtype==2);
			getParameter_(m_pBufBPMFreq).setVisible(newtype==3);
		}
	}

	void MemoryUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		int buftype = getParameter(m_pBufType).getInt();
		double sizemod = a_inputs.getValue(m_iSize);
		int bufsamples;
		switch(buftype)
		{
		case 3: // bpm
			bufsamples = freqToSamples(bpmToFreq(getParameter(m_pBufBPMFreq).getEnum() + sizemod, getTempo()), getFs());
			break;
		case 2: // freq
			bufsamples = freqToSamples(getParameter(m_pBufFreq).getDouble() + sizemod, getFs());
			break;
		case 1: // seconds
			bufsamples = periodToSamples(getParameter(m_pBufDelay).getDouble() + sizemod, getFs());
			break;
		case 0: // samples
		default:
			bufsamples = getParameter(m_pBufSize).getInt() + sizemod;
			break;
		}
		m_delay.resizeBuffer(bufsamples);
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
