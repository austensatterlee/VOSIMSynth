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

using namespace std;

namespace syn
{
	//---------------------
	// NSampleDelay
	//---------------------
	NSampleDelay::NSampleDelay() :
		m_buffer(48000),
		m_arraySize(48000),
		m_nBufSamples(1.0),
		m_curReadPhase(0.0),
		m_curWritePhase(0.0)
	{
		resizeBuffer(1);
	};

	double NSampleDelay::getPastSample(double a_offset)
	{
		int bufferReadPhase = WRAP(m_curReadPhase - a_offset, m_nBufSamples);
		return getresampled_single(&m_buffer[0], ceil(m_nBufSamples), bufferReadPhase, m_nBufSamples, lut_blimp_table_online);
	}

	void NSampleDelay::resizeBuffer(double a_nBufSamples)
	{
		if (m_nBufSamples == a_nBufSamples || a_nBufSamples <= 0)
			return;
		int requiredBufSize = ceil(a_nBufSamples);
		if (requiredBufSize > m_buffer.size())
			m_buffer.resize(requiredBufSize);
		m_arraySize = m_buffer.size();
		m_nBufSamples = a_nBufSamples;
		m_curReadPhase = WRAP<double>(m_curWritePhase - m_nBufSamples, m_arraySize);
	}

	void NSampleDelay::clearBuffer()
	{
		fill(&m_buffer.front(), &m_buffer.back(), 0.0);
	}

	int NSampleDelay::size() const
	{
		return m_buffer.size();
	}

	double NSampleDelay::process(double a_input)
	{
		if (isnan(m_buffer[m_curReadPhase]) || isinf(m_buffer[m_curReadPhase])) {
			clearBuffer();
		}

		int writeIndex = m_curWritePhase;
		m_buffer[writeIndex] = a_input;
		m_curWritePhase = WRAP<double>(m_curWritePhase + 1.0, m_arraySize);

		int rInd1 = int(m_curReadPhase);
		int rInd2 = WRAP<int>(int(m_curReadPhase) + 1, m_arraySize);
		double output = LERP(m_buffer[rInd1], m_buffer[rInd2], m_curReadPhase - rInd1);
		m_curReadPhase = WRAP<double>(m_curReadPhase + 1.0, m_arraySize);
		return output;
	}

	//---------------------
	// MemoryUnit
	//---------------------
	MemoryUnit::MemoryUnit(const string& a_name) :
		Unit(a_name),
		m_pBufSize(addParameter_(UnitParameter("n", 1.0, 192.0E3, 1.0)))
	{
		getParameter_(m_pBufSize).setControlType(UnitParameter::Unbounded);
		addInput_("in");
		addOutput_("out");
	}

	MemoryUnit::MemoryUnit(const MemoryUnit& a_rhs) :
		MemoryUnit(a_rhs.getName())
	{
	}

	string MemoryUnit::_getClassName() const
	{
		return "MemoryUnit";
	}

	Unit* MemoryUnit::_clone() const
	{
		return new MemoryUnit(*this);
	}

	void MemoryUnit::onParamChange_(int a_paramId)
	{
		if (a_paramId == m_pBufSize) {
			double newBufSize = getParameter(m_pBufSize).getDouble();
			m_delay.resizeBuffer(newBufSize);
		}
	}

	void MemoryUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
	{
		double input = a_inputs.getValue(0);
		double output = m_delay.process(input);
		a_outputs.setChannel(0, output);
	}

	//---------------------
	// ResampleUnit
	//---------------------
	ResampleUnit::ResampleUnit(const string& a_name) :
		Unit(a_name),
		m_delaySamples(0)
	{
		addInput_("in");
		m_iSize = addInput_("size");
		addOutput_("out");
		m_pBufSize = addParameter_({ "samples", 1.0, 48000.0, 1.0 });
		m_pBufDelay = addParameter_({ "delay", 0.001, 1.0, 0.001, UnitParameter::EUnitsType::Seconds });
		m_pBufFreq = addParameter_({ "freq", 1.0, 10000.0, 1.0, UnitParameter::EUnitsType::Freq });
		m_pBufBPMFreq = addParameter_({ "rate", g_bpmStrs, g_bpmVals, 0, UnitParameter::EUnitsType::BPM });
		getParameter_(m_pBufDelay).setVisible(false);
		getParameter_(m_pBufFreq).setVisible(false);
		getParameter_(m_pBufBPMFreq).setVisible(false);
		m_pBufType = addParameter_(UnitParameter{ "units",{"samples","seconds","Hz","BPM"} });
	}

	ResampleUnit::ResampleUnit(const ResampleUnit& a_rhs) :
		ResampleUnit(a_rhs.getName())
	{
	}

	string ResampleUnit::_getClassName() const
	{
		return "ResampleUnit";
	}

	Unit* ResampleUnit::_clone() const
	{
		return new ResampleUnit(*this);
	}

	void ResampleUnit::onParamChange_(int a_paramId)
	{
		if (a_paramId == m_pBufType) {
			int newtype = getParameter(m_pBufType).getInt();
			getParameter_(m_pBufSize).setVisible(newtype == 0);
			getParameter_(m_pBufDelay).setVisible(newtype == 1);
			getParameter_(m_pBufFreq).setVisible(newtype == 2);
			getParameter_(m_pBufBPMFreq).setVisible(newtype == 3);
		}
	}

	void ResampleUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs)
	{
		int bufType = getParameter(m_pBufType).getInt();
		if (bufType == m_pBufSize) { // samples
			m_delaySamples = getParameter(m_pBufSize).getDouble() + a_inputs.getValue(m_iSize);
		}
		else if (bufType == m_pBufDelay) { // seconds
			m_delaySamples = periodToSamples(getParameter(m_pBufDelay).getDouble() + a_inputs.getValue(m_iSize), getFs());
		}
		else if (bufType == m_pBufFreq) { // freq
			m_delaySamples = freqToSamples(getParameter(m_pBufFreq).getDouble() + a_inputs.getValue(m_iSize), getFs());
		}
		else if (bufType == m_pBufBPMFreq) { // bpm
			m_delaySamples = freqToSamples(bpmToFreq(getParameter(m_pBufBPMFreq).getEnum(getParameter(m_pBufFreq).getInt() + a_inputs.getValue(m_iSize)), getTempo()), getFs());
		}
		m_delay.resizeBuffer(m_delaySamples);
		double input = a_inputs.getValue(0);
		double output = m_delay.process(input);
		a_outputs.setChannel(0, output);
	}
}