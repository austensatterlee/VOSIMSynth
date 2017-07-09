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
#include <vosimlib/common_serial.h>
#include <vosimlib/common.h>
#include <eigen/src/Core/util/ForwardDeclarations.h>

namespace syn {
    class VoiceManager;
    class UnitFactory;
    class Unit;
}

namespace synui {
    class MainWindow;
    class UnitWidget;
    class UnitEditorHost;
    class UnitEditor;
}

namespace synui {
    class CircuitWire;
    class CircuitWidget;

    namespace cwstate {
        class State;
    }

    typedef std::function<UnitWidget*(CircuitWidget*, syn::VoiceManager*, int)> UnitWidgetConstructor;

    class CircuitWidget : public nanogui::Widget {
        friend class UnitWidget;
        friend class CircuitWire;
        friend class cwstate::State;
    public:
        typedef std::pair<int, int> Port;

        struct GridCell {
            struct State {
                enum StateType {
                    Wire,
                    Unit
                } type;
                void* ptr;

                friend bool operator==(const State& a_lhs, const State& a_rhs) { return a_lhs.type==a_rhs.type && a_lhs.ptr==a_rhs.ptr; }
                friend bool operator!=(const State& a_lhs, const State& a_rhs) { return !(a_lhs == a_rhs); }
            };

            friend bool operator==(const GridCell& a_lhs, const GridCell& a_rhs) {
                if(a_lhs.states.size()!=a_rhs.states.size())
                    return false;
                for(int i=0; i<a_lhs.states.size(); i++) {
                    auto& lhs_state = a_lhs.states[i];
                    auto& rhs_state = a_rhs.states[i];
                    if(lhs_state!=rhs_state)
                        return false;
                }
                return true;
            }

            friend bool operator!=(const GridCell& a_lhs, const GridCell& a_rhs) { return !(a_lhs == a_rhs); }

            operator bool() const {
                return !states.empty();
            }

            bool contains (State::StateType a_stateType) const {
                return std::any_of(states.begin(), states.end(), [&](auto state){ return state.type==a_stateType; });
            }

            bool contains (void* a_ptr) const {
                return std::any_of(states.begin(), states.end(), [&](auto state){ return state.ptr==a_ptr; });
            }

            bool contains (const State& a_state) const {
                return std::any_of(states.begin(), states.end(), [&](auto state){ return state==a_state; });
            }

            auto find(State::StateType a_stateType) {
                return std::find_if(states.begin(), states.end(), [&](auto state){ return state.type==a_stateType; });
            }

            auto find (void* a_ptr) {
                return std::find_if(states.begin(), states.end(), [&](auto state){ return state.ptr==a_ptr; });
            }

            auto find (const State& a_state) {
                return std::find_if(states.begin(), states.end(), [&](auto state){ return state==a_state; });
            }

            bool remove (const State& a_state) {
                auto old_size = states.size();
                states.erase(std::remove(states.begin(), states.end(), a_state), states.end());
                return old_size!=states.size();           
            }

            std::vector<State> states;
        };

        enum WireDrawStyle {
            Straight = 0,
            Curved
        };

        /**
         * \brief Displays and manipulates an audio circuit.
         * \param a_parent Parent widget.
         * \param a_mainWindow Serves as an interface for queueing messages to the real-time thread.
         * \param a_unitEditorHost The widget that will host unit editor interfaces.
         * \param a_vm Voice manager from the real-time thread.
         */
        CircuitWidget(Widget* a_parent, MainWindow* a_mainWindow, UnitEditorHost* a_unitEditorHost, syn::VoiceManager* a_vm);

        bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;
        bool mouseMotionEvent(const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) override;

        void draw(NVGcontext* ctx) override;

        /**
         * \brief Construct a new syn::Unit and its corresponding synui::UnitWidget, and prepare to place the
         * synui::UnitWidget somewhere on the grid. The widget will be placed on the grid with the next left
         * click on a valid location.
         * \param a_unitPrototype The prototype name of the unit to create. Corresponds to a name registered with the syn::UnitFactory.
         */
        void loadPrototype(syn::UnitTypeId a_unitPrototype);

