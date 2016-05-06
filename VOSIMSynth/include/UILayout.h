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
 *  \file UILayout.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 05/2016
 */

#ifndef __UILAYOUT__
#define __UILAYOUT__
#include "UIComponent.h"

namespace syn {
	class UILayout : public UIComponent
	{
	public:
		UILayout(VOSIMWindow* a_window)
			: UIComponent(a_window) {}

		void pack();

	};
}


#endif
