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
#include "../../libs/eigen/eigen/src/Core/util/ForwardDeclarations.h"

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
        friend class CircuitWire;
        typedef std::pair<int, int> Port;
    public:
        struct GridCell
        {
            friend bool operator==(const GridCell& a_lhs, const GridCell& a_rhs)
            {
                return a_lhs.state==a_rhs.state && (a_lhs.state==Empty || a_lhs.ptr==a_rhs.ptr);
            }

            friend bool operator!=(const GridCell& a_lhs, const GridCell& a_rhs) { return !(a_lhs == a_rhs); }

            operator bool() const
            {
                return state!=Empty;
            }

            enum
            {
                Empty,
                Wire,
                Unit
            } state;
            void* ptr;
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

        syn::VoiceManager* voiceManager() const { return m_vm; }

        int gridSpacing() const { return m_gridSpacing; }

        template<typename UnitType>
        void registerUnitWidget(UnitWidgetConstructor a_func) { m_registeredUnitWidgets[UnitType("").getClassIdentifier()] = a_func; }

        void performLayout(NVGcontext* ctx) override;
        void resizeGrid(int a_newGridSpacing);

        operator json() const;
        CircuitWidget* load(const json& j);
        void reset();
    protected:
        /**
         * \brief Schedule a call to performLayout for the next call to draw().
         */
        void performScreenLayout_()
        {
            Widget* screen = this;
            while(screen->parent()) screen = screen->parent();
            static_cast<nanogui::Screen *>(screen)->performLayout();
        }

        /**
         * \brief Create widgets for input/output units.
         */
        void createInputOutputUnits_();

        /**
         * \brief Sends a request to the real-time thread to create a new unit.
         * \param a_unitPrototypeName Prototype name of the unit to create. Corresponds to a prototype name in
         * the unit factory.
         */
        void createUnit_(const std::string& a_unitPrototypeName);

        /**
         * \brief Called upon successful creation of a unit on the real-time thread. Creates the associated
         * unit widget and places the CircuitWidget in the `PlacingUnit` state.
         * \param a_unitId Id of the newly created unit.
         */
        void onUnitCreated_(unsigned a_classId, int a_unitId);
        
        /**
         * \brief Creates a unit widget of the specified classId and associate it with the given unitId.
         */
        UnitWidget* createUnitWidget_(unsigned a_classId, int a_unitId);

        /**
         * \brief Try to end the `PlacingUnit` state by setting the position of the widget being placed.
         * \returns true if unit was placed, false if the position was not valid.
         */
        bool endPlaceUnit_(const Eigen::Vector2i& a_pos);

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
         * \brief Attempt to update the position of an existing unit and record the change if successful.
         * This method should be used to move units instead of interacting with the unit widget directly.
         * This method sets the position of the unit widget and updates the internal
         * grid to reflect the change in occupied and unoccupied cells.
         * \param a_force Perform the update even if the unit already occupies that position.
         * \return true if the move succeeded and false otherwise.
         */
        void updateUnitPos_(UnitWidget* a_unitWidget, const Eigen::Vector2i& a_newPos, bool a_force = false);

        /**
         * \brief Checks if a unit would intersect another unit if placed at the specified position.
         */
        bool checkUnitPos_(UnitWidget* a_unitWidget, const Eigen::Vector2i& a_newPos);


    private:
        /**
         * \brief Delete the given wire widget.
         * Note that this method only affects the GUI. The actual connection on the real-time thread is not affected.
         */
        void deleteWireWidget_(CircuitWire* wire);
        /**
         * \brief Delete the given unit widget.
         * Note that this method only affects the GUI. The actual connection on the real-time thread is not affected.         
         */
        void deleteUnitWidget_(UnitWidget* widget);
    private:
        synui::MainWindow* m_window;
        synui::UnitEditorHost* m_unitEditorHost;
        syn::UnitFactory* m_uf;
        syn::VoiceManager* m_vm;
        std::unordered_map<unsigned, UnitWidgetConstructor > m_registeredUnitWidgets;
        synui::Grid2D<GridCell> m_grid;
        int m_gridSpacing;

        enum class State
        {
            Uninitialized,
            Idle,
            PlacingUnit,
            DrawingWire
        } m_state;

        struct PlacingUnitState
        {
            int unitId;
            bool isValid;
            synui::UnitWidget* widget;
        } m_placingUnitState;

        std::unordered_map<int, UnitWidget*> m_unitWidgets;

        struct DrawingWireState
        {
            bool startedFromOutput;
            CircuitWire* wire;
        } m_drawingWireState;        
        double m_wireHighlightTime;
        CircuitWire* m_highlightedWire;
        std::vector<CircuitWire*> m_wires;
    };
}
