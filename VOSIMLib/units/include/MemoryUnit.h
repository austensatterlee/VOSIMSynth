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
		NSampleDelay();
		double process(double a_input);
		double getPastSample(double a_offset);
		void resizeBuffer(double a_newBufSize);
		void clearBuffer();
		int size() const;
	private:
		vector<double> m_buffer;
		int m_arraySize;
		double m_nBufSamples;
		double m_curReadPhase;
		double m_curWritePhase;
	};

	class MemoryUnit : public Unit
	{
		DERIVE_UNIT(MemoryUnit)
	public:
		explicit MemoryUnit(const string& a_name);
		MemoryUnit(const MemoryUnit& a_rhs);

	protected:
		void onParamChange_(int a_paramId) override;
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		NSampleDelay m_delay;
	};

	class ResampleUnit : public Unit
	{
		DERIVE_UNIT(ResampleUnit)
	public:
		explicit ResampleUnit(const string& a_name);
		ResampleUnit(const ResampleUnit& a_rhs);

	protected:
		void onParamChange_(int a_paramId) override;
		void MSFASTCALL process_() GCCFASTCALL override;
	private:
		NSampleDelay m_delay;
		double m_delaySamples;
		int m_iSize;

		int m_pBufDelay;
		int m_pBufFreq;
		int m_pBufBPMFreq;
		int m_pBufType;
	};
}
CEREAL_REGISTER_TYPE(syn::MemoryUnit)
CEREAL_REGISTER_TYPE(syn::ResampleUnit)
#endif
