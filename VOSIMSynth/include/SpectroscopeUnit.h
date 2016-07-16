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
#include <ffts.h>
#include <complex>
#include <array>
#include <vector>
#include "UIUnitControl.h"
#include "UIDefaultUnitControl.h"

using std::vector;
using std::array;
using std::complex;

namespace synui
{
	class SpectroscopeUnit : public syn::Unit
	{
		DERIVE_UNIT(SpectroscopeUnit)
	public:
		SpectroscopeUnit(const string& a_name);

		SpectroscopeUnit(const SpectroscopeUnit& a_rhs) :
			SpectroscopeUnit(a_rhs.getName()) {}

		size_t getBufferSize() const {
			return m_outBufferSize;
		}

		const double* getBufferPtr() const {
			return &m_magnitudeBuffer[0];
		}

		void setDirty()
		{
			m_isStale = false;
		}

	protected:
		void onParamChange_(int a_paramId) override;
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		friend class SpectroscopeUnitControl;
		int m_bufferIndex;
		int m_inBufferSize, m_outBufferSize;
		int m_pBufferSize, m_pTimeSmooth, m_pUpdatePeriod;

		int m_samplesSinceLastUpdate;

		vector<float> m_timeDomainBuffer;
		vector<complex<float> > m_freqDomainBuffer;
		vector<double> m_magnitudeBuffer;
		vector<double> m_window;
		bool m_isStale;
	};

	class SpectroscopeUnitControl : public UIUnitControl
	{
	public:
		SpectroscopeUnitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId);

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

CEREAL_REGISTER_TYPE(synui::SpectroscopeUnit)
