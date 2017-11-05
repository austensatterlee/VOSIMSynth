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

#include <nanogui/glcanvas.h>

namespace synui {
    class ScopeGL : public nanogui::GLCanvas {
    public:
        ScopeGL(Widget* parent);

        virtual ~ScopeGL();

        void setValues(const Eigen::MatrixXf& a_values) { m_values = a_values; }

        void setColor(const nanogui::Color& a_color) { m_color = a_color; }

        void draw(NVGcontext* ctx) override;

        void drawGL() override;

    private:
        nanogui::GLShader m_shader;
        Eigen::MatrixXf m_values;
        nanogui::Color m_color;
    };
}