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
 *  \file UIPlot.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 06/2016
 */

#ifndef __UIPLOT__
#define __UIPLOT__
#include "UIComponent.h"

namespace syn {
	class UIPlot : public UIComponent
	{
	public:
		enum InterpPolicy
		{
			LinInterp = 0,
			SincInterp
		};

		enum XScale
		{
			LinScale = 0,
			LogScale2,
			LogScale10
		};
	public:
		UIPlot(MainWindow* a_window);

		void setBufferPtr(const double* a_yBufPtr, int a_bufSize);

		/**
		 * Set minimum and maximum X sample values
		 */
		void setXBounds(const Vector2f& a_xBounds);
		/**
		* Set minimum and maximum Y sample values
		*/
		void setYBounds(const Vector2f& a_yBounds);

		void setInterpPolicy(InterpPolicy a_policy);
		void setXScale(XScale a_xScale) {
			m_xScale = a_xScale;
		}

		bool onMouseMove(const UICoord& a_relCursor, const Vector2i& a_diffCursor) override;

		void setXUnits(const string& a_xUnits);
		void setYUnits(const string& a_yUnits);

		Vector2i toScreenCoords(const Vector2f& a_sampleCoords) const;

		Vector2f toSampleCoords(const Vector2i& a_screenCoords) const;

		void setStatusLabel(UILabel* a_statusLabel);

	protected:
		void draw(NVGcontext* a_nvg) override;

	private:
		void _onResize() override;

		void _updateYBounds(const Vector2f& a_yExtrema);

		void _updateStatusLabel() const;

	private:
		Vector2f m_minBounds, m_maxBounds;
		double m_autoAdjustSpeed;

		const double* m_yBufPtr;
		int m_bufSize;
		int m_nScreenPts;
		Vector2i m_crosshairPos;
		string m_xUnits, m_yUnits;

		UILabel* m_statusLabel;

		InterpPolicy m_interpPolicy;
		XScale m_xScale;
	};
}

#endif
