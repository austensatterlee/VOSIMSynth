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
 *  \file UnitWidget.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once
#include <nanogui/nanogui.h>
#include <common_serial.h>

namespace syn
{
    class VoiceManager;
    class Unit;
}

namespace synui
{
    class CircuitWidget;

    class UnitWidget : public nanogui::Widget
    {
        friend class CircuitWidget;
    public:
        UnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId);
        virtual ~UnitWidget();

        void draw(NVGcontext* ctx) override;
        bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;

        const std::string& getName() const;
        int getUnitId() const { return m_unitId; }

        /**
         * \brief Compute the coordinates of the input port with the given id.
         * \param a_portId The id of the requested input port, corresponding with the id of an input port on the associated syn::Unit.
         * \return Coordinates of the requested input port.
         */
        Eigen::Vector2i getInputPortAbsPosition(int a_portId);

        /**
         * \brief Compute the coordinates of the output port with the given id.
         * \param a_portId The id of the requested output port, corresponding with the id of an output port on the associated syn::Unit.
         * \return Coordinates of the requested output port.
         */
        Eigen::Vector2i getOutputPortAbsPosition(int a_portId);

        Eigen::Vector2i preferredSize(NVGcontext* ctx) const override;
        void performLayout(NVGcontext* ctx) override;

        void setEditorCallback(std::function<void(unsigned, int)> a_callback) { m_editorCallback = a_callback; }
        void triggerEditorCallback() const { m_editorCallback(m_classIdentifier, m_unitId); }

        operator json() const;
        UnitWidget* load(const json& j);

        bool highlighted() const { return m_highlighted; }
        void setHighlighted(bool highlighted) { m_highlighted = highlighted; }

    protected:
        /// Update layout configuration based on parent circuit's grid spacing.
        void updateRowSizes_();
        const syn::Unit& getUnit_() const;
        void setName_(const std::string& a_name);

    protected:
        CircuitWidget* m_parentCircuit;
        syn::VoiceManager* m_vm;

        std::function<void(unsigned, int)> m_editorCallback;

        nanogui::Label* m_titleLabel;
        nanogui::TextBox* m_titleTextBox;
        std::map<int, Widget*> m_inputLabels;
        std::map<int, Widget*> m_outputLabels;
        std::map<int, Widget*> m_emptyInputLabels;
        std::map<int, Widget*> m_emptyOutputLabels;

        double m_lastClickTime;

        enum State
        {
            Uninitialized,
            Idle
        } m_state;

        int m_unitId;
        unsigned m_classIdentifier;

        bool m_highlighted;
    };

    class InputUnitWidget : public UnitWidget
    {
    public:
        InputUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
            : UnitWidget(a_parent, a_vm, a_unitId)
        {
            for (auto lbl : m_inputLabels)
            {
                auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());
                l->removeAnchor(lbl.second);
                removeChild(lbl.second);
            }
            m_inputLabels.clear();
        }
    };

    class OutputUnitWidget : public UnitWidget
    {
    public:
        OutputUnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId)
            : UnitWidget(a_parent, a_vm, a_unitId)
        {
            for (auto lbl : m_outputLabels)
            {
                auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());
                l->removeAnchor(lbl.second);
                removeChild(lbl.second);
            }
            m_outputLabels.clear();
        }
    };
}
