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
 *  \file CircuitWidget.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once
#include "Grid.h"
#include <nanogui/nanogui.h>
#include <common_serial.h>

namespace syn
{
    class VoiceManager;
    class UnitFactory;
    class Unit;
}

namespace synui
{
    class MainWindow;
    class UnitWidget;
    class UnitEditorHost;
    class UnitEditor;
}

namespace synui
{
    class CircuitWire;
    class CircuitWidget;

    typedef std::function<UnitWidget*(CircuitWidget*, syn::VoiceManager*, int)> UnitWidgetConstructor;

    class CircuitWidget : public nanogui::Widget
    {
        friend class UnitWidget;
        friend class SummingUnitWidget;
        friend class CircuitWire;
        typedef std::pair<int, int> Port;
    public:
        struct GridCell
        {
            friend bool operator==(const GridCell& a_lhs, const GridCell& a_rhs)
            {
                return a_lhs.state == a_rhs.state && (a_lhs.state == Empty || a_lhs.ptr == a_rhs.ptr);
            }

            friend bool operator!=(const GridCell& a_lhs, const GridCell& a_rhs) { return !(a_lhs == a_rhs); }

            operator bool() const
            {
                return state != Empty;
            }

            enum
            {
                Empty,
                Wire,
                Unit
            } state;
            void* ptr;
        };

        enum WireDrawStyle
        {
            Straight = 0,
            Curved
        };

        /**
         * \brief Displays and manipulates an audio circuit.
         * \param a_parent Parent widget.
         * \param a_mainWindow Serves as an interface for queueing messages to the real-time thread.
         * \param a_unitEditorHost The widget that will host unit editor interfaces.
         * \param a_vm
         * \param a_uf
         */
        CircuitWidget(nanogui::Widget* a_parent, synui::MainWindow* a_mainWindow, synui::UnitEditorHost* a_unitEditorHost, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf);
        virtual ~CircuitWidget();

        bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;
        bool mouseMotionEvent(const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) override;

        void draw(NVGcontext* ctx) override;

        /**
         * \brief Construct a new syn::Unit and its corresponding synui::UnitWidget, and prepare to place the
         * synui::UnitWidget somewhere on the grid. The widget will be placed on the grid with the next left
         * click on a valid location.
         * \param a_unitPrototype The prototype name of the unit to create. Corresponds to a name registered with the syn::UnitFactory.
         */
        void loadPrototype(const std::string& a_unitPrototype);

        /**
         * \brief Snaps the pixel coordinate to one that lies on the grid.
         */
        Eigen::Vector2i fixToGrid(const Eigen::Vector2i& a_pixelLocation) const;

        int getGridSpacing() const { return m_gridSpacing; }

        syn::VoiceManager* voiceManager() const { return m_vm; }

        WireDrawStyle wireDrawStyle() const { return m_wireDrawStyle; }
        void setWireDrawStyle(WireDrawStyle a_newStyle) { m_wireDrawStyle = a_newStyle; }

        template<typename UnitType>
        void registerUnitWidget(UnitWidgetConstructor a_func) { m_registeredUnitWidgets[UnitType("").getClassIdentifier()] = a_func; }

        void performLayout(NVGcontext* ctx) override;
        void resizeGrid(int a_newGridSpacing);

        operator json() const;
        CircuitWidget* load(const json& j);
        void reset();

        /**
         * \brief Attempt to update the position of an existing unit and record the change if successful.
         * This method should be used to move units instead of interacting with the unit widget directly.
         * This method sets the position of the unit widget and updates the internal
         * grid to reflect the change in occupied and unoccupied cells.
         * \param a_newPos The new position (in pixels).
         * \param a_force Perform the update even if the unit already occupies that position.
         */
        void updateUnitPos(UnitWidget* a_unitWidget, const Eigen::Vector2i& a_newPos, bool a_force = false);

        /**
         * \brief Checks if a unit would intersect another unit if placed at the specified position.
         */
        bool checkUnitPos(UnitWidget* a_unitWidget, const Eigen::Vector2i& a_newPos);

    protected:

        /**
         * \brief Create widgets for input/output units.
         */
        void createInputOutputUnits_();

        /**
         * \brief Sends a request to the real-time thread to create a new unit.
         * \param a_unitPrototypeName Prototype name of the unit to create. Corresponds to a prototype name in the unit factory.
         */
        void createUnit_(const std::string& a_unitPrototypeName);

