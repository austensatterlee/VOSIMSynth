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

#ifndef __UI__
#define __UI__
#include "NDPoint.h"
#include <IPlug/IPlugStructs.h>
#include <DSPMath.h>

#define SCREEN_PAD_L 25
#define X_PAD 10
#define Y_PAD 32
#define CELL_SIZE 32

using namespace std;

namespace syn
{
	class ColorPoint : public NDPoint<4>
	{
	public:
		ColorPoint(unsigned int word) :
			NDPoint<4>(double((word >> 24) & 0xFF) * 1.0 / 0xFF, double((word >> 16) & 0xFF) * 1.0 / 0xFF, double((word >> 8) & 0xFF) * 1.0 / 0xFF, double(word & 0xFF) * 1.0 / 0xFF) {}

		ColorPoint(const NDPoint<4>& pt) :
			NDPoint<4>(pt) { }

		IColor getIColor() const {
			return IColor(int(m_pvec[0] * 0xFF), int(m_pvec[1] * 0xFF), int(m_pvec[2] * 0xFF), int(m_pvec[3] * 0xFF));
		}
	};

	inline IRECT makeIRectFromPoints(int x1, int y1, int x2, int y2) {
		int L = MIN(x1, x2);
		int R = MAX(x1, x2);
		int T = MIN(y1, y2);
		int B = MAX(y1, y2);
		return{ L,T,R,B };
	}

	enum PALETTE
	{
		PRIMARY = 0,
		SECONDARY,
		TERTIARY,
		COMPLEMENT
	};

	const ColorPoint globalPalette[4][5] = {
		{
			0x3F51B5,
			0x5C6BC0,
			0x7986CB,
			0x9FA8DA,
			0xC5CAE9,
		},
		{
			0x2196F3,
			0x42A5F5,
			0x64B5F6,
			0x90CAF9,
			0xBBDEFB
		},
		{
			0x4CAF50,
			0x66BB6A,
			0x81C784,
			0xA5D6A7,
			0xC8E6C9
		},
		{
			0x80D8FF,
			0x40C4FF,
			0x00B0FF,
			0x0091EA,
			0x01579B
		}
	};
}
#endif

