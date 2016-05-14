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
 * \file MemoryUnit.h
 * \brief
 * \details
 * \author Austen Satterlee
 * \date March 6, 2016
 */
#ifndef __MEMORYUNIT__
#define __MEMORYUNIT__

#include "Unit.h"

using namespace std;

namespace syn
{
	/**
	* General N-Sample delay
	*/
	class NSampleDelay
	{
	public:
		NSampleDelay() :
			m_buffer(1, 0),
			m_bufferSize(1),
			m_bufferIndex(0) { };

		double process(double a_input);

		double getPastSample(int a_offset);

		void resizeBuffer(int a_newBufSize);

		void clearBuffer();

		int size() const;
	private:
		vector<double> m_buffer;
		int m_bufferSize;
		int m_bufferIndex;
	};

	class MemoryUnit : public Unit
	{
	public:
		explicit MemoryUnit(const string& a_name);

		MemoryUnit(const MemoryUnit& a_rhs);

	protected:
		void onParamChange_(int a_paramId) override;

		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;
	private:
		string _getClassName() const override;

		Unit* _clone() const override;

	private:
		NSampleDelay m_delay;
		int m_pBufSize;
	};
}
#endif
