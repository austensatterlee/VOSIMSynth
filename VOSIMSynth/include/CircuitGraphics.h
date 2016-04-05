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
* \file CircuitPanel.h
* \brief
* \details
* \author Austen Satterlee
* \date March 6, 2016
*/
#ifndef __CIRCUITPANEL__
#define __CIRCUITPANEL__
  
#include "AlloyApplication.h"

using namespace std;

namespace syn {

	class CircuitGraphics : public aly::Application {
	public:
		CircuitGraphics(int a_width, int a_height) :
			Application(a_width, a_height, "Circuit")
			{};
		bool init(aly::Composite& a_rootNode) override;
	};
}


#endif
