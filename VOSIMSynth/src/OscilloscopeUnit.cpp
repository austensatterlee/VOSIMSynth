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

namespace syn
{
	OscilloscopeUnit::OscilloscopeUnit(const string& a_name) :
		Unit(a_name),
		m_bufferIndex(0),
		m_pBufferSize(addParameter_(UnitParameter("buffer size", 16, 96000, 256))),
		m_pNumPeriods(addParameter_(UnitParameter("periods", 1, 16, 1))),
		m_lastPhase(0.0),
		m_lastSync(0),
		m_syncCount(0) 
	{
		addInput_("in");
		m_iPhase = addInput_("ph");
		m_bufferSize = getParameter(m_pBufferSize).getInt();
		m_buffer.resize(getParameter(m_pBufferSize).getMax());
	}

	const double* OscilloscopeUnit::getBufferPtr() const {
		return &m_buffer[0];
	}

	int OscilloscopeUnit::getBufferSize() const {
		return m_bufferSize;
	}

	void OscilloscopeUnit::onParamChange_(int a_paramId) {
		if (a_paramId == m_pBufferSize) {
			int newBufferSize = getParameter(m_pBufferSize).getInt();
			int oldBufferSize = m_bufferSize;
			m_bufferIndex = WRAP(m_bufferIndex, newBufferSize);
			m_bufferSize = newBufferSize;
			if (m_bufferSize > oldBufferSize) {
				m_buffer.resize(m_bufferSize);
			}
		}
	}

	void OscilloscopeUnit::process_() {
		double phase = getInputValue(m_iPhase);
		if (phase - m_lastPhase < -0.5) {
			_sync();
		}
		m_lastSync++;
		m_lastPhase = phase;
		m_buffer[m_bufferIndex] = getInputValue(0);		
		m_bufferIndex = WRAP(m_bufferIndex + 1, m_bufferSize);
	}

	void OscilloscopeUnit::_sync() {
		m_syncCount++;
		if (m_syncCount >= getParameter(m_pNumPeriods).getInt()) {
			setParameter(m_pBufferSize, m_lastSync);
			m_bufferIndex = 0;
			m_lastSync = 0;
			m_syncCount = 0;
		}
	}

	OscilloscopeUnitControl::OscilloscopeUnitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId) :
		UIUnitControl{ a_window, a_vm, a_unitId },
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

		addChild(m_resizeHandle);
		m_resizeHandle->setDragCallback([&](const Vector2i& a_relPos, const Vector2i& a_diffPos) {
			grow(a_diffPos);
			m_plot->grow(a_diffPos);
		});

		setMinSize(minSize().cwiseMax(m_col->minSize()));
	}

	void OscilloscopeUnitControl::_onResize() {
		m_col->setSize(size());
		m_resizeHandle->setRelPos(size() - m_resizeHandle->size());
	}

	void OscilloscopeUnitControl::draw(NVGcontext* a_nvg) {
		const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId, m_vm->getNewestVoiceIndex()));
		m_plot->setBufferPtr(unit->getBufferPtr(), unit->getBufferSize());
	}
}
