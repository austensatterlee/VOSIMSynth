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
#include <tables.h>
#include <UILabel.h>
#include <UIPlot.h>

namespace syn
{
	const vector<string> bufferSizeSelections = { "128","256","512","1024","2048","4096" };
	const vector<double> bufferSizeValues = { 128,256,512,1024,2048,4096 };

	SpectroscopeUnit::SpectroscopeUnit(const string& a_name) :
		Unit(a_name),
		m_bufferIndex(0),
		m_pBufferSize(addParameter_(UnitParameter("buffer size", bufferSizeSelections, bufferSizeValues))),
		m_pTimeSmooth(addParameter_(UnitParameter("time smooth", 0.0, 1.0, 0.1))),
		m_pUpdatePeriod(addParameter_(UnitParameter("update interval", 0.0, 1.0, 0.5))),
		m_samplesSinceLastUpdate(0),
		m_isActive(true) {
		addInput_("in");

		m_inBufferSize = getParameter(m_pBufferSize).getEnum();
		m_outBufferSize = m_inBufferSize >> 1;
		int maxBufferSize = getParameter(m_pBufferSize).getEnum(getParameter(m_pBufferSize).getMax());
		m_timeBuffer.resize(maxBufferSize);
		m_freqBuffer.resize(maxBufferSize);
		m_outBuffer.resize(maxBufferSize);

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
			m_bufferIndex = WRAP(m_bufferIndex, m_inBufferSize);
			

			m_window.resize(m_inBufferSize);
			for (int i = 0; i < m_inBufferSize; i++) {
				m_window[i] = blackman_harris(i, m_inBufferSize);
			}
		}
	}

	void SpectroscopeUnit::process_() {
		m_timeBuffer[m_bufferIndex] = getInputValue(0);

		m_samplesSinceLastUpdate++;
		int updatePeriod = LERP<double>(16, m_inBufferSize, 1 - getParameter(m_pUpdatePeriod).getDouble());
		if (m_samplesSinceLastUpdate >= updatePeriod && m_isActive) {
			m_samplesSinceLastUpdate = 0;

			for (int j = 0; j < m_inBufferSize; j++) {
				m_freqBuffer[j].im = 0;
				m_freqBuffer[j].re = m_window[j] * m_timeBuffer[j];
			}

			// Execute FFT on freq buffers
			WDL_fft(&m_freqBuffer[0], m_inBufferSize, 0);

			// Copy freq magnitudes to output buffers
			double timeSmooth = getParameter(m_pTimeSmooth).getDouble();
			for (int j = 0; j < m_outBufferSize; j++) {
				int k = WDL_fft_permute(m_inBufferSize, j);
				double target = m_freqBuffer[k].re * m_freqBuffer[k].re + m_freqBuffer[k].im * m_freqBuffer[k].im;
				if (target)
					target = 10 * log10(target); // this is scaled by 10 because we didn't take the square root above
				else
					target = -120; // minimum dB
				m_outBuffer[j] = m_outBuffer[j] + (1.0 / 60.0) * (1 - timeSmooth) * (target - m_outBuffer[j]);
			}
			
		}
		m_bufferIndex = WRAP(m_bufferIndex + 1, m_inBufferSize);
	}

	SpectroscopeUnitControl::SpectroscopeUnitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId) :
		UIUnitControl(a_window, a_vm, a_unitId), 
		m_col(new UICol(a_window)),
		m_statusLabel(new UILabel(a_window)),
		m_plot(new UIPlot(a_window)),
		m_defCtrl(new DefaultUnitControl(a_window, a_vm, a_unitId)),
		m_resizeHandle(new UIResizeHandle(a_window))
	{
		m_col->addChild(m_defCtrl);
		m_col->addChild(m_plot);
		m_col->addChild(m_statusLabel);
		m_statusLabel->setSize({ -1,12 });
		m_col->setChildResizePolicy(UICell::CMAX);
		m_col->setSelfResizePolicy(UICell::SRNONE);
		addChild(m_col);

		m_plot->setStatusLabel(m_statusLabel);
		m_plot->setXScale(UIPlot::LogScale2);
		m_plot->setXUnits("Hz");
		m_plot->setInterpPolicy(UIPlot::SincInterp);

		addChild(m_resizeHandle);
		m_resizeHandle->setDragCallback([&](const Vector2i& a_relPos, const Vector2i& a_diffPos) {
			grow(a_diffPos);
			m_plot->grow(a_diffPos);
		});

		setMinSize(minSize().cwiseMax(m_col->minSize()));
	}

	void SpectroscopeUnitControl::_onResize() {
		m_col->setSize(size());
		m_resizeHandle->setRelPos(size() - m_resizeHandle->size());
	}

	void SpectroscopeUnitControl::draw(NVGcontext* a_nvg) {
		const SpectroscopeUnit* unit = static_cast<const SpectroscopeUnit*>(&m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()));
		const double minFreq = 20;
		int minIndex = ceil(minFreq * unit->getBufferSize() / (0.5 * unit->getFs()));
		m_plot->setBufferPtr(unit->getBufferPtr() + minIndex, unit->getBufferSize() - minIndex);
		m_plot->setXBounds({ (minIndex * 1.0 / unit->getBufferSize()) * 0.5 * unit->getFs(),unit->getFs()*0.5 });
	}
}
