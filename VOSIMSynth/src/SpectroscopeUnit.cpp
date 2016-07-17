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
#include <UILabel.h>
#include <UIPlot.h>

namespace synui
{
	const vector<string> bufferSizeSelections = { "128","256","512","1024","2048","4096" };
	const vector<double> bufferSizeValues = { 128,256,512,1024,2048,4096 };

	SpectroscopeUnit::SpectroscopeUnit(const string& a_name) :
		Unit(a_name),
		m_bufferIndex(0),
		m_pBufferSize(addParameter_(syn::UnitParameter("buffer size", bufferSizeSelections, bufferSizeValues))),
		m_pTimeSmooth(addParameter_(syn::UnitParameter("time smooth", 0.0, 1.0, 0.1))),
		m_pUpdatePeriod(addParameter_(syn::UnitParameter("update interval", 0.0, 1.0, 0.5))),
		m_samplesSinceLastUpdate(0),
		m_isStale(true)
	{
		addInput_("in");

		m_inBufferSize = getParameter(m_pBufferSize).getEnum();
		m_outBufferSize = m_inBufferSize >> 1;
		int maxBufferSize = getParameter(m_pBufferSize).getEnum(getParameter(m_pBufferSize).getMax());
		m_timeDomainBuffer.resize(maxBufferSize);
		m_freqDomainBuffer.resize(maxBufferSize);
		m_magnitudeBuffer.resize(maxBufferSize);

		m_window.resize(m_inBufferSize);
		for (int i = 0; i < m_inBufferSize; i++) {
			m_window[i] = syn::blackman_harris(i, m_inBufferSize);
		}
	}

	void SpectroscopeUnit::onParamChange_(int a_paramId) {
		if (a_paramId == m_pBufferSize) {
			int newBufferSize = getParameter(m_pBufferSize).getEnum();
			m_inBufferSize = newBufferSize;
			m_outBufferSize = newBufferSize >> 1;
			m_bufferIndex = syn::WRAP(m_bufferIndex, m_inBufferSize);

			m_window.resize(m_inBufferSize);
			for (int i = 0; i < m_inBufferSize; i++) {
				m_window[i] = syn::blackman_harris(i, m_inBufferSize);
			}
		}
	}

	void SpectroscopeUnit::process_() {
		m_timeDomainBuffer[m_bufferIndex] = getInputValue(0) * m_window[m_bufferIndex];

		m_samplesSinceLastUpdate++;
		int updatePeriod = syn::LERP<double>(getFs()*0.001, m_inBufferSize, 1 - getParameter(m_pUpdatePeriod).getDouble());
		if (!m_isStale && m_samplesSinceLastUpdate >= updatePeriod) {
			m_samplesSinceLastUpdate = 0;

			// Execute FFT on freq buffers
			ffts_plan_t* plan = ffts_init_1d_real(m_inBufferSize, FFTS_FORWARD);
			ffts_execute(plan, &m_timeDomainBuffer[0], &m_freqDomainBuffer[0]);
			ffts_free(plan);

			// Copy freq magnitudes to output buffers
			double timeSmooth = getParameter(m_pTimeSmooth).getDouble();
			for (int j = 0; j < m_outBufferSize; j++) {
				double target = m_freqDomainBuffer[j].real() * m_freqDomainBuffer[j].real() + m_freqDomainBuffer[j].imag() * m_freqDomainBuffer[j].imag();
				if (target)
					target = 10 * log10(target); // this is scaled by 10 because we didn't take the square root above
				else
					target = -120; // minimum dB
				m_magnitudeBuffer[j] = m_magnitudeBuffer[j] + (1.0 / 60.0) * (1 - timeSmooth) * (target - m_magnitudeBuffer[j]);
			}

			m_isStale = true;
		}
		m_bufferIndex = syn::WRAP(m_bufferIndex + 1, m_inBufferSize);
	}

	SpectroscopeUnitControl::SpectroscopeUnitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId) :
		UIUnitControl(a_window, a_vm, a_unitId),
		m_col(new UICol(a_window)),
		m_statusLabel(new UILabel(a_window)),
		m_plot(new UIPlot(a_window)),
		m_defCtrl(new UIDefaultUnitControl(a_window, a_vm, a_unitId))
	{
		m_col->addChild(m_defCtrl);
		m_col->addChild(m_plot);
		m_col->addChild(m_statusLabel);
		m_statusLabel->setSize({ -1,8 });
		m_col->setChildResizePolicy(UICell::CMAX);
		m_col->setSelfResizePolicy(UICell::SRNONE);
		m_col->setGreedyChild(m_plot);
		addChild(m_col);

		m_plot->setStatusLabel(m_statusLabel);
		m_plot->setXScale(UIPlot::LogScale2);
		m_plot->setXUnits("Hz");
		m_plot->setYUnits("dB");
		m_plot->setInterpPolicy(UIPlot::SincInterp);
		m_plot->setAutoAdjustSpeed(10);

		m_plot->setNumBuffers(1);

		setMinSize(minSize().cwiseMax(m_col->minSize()));
	}

	void SpectroscopeUnitControl::_onResize() {
		m_col->setSize(size());
	}

	void SpectroscopeUnitControl::draw(NVGcontext* a_nvg) {
		SpectroscopeUnit* unit = static_cast<SpectroscopeUnit*>(&m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()));
		const double minFreq = 20;
		int minIndex = ceil(minFreq * unit->getBufferSize() / (0.5 * unit->getFs()));
		m_plot->setBufferPtr(0, unit->getBufferPtr() + minIndex, unit->getBufferSize() - minIndex);
		m_plot->setXBounds({ (minIndex * 1.0 / unit->getBufferSize()) * 0.5 * unit->getFs(),unit->getFs()*0.5 });
		unit->setDirty();
	}
}