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
#include "UnitControl.h"
#include "DSPMath.h"
#include "fft.h"
#include <array>
#include <vector>
#include "DefaultUnitControl.h"

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
		void process_(const SignalBus& a_inputs, SignalBus& a_outputs) override;
		
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
	};

	class SpectroscopeUnitControl : public UnitControl
	{
	public:
		SpectroscopeUnitControl() :
			UnitControl(),
			m_xBounds(0,1),
			m_yBounds(c_defaultYBounds),
			m_paramControl(nullptr),
			m_unit(nullptr)			
		{}

		virtual ~SpectroscopeUnitControl() {
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
			IColor bgcolor{ 235,0,0,0 };
			IRECT rect = { m_pos[0], m_pos[1], m_pos[0] + m_size[0], m_pos[1] + m_size[1] };
			a_graphics->FillIRect(&bgcolor, &rect);			
			m_paramControl->draw(a_graphics);
			updateMinSize_(m_paramControl->getMinSize());
			NDPoint<2,int> paramControlSize = m_paramControl->getMinSize();			

			IRECT screen_rect = { rect.L,rect.T + paramControlSize[1]+1,rect.R,rect.B };

			///\todo optimize
			m_unit = static_cast<const SpectroscopeUnit*>(&m_voiceManager->getUnit(m_unitId));
			m_xBounds = { 0, log2(0.5*m_unit->getFs()) };

			NDPoint<2, double> lastPoint(0,0);
			NDPoint<2, double> currPoint;
			resetYBounds_();
			for (int i = 0; i < m_unit->getNumBuffers(); i++) {
				updateYBounds_(m_unit->getBufferExtrema(i));
			}
			for (int i = 0; i < m_unit->getNumBuffers(); i++) {
				for (int j = 0; j < m_unit->getBufferSize(); j++) {
					currPoint = { log2(j*m_unit->getFs()*0.5/ m_unit->getBufferSize()),m_unit->getBuffer(i)[j] };
					currPoint = toScreen(currPoint,screen_rect);
					if (j>0)
						a_graphics->DrawLine(&c_colors[i], static_cast<float>(lastPoint[0]), static_cast<float>(lastPoint[1]), static_cast<float>(currPoint[0]), static_cast<float>(currPoint[1]), 0, true);
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

		void resetYBounds_() {
			m_yBounds = c_defaultYBounds;
		}

		void updateYBounds_(const NDPoint<2,double>& a_newBounds) {
			m_yBounds[0] = MIN(m_yBounds[0], a_newBounds[0]);
			m_yBounds[1] = MAX(m_yBounds[1], a_newBounds[1]);
		}

	private:
		SpectroscopeUnitControl(IPlugBase* a_plug, const shared_ptr<VoiceManager>& a_vm, int a_unitId, int a_x, int a_y) :
			UnitControl(a_plug, a_vm, a_unitId, a_x, a_y),
			m_xBounds(0, 1),
			m_yBounds(c_defaultYBounds),
			m_unit(nullptr)
		{
			DefaultUnitControl pctrl;
			m_paramControl = pctrl.construct(a_plug, a_vm, a_unitId, a_x, a_y);
		}

		UnitControl* _construct(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_x, int a_y) const override 
		{
			return new SpectroscopeUnitControl(a_plug, a_vm, a_unitId, a_x, a_y);
		};

		NDPoint<2,int> toScreen(NDPoint<2,double> a_pt, const IRECT& a_screen) {
			a_pt[0] = CLAMP(a_pt[0], m_xBounds[0], m_xBounds[1]);
			a_pt[1] = CLAMP(a_pt[1], m_yBounds[0], m_yBounds[1]);
			
			NDPoint<2, int> screenPt;
			screenPt[0] = (a_pt[0] - m_xBounds[0]) / (m_xBounds[1] - m_xBounds[0]) * a_screen.W() + a_screen.L;
			screenPt[1] = a_screen.B - (a_pt[1] - m_yBounds[0]) / (m_yBounds[1] - m_yBounds[0]) * a_screen.H();
			return screenPt;
		}
	private:
		NDPoint<2, double> m_xBounds,m_yBounds;
		const NDPoint<2, double> c_defaultYBounds = { -60.0,6.0 };
		UnitControl* m_paramControl;
		const IColor c_colors[2] = { {255,255,255,255},{255,128,128,255} };
		const SpectroscopeUnit* m_unit;
	};
}
