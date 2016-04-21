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
 *	 \file SpectroscopeUnit.h
 *   \brief 
 *   \details
 *   \author Austen Satterlee
 *   \date 03/14/2016
 */
#pragma once
#include "Unit.h"
#include "DSPMath.h"
#include "fft.h"
#include <array>
#include <vector>

using std::vector;
using std::array;

namespace syn {
	class SpectroscopeUnit : public Unit
	{
	public:
		SpectroscopeUnit(const string& a_name);

		SpectroscopeUnit(const SpectroscopeUnit& a_rhs) :
			SpectroscopeUnit(a_rhs.getName())
		{}

		size_t getNumBuffers() const {
			return m_nBuffers;
		}

		size_t getBufferSize() const {
			return m_outBufferSize;
		}

		const double* getBuffer(int a_bufInd) const {
			return &m_outBuffers[a_bufInd][0];
		}

		array<double,2> getBufferExtrema(int a_bufInd) const {
			double bufmin = m_outBuffers[a_bufInd][m_outBufferExtrema[a_bufInd][0]];
			double bufmax = m_outBuffers[a_bufInd][m_outBufferExtrema[a_bufInd][1]];
			return { bufmin, bufmax };
		}
	protected:
		void onParamChange_(int a_paramId) override;
		void MSFASTCALL process_(const SignalBus& a_inputs, SignalBus& a_outputs) GCCFASTCALL override;
		
	private:
		string _getClassName() const override { return "SpectroscopeUnit"; };
		Unit* _clone() const override { return new SpectroscopeUnit(*this);  };
	private:
		friend class SpectroscopeUnitControl;
		int m_bufferIndex;
		int m_inBufferSize, m_outBufferSize;
		int m_nBuffers;
		int m_pBufferSize, m_pTimeSmooth, m_pUpdatePeriod;

		int m_samplesSinceLastUpdate;
		
		vector<vector<double> > m_timeBuffers;
		vector<vector<WDL_FFT_COMPLEX> > m_freqBuffers;
		vector<vector<double> > m_outBuffers;
		vector<array<int,2> > m_outBufferExtrema;
		vector<double> m_window;
		bool m_isActive;
	};

}
