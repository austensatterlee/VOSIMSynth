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
 *  \file CircuitWire.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 06/2017
 */

#pragma once
#include "vosimsynth/CircuitWidget.h"
#include <set>

namespace synui {
    class CircuitWire : public std::enable_shared_from_this<CircuitWire> {
    public:

        explicit CircuitWire(CircuitWidget* a_parentCircuit)
            :
            highlight(None),
            m_parentCircuit(a_parentCircuit),
            m_inputPort({-1,-1}),
            m_outputPort({-1,-1}),
            m_start({-1,-1}),
            m_end({-1,-1}),
            m_cost(0),
            m_isFinal(false) { }

        ~CircuitWire() {
            m_parentCircuit->m_grid.map(
                [&](CircuitWidget::GridCell& cell){return cell.remove({CircuitWidget::GridCell::State::Wire, this}); }
            );
        }

        std::string info() const;

        void draw(NVGcontext* ctx);

        /**
         * \brief Determine and update the start and end points of the wire.
         * If both the input and output port have been set, then the start and end points are simply the
         * location of those ports. If only one of them has been set, the location of that port is the start
         * point, and the `a_endPt` argument is used to set the end point.
         * \param a_endPt End point to use in the case that one of the ports has not been set.
         */
        void updateStartAndEndPositions(const Eigen::Vector2i& a_endPt = {-1,-1});

        /**
         * \brief Weighting function used for pathfinding.
         *
         * Weights straight paths higher than jagged ones, and prefers crossing wires instead over going through units.
         */
        template <typename CellType>
        struct weight_func {
            int operator()(const Grid2D<CellType>& grid, const Grid2DPoint& prev, const Grid2DPoint& curr, const Grid2DPoint& next) const;
        };

        /**
         * \brief Refresh the wire's path.
         */
        void updatePath();

        float pathCost() const;

        void setInputPort(const CircuitWidget::Port& a_port);

        void setOutputPort(const CircuitWidget::Port& a_port);

        CircuitWidget::Port getInputPort() const;
        CircuitWidget::Port getOutputPort() const;

        operator json() const;

        void load(const json& j);

        enum HighlightStyle {
            None = 0,
            Incoming,
            Outgoing,
            Selected
        } highlight;

    private:
        CircuitWidget* m_parentCircuit;
        CircuitWidget::Port m_inputPort;
        CircuitWidget::Port m_outputPort;
        Eigen::Vector2i m_start;
        Eigen::Vector2i m_end;
        std::vector<Grid2DPoint> m_path;
        std::set<std::pair<int, int>> m_crossings;
        float m_cost;
        bool m_isFinal;
    };
}