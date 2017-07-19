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
 *  \file DefaultUnitWidget.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 04/2017
 */

#pragma once
#include "vosimsynth/UnitWidget.h"

namespace synui {
    class DefaultUnitWidget : public UnitWidget
    {
    public:
        DefaultUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId);

        void draw(NVGcontext* ctx) override;
        void setName(const std::string& a_name) override;;
        Eigen::Vector2i getInputPortAbsPosition(int a_portId) override;
        Eigen::Vector2i getOutputPortAbsPosition(int a_portId) override;
        int getInputPort(const Eigen::Vector2i& a_pos) override;
        int getOutputPort(const Eigen::Vector2i& a_pos) override;
        Eigen::Vector2i preferredSize(NVGcontext* ctx) const override;

        bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;

    protected:
        void onGridChange_() override;
        nanogui::Label* m_titleLabel;
        nanogui::TextBox* m_titleTextBox;
        std::map<int, nanogui::Label*> m_inputLabels;
        std::map<int, nanogui::Label*> m_outputLabels;
        std::map<int, nanogui::Label*> m_emptyInputLabels;
        std::map<int, nanogui::Label*> m_emptyOutputLabels;
    };

    class InputUnitWidget : public DefaultUnitWidget
    {
    public:
        InputUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId);
    };

    class OutputUnitWidget : public DefaultUnitWidget
    {
    public:
        OutputUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId);
    };
}