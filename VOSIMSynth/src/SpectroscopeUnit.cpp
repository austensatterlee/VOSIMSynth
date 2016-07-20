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
		m_pTimeSmooth(addParameter_(syn::UnitParameter("time smooth", 0.0, 1.0, 0.0))),
		m_pUpdatePeriod(addParameter_(syn::UnitParameter("update interval", 0.0, 1.0, 0.5))),
		m_samplesSinceLastUpdate(0),
		m_numBuffers(2),
		m_isStale(true)
	{
		m_inBufferSize = getParameter(m_pBufferSize).getEnum();
		m_outBufferSize = m_inBufferSize >> 1;
		m_plan = ffts_init_1d_real(m_inBufferSize, FFTS_FORWARD);
		int maxInBufferSize = getParameter(m_pBufferSize).getEnum(getParameter(m_pBufferSize).getMax());
		int maxOutBufferSize = maxInBufferSize >> 1;

		m_timeDomainBuffers.resize(m_numBuffers);
		m_freqDomainBuffers.resize(m_numBuffers);
		m_magnitudeBuffers.resize(m_numBuffers);
		for (int i = 0; i < m_numBuffers; i++) {
			addInput_("in" + to_string(i));
			m_timeDomainBuffers[i].resize(maxInBufferSize);
			m_freqDomainBuffers[i].resize(maxOutBufferSize);
			m_magnitudeBuffers[i].resize(maxOutBufferSize);
		}

		m_window.resize(m_inBufferSize);
		for (int i = 0; i < m_inBufferSize; i++) {
			m_window[i] = syn::blackman_harris(i, m_inBufferSize);
		}
	}

	SpectroscopeUnit::~SpectroscopeUnit() {
		ffts_free(m_plan);
		m_plan = nullptr;
	}

	int SpectroscopeUnit::getNumBuffers() const {
		return m_numBuffers;
	}

	const double* SpectroscopeUnit::getBufferPtr(int a_bufIndex) const {
		return &m_magnitudeBuffers[a_bufIndex][0];
	}

	int SpectroscopeUnit::getBufferSize(int a_bufIndex) const {
		return getInputSource(a_bufIndex) ? m_outBufferSize : 0;
	}

	void SpectroscopeUnit::onParamChange_(int a_paramId) {
		if (a_paramId == m_pBufferSize) {
			int newBufferSize = getParameter(m_pBufferSize).getEnum();
			m_inBufferSize = newBufferSize;
			m_outBufferSize = newBufferSize >> 1;
			m_bufferIndex = syn::WRAP(m_bufferIndex, m_inBufferSize);

			ffts_free(m_plan);
			m_plan = ffts_init_1d_real(m_inBufferSize, FFTS_FORWARD);

			m_window.resize(m_inBufferSize);
			for (int i = 0; i < m_inBufferSize; i++) {
				m_window[i] = syn::blackman_harris(i, m_inBufferSize);
			}
		}
	}

	void SpectroscopeUnit::process_() {
		for (int i = 0; i < m_numBuffers; i++) {
			m_timeDomainBuffers[i][m_bufferIndex] = getInputValue(i) * m_window[m_bufferIndex];
		}

		m_samplesSinceLastUpdate++;
		int updatePeriod = syn::LERP<double>(getFs()*0.001, m_inBufferSize, 1 - getParameter(m_pUpdatePeriod).getDouble());
		if (!m_isStale && m_samplesSinceLastUpdate >= updatePeriod) {
			m_samplesSinceLastUpdate = 0;

			// Execute FFT on freq buffers
			for (int i = 0; i < m_numBuffers; i++) {
				ffts_execute(m_plan, &m_timeDomainBuffers[i][0], &m_freqDomainBuffers[i][0]);

				// Copy freq magnitudes to output buffers
				double timeSmooth = getParameter(m_pTimeSmooth).getDouble();
				for (int j = 0; j < m_outBufferSize; j++) {
					double target = m_freqDomainBuffers[i][j].real() * m_freqDomainBuffers[i][j].real() + m_freqDomainBuffers[i][j].imag() * m_freqDomainBuffers[i][j].imag();
					if (target)
						target = syn::MAX(10 * log10(target), -60.0); // this is scaled by 10 because we didn't take the square root above
					else
						target = -60; // minimum dB
					m_magnitudeBuffers[i][j] = m_magnitudeBuffers[i][j] + (1 - timeSmooth) * (target - m_magnitudeBuffers[i][j]);
				}
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
		m_plot->setAutoAdjustSpeed(10.);

		setMinSize(minSize().cwiseMax(m_col->minSize()));
	}

	void SpectroscopeUnitControl::_onResize() {
		m_col->setSize(size());
	}

	void SpectroscopeUnitControl::draw(NVGcontext* a_nvg) {
		SpectroscopeUnit* unit = static_cast<SpectroscopeUnit*>(&m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()));
		const double minFreq = 80;

		m_plot->setNumBuffers(unit->getNumBuffers());
		for (int i = 0; i < unit->getNumBuffers(); i++) {
			int minIndex = ceil(minFreq * unit->getBufferSize(i) / (0.5 * unit->getFs()));
			m_plot->setBufferPtr(i, unit->getBufferPtr(i) + minIndex, unit->getBufferSize(i) - minIndex);
			if(unit->getBufferSize(i))
				m_plot->setXBounds({ (minIndex * 1.0 / unit->getBufferSize(i)) * 0.5 * unit->getFs(),unit->getFs()*0.5 });
		}
		unit->setDirty();
	}
}