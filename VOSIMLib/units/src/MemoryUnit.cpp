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
#include <tables.h>

#include "common.h"
CEREAL_REGISTER_TYPE(syn::MemoryUnit);
CEREAL_REGISTER_TYPE(syn::VariableMemoryUnit);

using namespace std;

namespace syn
{
	//---------------------
	// NSampleDelay
	//---------------------
	NSampleDelay::NSampleDelay() :
		m_buffer(1),
		m_arraySize(1),
		m_nBufSamples(1.0),
		m_curReadPhase(0.0),
		m_curWritePhase(0.0),
		m_currOutput(0.0)
	{
		resizeBuffer(1);
	}

	double NSampleDelay::getCurrOutput() const
	{
		return m_currOutput;
	};

	double NSampleDelay::getPastSample(double a_offset)
	{
		int bufferReadPhase = WRAP(m_curReadPhase - a_offset, m_nBufSamples);
		return getresampled_single(&m_buffer[0], ceil(m_nBufSamples), bufferReadPhase, m_nBufSamples, lut_blimp_table_online());
	}

	void NSampleDelay::resizeBuffer(double a_nBufSamples)
	{
		if (m_nBufSamples == a_nBufSamples || a_nBufSamples <= 0)
			return;
		int requiredBufSize = ceil(a_nBufSamples);
		if (requiredBufSize > m_buffer.size())
			m_buffer.resize(requiredBufSize);
		m_arraySize = static_cast<int>(m_buffer.size());
		m_nBufSamples = a_nBufSamples;
		m_curReadPhase = WRAP<double>(m_curWritePhase - m_nBufSamples, m_arraySize);
	}

	void NSampleDelay::clearBuffer()
	{
		fill(&m_buffer.front(), &m_buffer.back(), 0.0);
	}

	int NSampleDelay::size() const
	{
		return static_cast<int>(m_buffer.size());
	}

	double NSampleDelay::process(double a_input)
	{
		// Read
		int rInd1 = static_cast<int>(m_curReadPhase);
		int rInd2 = WRAP<int>(rInd1 + 1, m_arraySize);
		m_currOutput = LERP(m_buffer[rInd1], m_buffer[rInd2], m_curReadPhase - rInd1);
		m_curReadPhase = WRAP<double>(m_curReadPhase + 1.0, m_arraySize);

		// Write
		m_buffer[static_cast<int>(m_curWritePhase)] = a_input;
		m_curWritePhase = WRAP<double>(m_curWritePhase + 1.0, m_arraySize);
		return m_currOutput;
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
		MemoryUnit(a_rhs.name())
	{
	}

	void MemoryUnit::reset() { m_delay.clearBuffer(); }

	void MemoryUnit::onParamChange_(int a_paramId)
	{
	}

	void MemoryUnit::process_()
	{
		setOutputChannel_(0, m_delay.process(readInput(0)));
	}

	//---------------------
	// ResampleUnit
	//---------------------
	VariableMemoryUnit::VariableMemoryUnit(const string& a_name) :
		Unit(a_name),
		m_delaySamples(0)
	{
		addInput_(iIn, "in");
		addInput_(iReceive, "recv");
		addInput_(iSizeMod, "size");
		addOutput_(oOut, "out");
		addOutput_(oSend, "send");
		addParameter_(pBufDelay, UnitParameter("time", 0.0001, 1.0, 0.0001, UnitParameter::EUnitsType::Seconds, 4));
		addParameter_(pBufFreq, UnitParameter("freq", 1.0, 10000.0, 10000.0, UnitParameter::EUnitsType::Freq).setVisible(false));
		addParameter_(pBufBPMFreq, UnitParameter("rate", g_bpmStrs, g_bpmVals, 0, UnitParameter::EUnitsType::BPM).setVisible(false));
		addParameter_(pBufType, UnitParameter("units", { "sec","Hz","BPM" }, { pBufDelay, pBufFreq, pBufBPMFreq }));
		addParameter_(pDryGain, UnitParameter("dry", 0.0, 1.0, 0.0));
		m_delay.resizeBuffer(48000);
	}

	VariableMemoryUnit::VariableMemoryUnit(const VariableMemoryUnit& a_rhs) :
		VariableMemoryUnit(a_rhs.name())
	{
	}

	void VariableMemoryUnit::reset() { m_delay.clearBuffer(); }

	void VariableMemoryUnit::onParamChange_(int a_paramId)
	{
		int newtype;
		switch (a_paramId) {
		case pBufType:
			newtype = param(pBufType).getInt();
			param(pBufDelay).setVisible(newtype == 0);
			param(pBufFreq).setVisible(newtype == 1);
			param(pBufBPMFreq).setVisible(newtype == 2);
			break;
		default:
			break;
		}
	}

	void VariableMemoryUnit::process_()
	{
		int bufType = param(pBufType).getEnum();
		switch (bufType) {
		case pBufDelay:
			m_delaySamples = periodToSamples(param(pBufDelay).getDouble() + readInput(iSizeMod), fs());
			break;
		case pBufFreq:
			m_delaySamples = freqToSamples(param(pBufFreq).getDouble() + readInput(iSizeMod), fs());
			break;
		case pBufBPMFreq:
			m_delaySamples = freqToSamples(bpmToFreq(param(pBufBPMFreq).getEnum(param(pBufBPMFreq).getInt() + readInput(iSizeMod)), tempo()), fs());
			break;
		default:
			break;
		}
		m_delay.resizeBuffer(m_delaySamples);
		double input = readInput(iIn);
		double receive = readInput(iReceive);
		double dryMix = input * param(pDryGain).getDouble();
		double output = m_delay.process(input + receive);
		setOutputChannel_(oOut, output + dryMix);
		setOutputChannel_(oSend, output);
	}
}