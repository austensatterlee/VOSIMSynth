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

#pragma once
#include <nanogui/theme.h>

namespace synui {    
    class VOSIMTheme : public nanogui::Theme {
    public:
        VOSIMTheme(NVGcontext* ctx)
            : Theme(ctx) 
        {
            mProperties["/SummerUnitWidget/bgColor"_json_pointer] = nanogui::Color(140, 73, 191, 255);
            mProperties["/SummerUnitWidget/fgColor"_json_pointer] = nanogui::Color(3, 88, 88, 255);
            mProperties["/GainUnitWidget/bgColor"_json_pointer] = nanogui::Color(127, 32, 11, 255);
            mProperties["/GainUnitWidget/fgColor"_json_pointer] = nanogui::Color(32, 33, 68, 255);
        }
    };
}