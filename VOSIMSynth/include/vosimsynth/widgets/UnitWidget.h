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
#include "vosimsynth/common.h"
#include "vosimsynth/Signal.h"
#include <vosimlib/Unit.h>
#include <nanogui/widget.h>
#include "CircuitWidget.h"

namespace syn {
    class VoiceManager;
    class Unit;
}

namespace synui {
    class CircuitWidget;

    class UnitWidget : public nanogui::Widget {
        friend class CircuitWidget;
    public:
        UnitWidget(CircuitWidget* a_parent, syn::VoiceManager* a_vm, int a_unitId);
        virtual ~UnitWidget();

        const std::string& getName() const;
        virtual void setName(const std::string& a_name);
        int getUnitId() const { return m_unitId; }

        /**
         * \brief Compute the coordinates of the input port with the given id.
         * \param a_portId The id of the requested input port, corresponding with the id of an input port on the associated syn::Unit.
         * \return Coordinates of the requested input port.
         */
        virtual Eigen::Vector2i getInputPortAbsPosition(int a_portId) = 0;

        /**
         * \returns The id of the input port containing the given position. If none, return -1.
         */
        virtual int getInputPort(const Eigen::Vector2i& a_pos) = 0;

        /**
         * \brief Compute the coordinates of the output port with the given id.
         * \param a_portId The id of the requested output port, corresponding with the id of an output port on the associated syn::Unit.
         * \return Coordinates of the requested output port.
         */
        virtual Eigen::Vector2i getOutputPortAbsPosition(int a_portId) = 0;

        /**
         * \returns The id of the output port containing the given position. If none, return -1.
         */
        virtual int getOutputPort(const Eigen::Vector2i& a_pos) = 0;

        void setEditorCallback(std::function<void(syn::UnitTypeId, int)> a_callback) { m_editorCallback = a_callback; }
        void triggerEditorCallback() const { m_editorCallback(m_classIdentifier, m_unitId); }

        operator json() const;
        UnitWidget* load(const json& j);

        bool highlighted() const { return m_highlighted; }
        void setHighlighted(bool highlighted) { m_highlighted = highlighted; }

    protected:
        /// Update layout configuration based on parent circuit's grid spacing.
        virtual void onGridChange_() = 0;
        const syn::Unit& getUnit_() const;
        bool promptForDelete_();

    
    protected:
        CircuitWidget* m_parentCircuit;
        syn::VoiceManager* m_vm;

        std::function<void(syn::UnitTypeId, int)> m_editorCallback;

        int m_unitId;
        syn::UnitTypeId m_classIdentifier;
        std::string m_name;

        bool m_highlighted;
    };
}
