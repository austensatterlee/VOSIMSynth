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

#include "CircuitGraphics.h"
#include "Alloy.h"

using namespace aly;

namespace syn
{
	bool CircuitGraphics::init(Composite& a_rootNode) {
		HSliderPtr hslider1 = HSliderPtr(
			new HorizontalSlider("Integer Slider", CoordPerPX(0.1f, 0.3f, 0, 0),
				CoordPX(200.0f, 40.0f), false, Integer(1), Integer(12),
				Integer(7)));
		a_rootNode.add(hslider1);
		return true;
	}
}