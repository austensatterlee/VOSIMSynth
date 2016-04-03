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

#include "SpectroscopeUnit.h"
#include <DSPMath.h>

namespace syn
{
	const vector<string> bufferSizeSelections = { "128","256","512","1024","2048","4096" };
	const vector<double> bufferSizeValues = {128,256,512,1024,2048,4096};

	SpectroscopeUnit::SpectroscopeUnit(const string& a_name) :
		Unit(a_name),
		m_bufferIndex(0),
		m_nBuffers(2),
		m_pBufferSize(addParameter_(UnitParameter("buffer size", bufferSizeSelections, bufferSizeValues))),
		m_pTimeSmooth(addParameter_(UnitParameter("time smooth", 0.0, 1.0, 0.1))),
		m_pUpdatePeriod(addParameter_(UnitParameter("update interval",0.0,1.0,0.5))),
		m_samplesSinceLastUpdate(0),
		m_isActive(true)
	{
		addInput_("in1");
		addInput_("in2");

		m_timeBuffers.resize(m_nBuffers);
		m_freqBuffers.resize(m_nBuffers);
		m_outBuffers.resize(m_nBuffers);
		m_outBufferExtrema.resize(m_nBuffers);

		m_inBufferSize = getParameter(m_pBufferSize).getEnum();
		m_outBufferSize = m_inBufferSize >> 1;
		for (int i = 0; i < m_nBuffers; i++) {
			m_timeBuffers[i].resize(m_inBufferSize);
			m_freqBuffers[i].resize(m_inBufferSize);
			m_outBuffers[i].resize(m_outBufferSize);
		}

		m_window.resize(m_inBufferSize);
		for (int i = 0; i < m_inBufferSize; i++) {
			m_window[i] = blackman_harris(i, m_inBufferSize);
		}
	}

	void SpectroscopeUnit::onParamChange_(int a_paramId) {
		if (a_paramId == m_pBufferSize) {
			int newBufferSize = getParameter(m_pBufferSize).getEnum();
			m_inBufferSize = newBufferSize;
			m_outBufferSize = newBufferSize >> 1;
			m_bufferIndex = MIN(m_bufferIndex, m_inBufferSize-1);
			for (int i = 0; i < m_nBuffers; i++) {
				m_timeBuffers[i].resize(m_inBufferSize);
				m_freqBuffers[i].resize(m_inBufferSize);
				m_outBuffers[i].resize(m_outBufferSize);
				m_outBufferExtrema[i] = { 0,0 };
			}

			m_window.resize(m_inBufferSize);
			for (int i = 0; i < m_inBufferSize;i++) {
				m_window[i] = blackman_harris(i, m_inBufferSize);
			}
		}
	}

	void SpectroscopeUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		for (int i = 0; i < m_nBuffers; i++) {
			m_timeBuffers[i][m_bufferIndex] = a_inputs.getValue(i);
		}

		m_samplesSinceLastUpdate++;
		int updatePeriod = LERP(16,m_inBufferSize, 1 - getParameter(m_pUpdatePeriod).getDouble());
		if (m_samplesSinceLastUpdate >= updatePeriod && m_isActive) {
			m_samplesSinceLastUpdate = 0;
			for (int i = 0; i < m_nBuffers; i++) {
				// Copy from input buffers to freq buffers
				
				for (int j = 0; j < m_inBufferSize; j++) {
					m_freqBuffers[i][j].im = 0;
					m_freqBuffers[i][j].re = m_window[j]*m_timeBuffers[i][j];
				}

				// Execute FFT on freq buffers
				WDL_fft(&m_freqBuffers[i][0], m_inBufferSize, 0);

				// Copy freq magnitudes to output buffers
				double timeSmooth = getParameter(m_pTimeSmooth).getDouble();
				double outmin = INFINITY, outmax = -INFINITY;
				int argoutmin = 0, argoutmax = 0;
				for (int j = 0; j < m_outBufferSize;j++) {
					int k = WDL_fft_permute(m_inBufferSize, j);
					double target = m_freqBuffers[i][k].re*m_freqBuffers[i][k].re + m_freqBuffers[i][k].im*m_freqBuffers[i][k].im;
					target = 10 * log10(target+1e-12);
					m_outBuffers[i][j] = m_outBuffers[i][j] + (1.0/50.0)*(1-timeSmooth)*(target - m_outBuffers[i][j]);

					if(m_outBuffers[i][j]>outmax) {
						outmax = m_outBuffers[i][j];
						argoutmax = j;
					}
					if (m_outBuffers[i][j]<outmin) {
						outmin = m_outBuffers[i][j];
						argoutmin = j;
					}
				}
				// Record extrema locations
				m_outBufferExtrema[i] = { argoutmin,argoutmax };
			}
		}
		m_bufferIndex = WRAP(m_bufferIndex + 1, m_inBufferSize);
	}
}

