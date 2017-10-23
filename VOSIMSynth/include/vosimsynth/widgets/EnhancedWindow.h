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
#include <nanogui/window.h>

namespace synui {
    class EnhancedWindow : public nanogui::Window {
    public:
        typedef std::function<void(EnhancedWindow*, NVGcontext*)> DrawFunc;

        EnhancedWindow(Widget* a_parent, const std::string& a_title);

        bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;

        void draw(NVGcontext* ctx) override;

        nanogui::Button* createOpenButton(Widget* a_parent, const std::string& text = "", int icon = 0,
                                          std::function<void()> a_callback = nullptr);

        void setDrawCallback(DrawFunc f);
        DrawFunc getDrawCallback() const;
    private:
        DrawFunc m_drawCallback;
    };
}
