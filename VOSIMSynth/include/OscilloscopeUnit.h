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
 *	 \file OscilloscopeUnit.h
 *   \brief 
 *   \details
 *   \author Austen Satterlee
 *   \date 02/15/2016
 */
#pragma once
#include <Unit.h>
#include <UnitControl.h>
#include <DSPMath.h>
#include "DefaultUnitControl.h"

namespace syn {
	class OscilloscopeUnit : public Unit
	{	
	public:
		OscilloscopeUnit(const string& a_name);

		OscilloscopeUnit(const OscilloscopeUnit& a_rhs) :
			OscilloscopeUnit(a_rhs.getName())
		{}
	protected:
		void onParamChange_(int a_paramId) override;
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
		
	private:
		string _getClassName() const override { return "OscilloscopeUnit"; };
		Unit* _clone() const override { return new OscilloscopeUnit(*this);  };
		void _sync();
	private:
		friend class OscilloscopeUnitControl;
		int m_bufferIndex;
		int m_bufferSize;
		int m_nBuffers;
		int m_pBufferSize;
		int m_pNumPeriods;
		int m_iPhase;
		
		vector<vector<double> > m_buffers;

		double m_lastPhase;
		int m_lastSync;
		int m_syncCount;
	};

	class OscilloscopeUnitControl : public UnitControl
	{
	public:
		OscilloscopeUnitControl() :
			UnitControl(),
			m_yBounds(-1.0, 1.0),
			m_paramControl(nullptr)
		{}

		virtual ~OscilloscopeUnitControl() {
			if(m_paramControl!=nullptr)
				delete m_paramControl;
		}

		void onMouseDblClick(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			m_paramControl->onMouseDblClick(a_x, a_y, a_mouseMod);
		}
		void onMouseDown(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			m_paramControl->onMouseDown(a_x, a_y, a_mouseMod);
		}
		void onMouseUp(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			m_paramControl->onMouseUp(a_x, a_y, a_mouseMod);
		}
		void onMouseDrag(int a_x, int a_y, int a_dX, int a_dY, IMouseMod* a_mouseMod) override {
			m_paramControl->onMouseDrag(a_x, a_y, a_dX, a_dY, a_mouseMod);
		}

		void onMouseWheel(int a_x, int a_y, IMouseMod* a_mouseMod, int a_d) override { 
			m_paramControl->onMouseWheel(a_x, a_y, a_mouseMod, a_d);
		}

		void onMouseOver(int a_x, int a_y, IMouseMod* a_mouseMod) override {
			m_paramControl->onMouseOver(a_x, a_y, a_mouseMod);
		}


		void draw(IGraphics* a_graphics) override 
		{
			// Draw background
			IRECT rect = { m_pos[0], m_pos[1], m_pos[0] + m_size[0], m_pos[1] + m_size[1] };
			a_graphics->FillIRect(&COLOR_BLACK, &rect);			
			m_paramControl->draw(a_graphics);
			NDPoint<2, int> paramControlSize = m_paramControl->getMinSize();
			updateMinSize_(paramControlSize+NDPoint<2,int>(50,50));

			IRECT screen_rect = { rect.L,rect.T + paramControlSize[1]+1,rect.R,rect.B };
			const OscilloscopeUnit& unit = static_cast<const OscilloscopeUnit&>(m_voiceManager->getUnit(m_unitId));
			NDPoint<2, int> lastPoint(0,0);
			NDPoint<2, int> currPoint;
			for (int i = 0; i < unit.m_nBuffers; i++) {
				for (int j = 0; j < unit.m_buffers[i].size(); j++) {
					currPoint = toScreen({ static_cast<double>(j),unit.m_buffers[i][j] }, screen_rect);
					if (j>0)
						a_graphics->DrawLine(&m_colors[i], static_cast<float>(lastPoint[0]), static_cast<float>(lastPoint[1]), static_cast<float>(currPoint[0]), static_cast<float>(currPoint[1]), 0, true);
					lastPoint = currPoint;
				}
			}
		}
		int getSelectedParam(int a_x, int a_y) override {
			return m_paramControl->getSelectedParam(a_x, a_y);
		}
	protected:
		void onSetUnitId_() override{
			m_paramControl->setUnitId(m_unitId);
		};
		void onChangeRect_() override {
			m_paramControl->move(m_pos);
			m_paramControl->resize({ m_size[0]-1,0 });
		}
	private:

		OscilloscopeUnitControl(IPlugBase* a_plug, const shared_ptr<VoiceManager>& a_vm, int a_unitId, int a_x, int a_y) :
			UnitControl(a_plug, a_vm, a_unitId, a_x, a_y),
			m_yBounds(-1.0, 1.0)
		{
			DefaultUnitControl pctrl;
			m_paramControl = pctrl.construct(a_plug, a_vm, a_unitId, a_x, a_y);
		}

		UnitControl* _construct(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y) const override 
		{
			return new OscilloscopeUnitControl(a_plug, a_vm, a_unitId, a_x, a_y);
		};

		NDPoint<2,int> toScreen(NDPoint<2,double> a_pt, const IRECT& a_screen) {
			const OscilloscopeUnit& unit = static_cast<const OscilloscopeUnit&>(m_voiceManager->getUnit(m_unitId));
			NDPoint<2, double> xBounds(0, static_cast<double>(unit.m_bufferSize));
			a_pt[0] = CLAMP(a_pt[0], xBounds[0], xBounds[1]);
			a_pt[1] = CLAMP(a_pt[1], m_yBounds[0], m_yBounds[1]);
			
			NDPoint<2, int> screenPt;
			screenPt[0] = (a_pt[0] - xBounds[0]) / (xBounds[1] - xBounds[0]) * a_screen.W() + a_screen.L;
			screenPt[1] = (-a_pt[1] - m_yBounds[0]) / (m_yBounds[1] - m_yBounds[0]) * a_screen.H() + a_screen.T;
			return screenPt;
		}
	private:
		NDPoint<2, double> m_yBounds;
		UnitControl* m_paramControl;
		IColor m_colors[2] = { {255,255,255,255},{255,128,128,255} };
	};
}
