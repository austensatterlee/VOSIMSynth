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

using namespace std;

namespace syn {
	void MemoryUnit::onParamChange_(int a_paramId) {
		if(a_paramId == m_pBufSize) {
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
}

