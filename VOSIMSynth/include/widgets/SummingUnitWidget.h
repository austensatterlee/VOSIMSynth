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
 *  \file SummingUnitWidget.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 04/2017
 */

#pragma once
#include "UnitWidget.h"

namespace synui {
    class SummingUnitWidget : public synui::UnitWidget
    {

    public:
        SummingUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId);

        void draw(NVGcontext* ctx) override;

        Eigen::Vector2i preferredSize(NVGcontext* ctx) const override;

        Eigen::Vector2i getInputPortAbsPosition(int a_portId) override;

        Eigen::Vector2i getOutputPortAbsPosition(int a_portId) override;

        bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;

        bool isHandleSelected(const Eigen::Vector2i& p) const;

    protected:
        void onGridChange_() override {};

        /// \returns -1 if no ports are selected
        int getSelectedInputPort(const Eigen::Vector2i& p) const;

        /// \returns -1 if no ports are selected
        int getSelectedOutputPort(const Eigen::Vector2i& p) const;

    protected:
        float m_handleRadiusRatio;
    };
}