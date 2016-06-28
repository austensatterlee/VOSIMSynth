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
#include "UIUnitControl.h"
#include "UIDefaultUnitControl.h"

using std::vector;
using std::array;

namespace syn
{
	class SpectroscopeUnit : public Unit
	{
	public:
		SpectroscopeUnit(const string& a_name);

		SpectroscopeUnit(const SpectroscopeUnit& a_rhs) :
			SpectroscopeUnit(a_rhs.getName()) {}

		size_t getBufferSize() const {
			return m_outBufferSize;
		}

		const double* getBufferPtr() const {
			return &m_outBuffer[0];
		}

	protected:
		void onParamChange_(int a_paramId) override;
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override {
			return "SpectroscopeUnit";
		};

		Unit* _clone() const override {
			return new SpectroscopeUnit(*this);
		};

	private:
		friend class SpectroscopeUnitControl;
		int m_bufferIndex;
		int m_inBufferSize, m_outBufferSize;
		int m_pBufferSize, m_pTimeSmooth, m_pUpdatePeriod;

		int m_samplesSinceLastUpdate;

		vector<double> m_timeBuffer;
		vector<WDL_FFT_COMPLEX> m_freqBuffer;
		vector<double> m_outBuffer;
		vector<double> m_window;
		bool m_isActive;
	};

	class SpectroscopeUnitControl : public UIUnitControl
	{
	public:
		SpectroscopeUnitControl(MainWindow* a_window, VoiceManager* a_vm, int a_unitId); 

	protected:
		void draw(NVGcontext* a_nvg) override;

	private:
		void _onResize() override;
	private:
		UICol* m_col;
		UILabel* m_statusLabel;
		UIPlot* m_plot;
		UIDefaultUnitControl* m_defCtrl;
	};
}
