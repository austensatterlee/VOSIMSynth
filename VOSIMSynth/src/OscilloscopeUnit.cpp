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
#include <DSPMath.h>

namespace syn
{
	OscilloscopeUnit::OscilloscopeUnit(const string& a_name) :
		Unit(a_name),
		m_bufferIndex(0),
		m_nBuffers(2),
		m_pBufferSize(addParameter_(UnitParameter("buffer size", 2, 16384, 256))),
		m_pNumPeriods(addParameter_(UnitParameter("periods", 1, 16, 1))),
		m_lastPhase(0.0),
		m_lastSync(0),
		m_syncCount(0) {
		addInput_("in1");
		addInput_("in2");
		m_iPhase = addInput_("ph");
		m_buffers.resize(m_nBuffers);
		m_bufferSize = getParameter(m_pBufferSize).getInt();
		for (int i = 0; i < m_nBuffers; i++) {
			m_buffers[i].resize(m_bufferSize);
		}
	}

	void OscilloscopeUnit::onParamChange_(int a_paramId) {
		if (a_paramId == m_pBufferSize) {
			int newBufferSize = getParameter(m_pBufferSize).getInt();
			int oldBufferSize = m_bufferSize;
			m_bufferIndex = MIN(m_bufferIndex,newBufferSize);
			m_bufferSize = newBufferSize;
			if (m_bufferSize > oldBufferSize) {
				for (int i = 0; i < m_nBuffers; i++) {
					m_buffers[i].resize(m_bufferSize);
				}
			}
		}
	}

	void OscilloscopeUnit::process_(const SignalBus& a_inputs, SignalBus& a_outputs) {
		double phase = a_inputs.getValue(m_iPhase);
		if (1 - (m_lastPhase - phase) < 0.5) {
			_sync();
		}
		m_lastSync++;
		m_lastPhase = phase;
		for (int i = 0; i < m_nBuffers; i++) {
			m_buffers[i][m_bufferIndex] = a_inputs.getValue(i);
		}
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

	OscilloscopeUnitControl::OscilloscopeUnitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId):
		UIUnitControl{a_window, a_vm, a_unitId},
		m_defCtrl(new DefaultUnitControl(a_window, a_vm, a_unitId)),
		m_resizeHandle(new UIResizeHandle(a_window)),
		m_yBounds{-1,1},
		m_screenSize{0,0},
		m_screenPos{0,0} {
		addChild(m_defCtrl);
		addChild(m_resizeHandle);
		m_resizeHandle->setDragCallback([&](const Vector2i& a_relPos, const Vector2i& a_diffPos) {
			grow(a_diffPos);			
		});

		Vector2i minSize{200,100};
		Vector2i defCtrlSize = m_defCtrl->minSize();
		minSize[1] += defCtrlSize[1];
		minSize[0] = MAX(minSize[0], defCtrlSize[0]);
		setMinSize(minSize);
	}

	Vector2i OscilloscopeUnitControl::toPixCoords(const Vector2f& a_sample) {
		double unitY = (a_sample[1] - m_yBounds[0]) / (m_yBounds[1] - m_yBounds[0]);
		int screenY = -unitY * m_screenSize[1] + m_screenPos[1] + m_screenSize[1];

		const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId));
		double unitX = a_sample[0] / unit->m_bufferSize;
		int screenX = unitX * m_screenSize[0] + m_screenPos[0];

		return {screenX, screenY};
	}

	void OscilloscopeUnitControl::_onResize() {
		m_screenPos = {0,m_defCtrl->size()[1]};
		m_screenSize = size() - Vector2i{0,m_defCtrl->size()[1] + m_resizeHandle->size()[1]};
		m_defCtrl->setSize({ size()[0],-1 });		
		m_resizeHandle->setRelPos(size() - m_resizeHandle->size());	
	}

	void OscilloscopeUnitControl::draw(NVGcontext* a_nvg) {
		const OscilloscopeUnit* unit = static_cast<const OscilloscopeUnit*>(&m_vm->getUnit(m_unitId));

		// Draw background
		nvgBeginPath(a_nvg);
		nvgFillColor(a_nvg, Color(0.0f, 0.85f));
		nvgRect(a_nvg, m_screenPos[0], m_screenPos[1], m_screenSize[0], m_screenSize[1]);
		nvgFill(a_nvg);

		// Draw paths
		for (int i = 0; i < unit->m_nBuffers; i++) {
			Color color = m_colors[i % m_colors.size()];
			nvgBeginPath(a_nvg);
			nvgStrokeColor(a_nvg, color);
			for (int j = 0; j < unit->m_bufferSize; j++) {
				Vector2i screenPt = toPixCoords({j,unit->m_buffers[i][j]});
				if (j == 0)
					nvgMoveTo(a_nvg, screenPt[0], screenPt[1]);
				else
					nvgLineTo(a_nvg, screenPt[0], screenPt[1]);
			}
			nvgStroke(a_nvg);
		}
	}
}
