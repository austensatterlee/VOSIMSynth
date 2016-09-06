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

#include "OscilloscopeUnit.h"
#include "DSPMath.h"
#include "UIDefaultUnitControl.h"
#include "UIPlot.h"
#include "UILabel.h"

#include "common.h"

CEREAL_REGISTER_TYPE(synui::OscilloscopeUnit);

namespace synui
{
	OscilloscopeUnit::OscilloscopeUnit(const string& a_name) :
		Unit(a_name),
		m_bufferIndex(0),
		m_lastPhase(0.0),
		m_lastSync(0),
		m_syncCount(0),
		m_numBuffers(2)
	{
		addParameter_(BufferSize, syn::UnitParameter("buffer size", 16, 96000, 256));
		addParameter_(NumPeriods, syn::UnitParameter("periods", 1, 16, 1));
		m_bufferSize = getParameter(BufferSize).getInt();
		m_buffers.resize(m_numBuffers);
		for(int i=0;i<m_numBuffers;i++) {
			addInput_("in"+to_string(i));
			m_buffers[i].resize(getParameter(BufferSize).getMax());
		}
		m_iPhase = addInput_("ph");
	}

	int OscilloscopeUnit::getNumBuffers() const
	{
		return m_numBuffers;
	}

	const double* OscilloscopeUnit::getBufferPtr(int a_bufIndex) const {
		return &m_buffers[a_bufIndex][0];
	}

	int OscilloscopeUnit::getBufferSize(int a_bufIndex) const {
		return getInputSource(a_bufIndex) ? m_bufferSize : 0;
	}

	void OscilloscopeUnit::onParamChange_(int a_paramId) {
		if (a_paramId == BufferSize) {
			int newBufferSize = getParameter(BufferSize).getInt();
			m_bufferIndex = syn::WRAP(m_bufferIndex, newBufferSize);
			m_bufferSize = newBufferSize;
		}
	}

	void OscilloscopeUnit::process_() {
		double phase = getInputValue(m_iPhase);
		if (phase - m_lastPhase < -0.5) {
			_sync();
		}
		m_lastSync++;
		m_lastPhase = phase;
		for (int i = 0; i < m_numBuffers; i++) {
			m_buffers[i][m_bufferIndex] = getInputValue(i);
		}
		m_bufferIndex = syn::WRAP(m_bufferIndex + 1, m_bufferSize);
	}

	void OscilloscopeUnit::_sync() {
		m_syncCount++;
		if (m_syncCount >= getParameter(NumPeriods).getInt()) {
			setParameterValue(BufferSize, m_lastSync);
			m_bufferIndex = 0;
			m_lastSync = 0;
			m_syncCount = 0;
		}
	}

	OscilloscopeUnitControl::OscilloscopeUnitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId) :
		UIUnitControl{ a_window, a_vm, a_unitId },
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
		m_col->setSelfMinSizePolicy(UICell::SNONE);
		m_col->setGreedyChild(m_plot);
		addChild(m_col);
		
		m_plot->setStatusLabel(m_statusLabel);
		m_plot->setInterpPolicy(UIPlot::LinInterp);

		setMinSize(minSize().cwiseMax(m_col->minSize()));
	}

	void OscilloscopeUnitControl::_onResize() {
		m_col->setSize(size());
	}

	void OscilloscopeUnitControl::draw(NVGcontext* a_nvg) {
		const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()));
		m_plot->setNumBuffers(unit->getNumBuffers());
		for (int i = 0; i < unit->getNumBuffers(); i++) {
			m_plot->setBufferPtr(i,unit->getBufferPtr(i), unit->getBufferSize(i));
		}
		
		nvgBeginPath(a_nvg);
		nvgFillColor(a_nvg, Color(0.3f, 1.0f));
		Vector2i lPos = m_statusLabel->getRelPos();
		Vector2i lSize = m_statusLabel->size();
		nvgRect(a_nvg, lPos.x(), lPos.y(), lSize.x(), lSize.y());
		nvgFill(a_nvg);

	}
}
