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
#include "vosimsynth/widgets/UnitWidget.h"
#include <nanogui/screen.h>

namespace synui {
    class SummerUnitWidget : public UnitWidget {

    public:
        SummerUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId);

        void draw(NVGcontext* ctx) override;

        Eigen::Vector2i getInputPortAbsPosition(int a_portId) override;
        Eigen::Vector2i getOutputPortAbsPosition(int a_portId) override;
        int getInputPort(const Eigen::Vector2i& p) override;
        int getOutputPort(const Eigen::Vector2i& p) override;

        Eigen::Vector2i preferredSize(NVGcontext* ctx) const override;

        bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;

        bool isHandleSelected(const Eigen::Vector2i& p) const;
        bool isOutputSelected(const Eigen::Vector2i& p) const;

    protected:
        void onGridChange_() override {}

    protected:
        float m_handleRadiusRatio;
    };
}
