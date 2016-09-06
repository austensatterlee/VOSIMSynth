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
#include "Unit.h"
#include "UIUnitControl.h"

namespace synui
{
	class OscilloscopeUnit : public syn::Unit
	{
		DERIVE_UNIT(OscilloscopeUnit)
	public:
		enum Parameter
		{
			BufferSize,
			NumPeriods
		};

		explicit OscilloscopeUnit(const string& a_name);

		OscilloscopeUnit(const OscilloscopeUnit& a_rhs) :
			OscilloscopeUnit(a_rhs.getName()) {}

		int getNumBuffers() const;
		const double* getBufferPtr(int a_bufIndex) const;
		int getBufferSize(int a_bufIndex) const;

	protected:
		void onParamChange_(int a_paramId) override;
		void MSFASTCALL process_() GCCFASTCALL override;

	private:

		void _sync();
	private:
		friend class OscilloscopeUnitControl;
		int m_bufferIndex;
		int m_bufferSize;
		int m_iPhase;

		int m_numBuffers;
		vector<vector<double>> m_buffers;

		double m_lastPhase;
		int m_lastSync;
		int m_syncCount;
	};

	class OscilloscopeUnitControl : public UIUnitControl
	{
	public:
		OscilloscopeUnitControl(MainWindow* a_window, syn::VoiceManager* a_vm, int a_unitId);

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