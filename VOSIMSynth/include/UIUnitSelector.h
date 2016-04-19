/*
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
along with VOSIMProject.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2016, Austen Satterlee
*/

/**
*  \file UIUnitSelector.h
*  \brief
*  \details
*  \author Austen Satterlee
*  \date 04/2016
*/

#ifndef __UIUNITSELECTOR__
#define __UIUNITSELECTOR__
#include "nanovg.h"
#include <UIComponent.h>

namespace syn
{
	class UIUnitSelector : public UIComponent
	{
	public:
		UIUnitSelector(VOSIMWindow* a_window, UIComponent* a_parent)
			: UIComponent{a_window, a_parent} {}
	};
}
#endif