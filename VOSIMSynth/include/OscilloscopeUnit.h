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
#include "UIUnitControl.h"

namespace syn
{
	class OscilloscopeUnit : public Unit
	{
	public:
		explicit OscilloscopeUnit(const string& a_name);

		OscilloscopeUnit(const OscilloscopeUnit& a_rhs) :
			OscilloscopeUnit(a_rhs.getName()) {}

		const double* getBufferPtr() const;
		int getBufferSize() const;

	protected:
		void onParamChange_(int a_paramId) override;
		void MSFASTCALL process_() GCCFASTCALL override;

	private:
		string _getClassName() const override {
			return "OscilloscopeUnit";
		};

		Unit* _clone() const override {
			return new OscilloscopeUnit(*this);
		};

		void _sync();
	private:
		friend class OscilloscopeUnitControl;
		int m_bufferIndex;
		int m_bufferSize;
		int m_pBufferSize;
		int m_pNumPeriods;
		int m_iPhase;

		vector<double> m_buffer;

		double m_lastPhase;
		int m_lastSync;
		int m_syncCount;
	};

	class OscilloscopeUnitControl : public UIUnitControl
	{

	public:
		OscilloscopeUnitControl(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId);

	protected:
		void draw(NVGcontext* a_nvg) override;

	private:
		void _onResize() override;
	private:
		UICol* m_col;
		UILabel* m_statusLabel;
		UIPlot* m_plot;
		DefaultUnitControl* m_defCtrl;
		UIResizeHandle* m_resizeHandle;
	};
}