        /**
         * \brief Snaps the pixel coordinate to one that lies on the grid.
         */
        Eigen::Vector2i fixToGrid(const Eigen::Vector2i& a_pixelLocation) const;

        int gridSpacing() const { return m_gridSpacing; }

        Grid2D<GridCell>& grid() { return m_grid; }
        std::vector<std::shared_ptr<CircuitWire>>& wires() { return m_wires; }
        syn::VoiceManager& vm() const { return *m_vm; }
        UnitEditorHost& unitEditorHost() const { return *m_unitEditorHost; }
        std::unordered_set<UnitWidget*>& unitSelection() { return m_unitSelection; }
        std::unordered_map<int, UnitWidget*>& unitWidgets() { return m_unitWidgets; }

        WireDrawStyle wireDrawStyle() const { return m_wireDrawStyle; }
        void setWireDrawStyle(WireDrawStyle a_newStyle) { m_wireDrawStyle = a_newStyle; }

        template <typename UnitType>
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


        /**
         * \brief Creates a unit widget of the specified classId and associate it with the given unitId.
         */
        UnitWidget* createUnitWidget(syn::UnitTypeId a_classId, int a_unitId);

        /**
         * \brief Delete the given unit widget.
         * Note that this method only affects the GUI. The actual connection on the real-time thread is not affected.
         */
        void deleteUnitWidget(UnitWidget* widget);

        /**
         * \brief Create a wire widget.
         * Note that this method only affects the GUI. The actual connection on the real-time thread is not affected.
         */
        void createWireWidget(const Port& a_inputPort, const Port& a_outputPort);

        /**
         * \brief Delete the given wire widget.
         * Note that this method only affects the GUI. The actual connection on the real-time thread is not affected.
         */
        void deleteWireWidget(std::shared_ptr<CircuitWire> wire);

        /**
         * \brief Sends a message to the real-time thread to create a new connection between two units.
         */
        void createConnection(const Port& a_inputPort, const Port& a_outputPort);

        void deleteConnection(const Port& a_inputPort, const Port& a_outputPort);

        /**
         * \brief Combine the two wires into a new unit.
         */
        void createJunction(std::shared_ptr<CircuitWire> a_toWire, std::shared_ptr<CircuitWire> a_fromWire, const Eigen::Vector2i& a_pos, syn::UnitTypeId a_classId);

        /**
         * \brief Sends a request to the real-time thread to create a new unit.
         * \param a_classId Class id of the type of the unit to create.
         */
        void createUnit(syn::UnitTypeId a_classId);

        void deleteUnit(int a_unitId);

    protected:

        /**
         * \brief Create widgets for input/output units.
         */
        void createInputOutputUnits_();

    private:

        void _changeState(cwstate::State* a_state);

    private:
        MainWindow* m_window;
        UnitEditorHost* m_unitEditorHost;
        syn::VoiceManager* m_vm;
        std::unordered_map<syn::UnitTypeId, UnitWidgetConstructor> m_registeredUnitWidgets;
        Grid2D<GridCell> m_grid;
        int m_gridSpacing;
        WireDrawStyle m_wireDrawStyle;
        std::unordered_set<UnitWidget*> m_unitSelection;
        std::unique_ptr<cwstate::State> m_state;
        bool m_uninitialized;

        std::unordered_map<int, UnitWidget*> m_unitWidgets;

        struct DrawingWireState { } m_drawingWireState;

        std::vector<std::shared_ptr<CircuitWire>> m_wires;
    };

    namespace cwstate {
        class State {
        public:
            virtual ~State() {}
            virtual bool mouseButtonEvent(CircuitWidget& cw, const Eigen::Vector2i& p, int button, bool down, int modifiers) = 0;
            virtual bool mouseMotionEvent(CircuitWidget& cw, const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) = 0;
            virtual void draw(CircuitWidget& cw, NVGcontext* ctx) = 0;
            virtual void enter(CircuitWidget& cw, State& oldState) = 0;
            virtual void exit(CircuitWidget& cw, State& newState) = 0;
        protected:
            void changeState(CircuitWidget& cw, State& state) const {
                cw._changeState(&state);
            }
        };