        /**
         * \brief Called upon successful creation of a unit on the real-time thread. Creates the associated
         * unit widget and places the CircuitWidget in the `CreatingUnit` state.
         * \param a_unitId Id of the newly created unit.
         */
        void onUnitCreated_(unsigned a_classId, int a_unitId);

        /**
         * \brief Creates a unit widget of the specified classId and associate it with the given unitId.
         */
        UnitWidget* createUnitWidget_(unsigned a_classId, int a_unitId);

        /**
         * \brief Try to end the `CreatingUnit` state by setting the position of the widget being placed.
         * \returns true if unit was placed, false if the position was not valid.
         */
        bool endCreateUnit_(const Eigen::Vector2i& a_pos);

        void deleteUnit_(int a_unitId);

        void deleteConnection_(const Port& a_inputPort, const Port& a_outputPort);

        /**
         * \brief Sends a message to the real-time thread to create a new connection between two units.
         */
        void createConnection_(const Port& a_inputPort, const Port& a_outputPort);

        /**
         * \brief End the `DrawingWire` state.
         */
        void onConnectionCreated_(const Port& a_inputPort, const Port& a_outputPort);

        /**
         * \brief Place the CircuitWidget in the `DrawingWire` state.
         * \param a_isOutput Whether or not the specified port is an output port.
         */
        void startWireDraw_(int a_unitId, int a_portId, bool a_isOutput);

        /**
         * \brief Finishes the wire currently being drawn and attempts to create the connection on the real-time thread if the wire is valid.
         * \param a_isOutput Whether or not the specified port is an output port.
         */
        void endWireDraw_(int a_unitId, int a_portId, bool a_isOutput);

        /**
         * \brief Place the CircuitWidget in the `MovingUnit` state.
         * During this state, all unit widgets in the selection set are moved.
         * \param a_start The location of the mouse; used as a reference point for determining movement amounts.
         */
        void startUnitMove_(const Eigen::Vector2i& a_start);

        /**
         * \brief Tentatively update the positions of the units being moved for visual feedback.
         * Should only be called while in the `MovingUnit` state.
         * \param a_current The current location of the mouse.
         */
        void updateUnitMove_(const Eigen::Vector2i& a_current);

        /**
         * \brief Try to commit the unit widgets to their new positions. Ends the `MovingUnit` state.
         * Should only be called while in the `MovingUnit` state.
         * \param a_end The current location of the mouse.
         */
        void endUnitMove_(const Eigen::Vector2i& a_end);

    private:
        /**
         * \brief Delete the given wire widget.
         * Note that this method only affects the GUI. The actual connection on the real-time thread is not affected.
         */
        void _deleteWireWidget(CircuitWire* wire);
        /**
         * \brief Delete the given unit widget.
         * Note that this method only affects the GUI. The actual connection on the real-time thread is not affected.
         */
        void _deleteUnitWidget(UnitWidget* widget);

        /**
         * \brief Combine the two wires into a the ports of a new unit.
         */
        void _createJunction(CircuitWire* toWire, CircuitWire* fromWire, const Eigen::Vector2i& pos, const std::string& a_unitPrototype);
        
    private:
        synui::MainWindow* m_window;
        synui::UnitEditorHost* m_unitEditorHost;
        syn::UnitFactory* m_uf;
        syn::VoiceManager* m_vm;
        std::unordered_map<unsigned, UnitWidgetConstructor > m_registeredUnitWidgets;
        synui::Grid2D<GridCell> m_grid;
        int m_gridSpacing;
        WireDrawStyle m_wireDrawStyle;

        enum class State
        {
            Uninitialized,
            Idle,
            CreatingUnit,
            MovingUnit,
            DrawingWire,
            DrawingSelection
        } m_state;

        struct CreatingUnitState
        {
            int unitId;
            bool isValid;
            synui::UnitWidget* widget;
        } m_creatingUnitState;
        std::unordered_map<int, UnitWidget*> m_unitWidgets;

        struct MovingUnitState
        {
            std::unordered_map<int, Eigen::Vector2i> originalPositions;
            Eigen::Vector2i start;
        } m_movingUnitState;

        struct DrawingWireState
        {
            bool startedFromOutput;
            CircuitWire* wire;
        } m_drawingWireState;
        double m_wireHighlightTime;
        CircuitWire* m_highlightedWire;
        std::vector<CircuitWire*> m_wires;

        struct DrawingSelectionState
        {
            Eigen::Vector2i startPos;
            Eigen::Vector2i endPos;
        } m_drawingSelectionState;
        std::unordered_set<UnitWidget*> m_selection;
    };
}