        class IdleState : public State {
            std::shared_ptr<CircuitWire> m_highlightedWire;
            double m_wireHighlightTime;
            UnitWidget* m_clickedWidget;
        public:
            IdleState()
                : m_highlightedWire(nullptr),
                  m_wireHighlightTime(0),
                  m_clickedWidget(nullptr) {}

            bool mouseButtonEvent(CircuitWidget& cw, const Eigen::Vector2i& p, int button, bool down, int modifiers) override;
            bool mouseMotionEvent(CircuitWidget& cw, const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) override;

            void draw(CircuitWidget& cw, NVGcontext* ctx) override;
            void enter(CircuitWidget& cw, State& oldState) override {}
            void exit(CircuitWidget& cw, State& newState) override {}
        };

        /**
         * `MovingUnit` state.
         * 
         *  During this state, all unit widgets in the selection set are moved.
         */
        class MovingUnitState : public State {
            std::unordered_map<int, Eigen::Vector2i> m_origPositions;
            Eigen::Vector2i m_start; //<! The location of the mouse; used as a reference point for determining movement amounts.
        public:
            bool mouseButtonEvent(CircuitWidget& cw, const Eigen::Vector2i& p, int button, bool down, int modifiers) override;
            bool mouseMotionEvent(CircuitWidget& cw, const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) override;
            void draw(CircuitWidget& cw, NVGcontext* ctx) override;
            void enter(CircuitWidget& cw, State& oldState) override;
            void exit(CircuitWidget& cw, State& newState) override {};
        };

        class DrawingSelectionState : public State {
            Eigen::Vector2i m_startPos;
            Eigen::Vector2i m_endPos;
        public:
            bool mouseButtonEvent(CircuitWidget& cw, const Eigen::Vector2i& p, int button, bool down, int modifiers) override;
            bool mouseMotionEvent(CircuitWidget& cw, const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) override;
            void draw(CircuitWidget& cw, NVGcontext* ctx) override;
            void enter(CircuitWidget& cw, State& oldState) override;
            void exit(CircuitWidget& cw, State& newState) override {};
        };

        class DrawingWireState : public State {
            bool m_startedFromOutput;
            bool m_endedOnOutput;
            CircuitWidget::Port m_startPort;
            CircuitWidget::Port m_endPort;
            std::shared_ptr<CircuitWire> m_wire;
        public:
            DrawingWireState(const CircuitWidget::Port& a_port, bool a_isOutput)
                : m_startedFromOutput(a_isOutput),
                  m_endedOnOutput(a_isOutput),
                  m_startPort(a_port),
                  m_endPort({-1,-1}),
                  m_wire(nullptr) {}

            bool mouseButtonEvent(CircuitWidget& cw, const Eigen::Vector2i& p, int button, bool down, int modifiers) override;
            bool mouseMotionEvent(CircuitWidget& cw, const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) override { return false; }
            void draw(CircuitWidget& cw, NVGcontext* ctx) override;
            void enter(CircuitWidget& cw, State& oldState) override;
            void exit(CircuitWidget& cw, State& newState) override;
        };

        class CreatingUnitState : public State {
            typedef std::function<void(void)> ExitFunc;
            int m_unitId;
            bool m_isValid;
            UnitWidget* m_widget;
            ExitFunc m_onSuccess;
            ExitFunc m_onFailure;
        public:
            CreatingUnitState(int a_unitId, ExitFunc a_onSuccess = [] {}, ExitFunc a_onFailure = [] {})
                : m_unitId(a_unitId),
                  m_isValid(true),
                  m_widget(nullptr),
                  m_onSuccess(a_onSuccess),
                  m_onFailure(a_onFailure) {
                m_unitId = a_unitId;
            }

            bool mouseButtonEvent(CircuitWidget& cw, const Eigen::Vector2i& p, int button, bool down, int modifiers) override;
            bool mouseMotionEvent(CircuitWidget& cw, const Eigen::Vector2i& p, const Eigen::Vector2i& rel, int button, int modifiers) override { return false; };
            void draw(CircuitWidget& cw, NVGcontext* ctx) override;
            void enter(CircuitWidget& cw, State& oldState) override;
            void exit(CircuitWidget& cw, State& newState) override;
        };
    }
}
