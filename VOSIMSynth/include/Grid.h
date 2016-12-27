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
 *  \file Grid.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once

#include <eigen/Core>
#include <vector>
#include <array>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <boost/heap/binomial_heap.hpp>
#include <DSPMath.h>

namespace synui
{
    typedef Eigen::Vector2i Grid2DPoint; /// Coordinate of a point on the grid (row, col).
    typedef int Grid2DIndex; /// Flat index.
    typedef std::pair<Grid2DIndex, Grid2DIndex> Grid2DEdge;

    template <typename T>
    class Grid2D;

    template <typename CellType>
    struct manhattan_distance
    {
        int operator()(const Grid2D<CellType>& grid, const Grid2DPoint& a, const Grid2DPoint& b) const { return std::abs(a[0] - b[0]) + std::abs(a[1] - b[1]); }
    };

    template <typename CellType>
    struct default_weight_func
    {
        int operator()(const Grid2D<CellType>& grid, const Grid2DPoint& prev, const Grid2DPoint& curr, const Grid2DPoint& next)
        {
            if (grid.get(next) == grid.getEmptyValue())
                return 1;
            else
                return 10;
        }
    };

    /**
     * \brief A 2-dimensional grid, on which path-finding and other operations may be performed.
     * \tparam T Type of object to be stored at each grid cell.
     */
    template <typename CellType>
    class Grid2D
    {
    public:
        typedef Eigen::Matrix<CellType, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixXt;

        /**
         * \brief Construct a grid with the given shape, with all cells initialized with the given default value.
         * \param a_size Shape of the grid.
         * \param a_unoccupiedValue Value used to initialize new cells and mark unoccupied cells.
         */
        Grid2D<CellType>(const Eigen::Vector2i& a_size, const CellType& a_unoccupiedValue) :
            m_shape{ a_size },
            m_grid{ a_size[0], a_size[1] },
            m_unoccupiedValue{ a_unoccupiedValue } {
            m_grid.fill(a_unoccupiedValue);
        }

        const CellType& getEmptyValue() const { return m_unoccupiedValue; }

        Eigen::Vector2i getShape() const { return m_shape; }
        int getSize() const { return m_shape[0] * m_shape[1]; }

        CellType& get(const Grid2DPoint& a_pt) { return m_grid(a_pt[0], a_pt[1]); }

        const CellType& get(const Grid2DPoint& a_pt) const { return m_grid(a_pt[0], a_pt[1]); }

        CellType& get(const Grid2DIndex& a_ind) { return m_grid(a_ind); }

        const CellType& get(const Grid2DIndex& a_ind) const { return m_grid(a_ind); }

        MatrixXt& get() { return m_grid; }

        /**
         * \brief Save the current state of the grid so that it can later be restored. Any previous backup is
         * destroyed.
         */
        void save() { m_gridBackup = m_grid; }

        /**
         * \brief Restore the grid to its state at the time of the most recent call to save(). Should not be
         * called unless save() has been called at least once.
         */
        void restore() { m_grid = m_gridBackup; }

        void reset()
        {
            setBlock({ 0, 0 }, m_shape, getEmptyValue());
        }

        /**
         * \brief Assign a value to a block of cells
         * \param a_topLeft Top left point of the block
         * \param a_bottomRight Bottom right point of the block
         * \param a_value New value
         * \param requireUnoccupied Fail the operation if some of the cells are occupied.
         * \returns False if no assignments occured.
         */
        bool setBlock(const Grid2DPoint& a_topLeft, const Grid2DPoint& a_bottomRight, const CellType& a_value, bool requireUnoccupied = true)
        {
            int t = std::min(a_topLeft[0], a_bottomRight[0]);
            int b = std::max(a_topLeft[0], a_bottomRight[0]);
            int l = std::min(a_topLeft[1], a_bottomRight[1]);
            int r = std::max(a_topLeft[1], a_bottomRight[1]);

            // Check that bounds lie inside grid
            if (t >= 0 && t <= m_shape[0] && l >= 0 && l <= m_shape[1] && b >= 0 && b <= m_shape[0] && r >= 0 && r <= m_shape[1])
            {
                auto blk = m_grid.block(t, l, b - t, r - l);
                if (!requireUnoccupied || (blk.array() == m_unoccupiedValue).all())
                {
                    blk.fill(a_value);
                    return true;
                }
            }

            return false;
        }

        /**
           * \brief Like setBlock(), but forces the operation to occur by changing a_topLeft and a_topRight to fit within the grid boundaries.
           * \param requireUnoccupied Fail the operation if some of the cells are occupied.
           * \return False if no assignments occured.
           */
        bool forceSetBlock(Grid2DPoint& a_topLeft, Grid2DPoint& a_bottomRight, const CellType& a_value, bool requireUnoccupied = true)
        {
            int t = std::min(a_topLeft[0], a_bottomRight[0]);
            int b = std::max(a_topLeft[0], a_bottomRight[0]);
            int l = std::min(a_topLeft[1], a_bottomRight[1]);
            int r = std::max(a_topLeft[1], a_bottomRight[1]);

            // Fix bounds to be within grid
            t = syn::CLAMP(t, 0, m_shape[0]);
            b = syn::CLAMP(b, 0, m_shape[0]);
            l = syn::CLAMP(l, 0, m_shape[1]);
            r = syn::CLAMP(r, 0, m_shape[1]);
            auto blk = m_grid.block(t, l, b - t, r - l);
            if (!requireUnoccupied || (blk.array() == m_unoccupiedValue).all())
            {
                blk.fill(a_value);
                a_topLeft = { t,l };
                a_bottomRight = { b,r };
                return true;
            }
            return false;
        }

        /**
         * \brief Retrieve a block of the grid.
         * \param a_topLeft Top left point of the block
         * \param a_bottomRight Bottom right point of the block
         */
        Eigen::Block<MatrixXt, -1, -1> getBlock(const Grid2DPoint& a_topLeft, const Grid2DPoint& a_bottomRight)
        {
            int t = std::min(a_topLeft[0], a_bottomRight[0]);
            int b = std::max(a_topLeft[0], a_bottomRight[0]);
            int l = std::min(a_topLeft[1], a_bottomRight[1]);
            int r = std::max(a_topLeft[1], a_bottomRight[1]);
            return m_grid.block(t, l, b - t, r - l);
        }

        /**
         * \brief Like getBlock(), but forces the operation to occur by changing a_topLeft and a_topRight to fit within the grid boundaries.
         */
        Eigen::Block<MatrixXt, -1, -1> forceGetBlock(Grid2DPoint& a_topLeft, Grid2DPoint& a_bottomRight)
        {
            int t = std::min(a_topLeft[0], a_bottomRight[0]);
            int b = std::max(a_topLeft[0], a_bottomRight[0]);
            int l = std::min(a_topLeft[1], a_bottomRight[1]);
            int r = std::max(a_topLeft[1], a_bottomRight[1]);
            // Fix bounds to be within grid
            t = syn::CLAMP(t, 0, m_shape[0]);
            b = syn::CLAMP(b, 0, m_shape[0]);
            l = syn::CLAMP(l, 0, m_shape[1]);
            r = syn::CLAMP(r, 0, m_shape[1]);
            a_topLeft = { t,l };
            a_bottomRight = { b,r };
            return m_grid.block(t, l, b - t, r - l);
        }

        /**
         * \brief Replace all occurences of the given value with a new value.
         * \param a_value Old value.
         * \param a_newValue New value.
         */
        void replaceValue(const CellType& a_value, const CellType& a_newValue) { m_grid = (m_grid.array() == a_value).select(a_newValue, m_grid); }

        bool isOccupied(const Grid2DPoint& a_pt) const { return m_grid(a_pt[0], a_pt[1]) != m_unoccupiedValue; }
        bool isOccupied(Grid2DIndex a_ind) const { return m_grid(a_ind) != m_unoccupiedValue; }

        /**
         * \brief Compute whether or not the given point lies within the bounds of the grid.
         */
        bool contains(const Grid2DPoint& a_pt) const { return a_pt[0] >= 0 && a_pt[1] >= 0 && a_pt[0] < m_shape[0] && a_pt[1] < m_shape[1]; }

        /**
         * \brief Compute whether or not all points within the given block lie within the bounds of the grid.
         * \param a_topLeft Top left corner of the block.
         * \param a_bottomRight Bottom right corner of the block.
         */
        bool contains(const Grid2DPoint& a_topLeft, const Grid2DPoint& a_bottomRight)
        {
            int t = std::min(a_topLeft[0], a_bottomRight[0]);
            int b = std::max(a_topLeft[0], a_bottomRight[0]);
            int l = std::min(a_topLeft[1], a_bottomRight[1]);
            int r = std::max(a_topLeft[1], a_bottomRight[1]);
            return t >= 0 && t <= m_shape[0] && l >= 0 && l <= m_shape[1] && b >= 0 && b <= m_shape[0] && r >= 0 && r <= m_shape[1];
        }

        /**
         * \brief Convert a multi-dimensional index into a flat index.
         */
        Grid2DIndex ravel_index(const Grid2DPoint& a_pt) const { return a_pt[0] * m_shape[1] + a_pt[1]; }

        /**
         * \brief Convert a flat index into a multi-dimensional index.
         */
        Grid2DPoint unravel_index(const Grid2DIndex& a_index) const
        {
            int row = a_index / m_shape[1];
            int col = a_index % m_shape[1];
            return{ row,col };
        }

        /**
         * \brief Converts a pixel coordinate (x,y) into a grid coordinate (row,col).
         */
        Grid2DPoint fromPixel(const Eigen::Vector2i& a_pixel, int pixelsPerCell) const
        {
            Eigen::Vector2f point{ std::round(a_pixel[1] * 1.0f / pixelsPerCell),std::round(a_pixel[0] * 1.0f / pixelsPerCell) };
            return point.cast<int>();
        }

        /**
         * \brief Converts a grid coordinate (row, col) into a pixel coordinate (x,y).
         */
        Eigen::Vector2i toPixel(const Grid2DPoint& a_pt, int pixelsPerCell) const { return a_pt.reverse() * pixelsPerCell; }

        /**
         * \brief Return points that are adjacent to the one provided.
         * \param a_omitOccupied If set to true, only neighbors which are unoccupied will be returned.
         */
        std::vector<Grid2DPoint> getNeighbors(const Grid2DPoint& a_pt, bool a_omitOccupied = false) const
        {
            std::vector<Grid2DPoint> result;
            std::array<Grid2DPoint, 4> directions;
            directions[0] = Eigen::Vector2i::Unit(0);
            directions[2] = -Eigen::Vector2i::Unit(0);
            directions[1] = Eigen::Vector2i::Unit(1);
            directions[3] = -Eigen::Vector2i::Unit(1);
            for (const Grid2DPoint& dir : directions)
            {
                Grid2DPoint neighbor = a_pt + dir;
                if (contains(neighbor) && !(a_omitOccupied && isOccupied(neighbor)))
                    result.push_back(neighbor);
            }
            return result;
        }

        std::vector<Grid2DIndex> getNeighbors(const Grid2DIndex& a_index, bool a_omitOccupied = false) const
        {
            std::vector<Grid2DPoint> pointNeighbors = getNeighbors(unravel_index(a_index), a_omitOccupied);
            std::vector<Grid2DIndex> vertexNeighbors(pointNeighbors.size());
            std::transform(pointNeighbors.begin(), pointNeighbors.end(), vertexNeighbors.begin(), [this](const Grid2DPoint& pt) { return ravel_index(pt); });
            return vertexNeighbors;
        }

        /**
         * \brief Return a list of all the edges in the graph.
         * \param a_omitOccupied If set to true, edges involving occupied point will be omitted.
         */
        std::vector<Grid2DEdge> getEdges(bool a_omitOccupied = false) const
        {
            std::vector<Grid2DEdge> edges;
            for (int i = 0; i < getSize(); i++)
            {
                std::vector<Grid2DIndex> neighbors = getNeighbors(i, a_omitOccupied);
                for (auto neighbor : neighbors) { edges.push_back(Grid2DEdge{ i, neighbor }); }
            }
            return edges;
        }

        /**
         * \brief Change the size/shape of the grid.
         *
         * If new cells are added, they will be initialized with the grid's default value.
         */
        void resize(const Eigen::Vector2i& a_newSize)
        {
            int commonRows = std::min(m_shape[0], a_newSize[0]);
            int commonCols = std::min(m_shape[1], a_newSize[1]);
            MatrixXt oldMatrix = m_grid.block(0, 0, commonRows, commonCols);
            m_grid.resize(a_newSize[0], a_newSize[1]);
            m_grid.fill(m_unoccupiedValue);
            m_grid.block(0, 0, commonRows, commonCols) = oldMatrix;
            m_shape = a_newSize;
        }

        template <template <typename> typename Heuristic = manhattan_distance, template <typename> typename WeightFunc = default_weight_func>
        std::list<Grid2DPoint> findPath(const Grid2DPoint& a_start, const Grid2DPoint& a_end) const
        {
            using Vertex = Grid2DIndex;
            struct ScoredVertex
            {
                Vertex ind;
                int fscore;
            };

            struct ScoredVertexComparator
            {
                bool operator()(const ScoredVertex& a, const ScoredVertex& b) const { return a.fscore >= b.fscore; }
            };
            Heuristic<CellType> h;
            WeightFunc<CellType> w;
            Vertex start = ravel_index(a_start);
            Vertex end = ravel_index(a_end);

            std::unordered_map<Vertex, Vertex> cameFrom;
            std::unordered_map<Vertex, int> gScore;
            gScore[start] = 0;
            std::unordered_set<Vertex> closedSet;

            std::unordered_map<Vertex, typename boost::heap::binomial_heap<ScoredVertex, boost::heap::compare<ScoredVertexComparator>>::handle_type> openSet;
            boost::heap::binomial_heap<ScoredVertex, boost::heap::compare<ScoredVertexComparator>> open;
            openSet[start] = open.push({ start, h(*this, a_start, a_end) });

            std::list<Grid2DPoint> path;

            auto reconstruct_path = [this, &path, &cameFrom, &end]()
            {
                Vertex current = end;
                path.insert(path.begin(), unravel_index(current));
                while (cameFrom.find(current) != cameFrom.end())
                {
                    current = cameFrom[current];
                    path.insert(path.begin(), unravel_index(current));
                }
            };

            while (open.size())
            {
                auto scored_current = open.top();
                Vertex current = scored_current.ind;
                open.pop();
                openSet.erase(current);
                closedSet.insert(current);
                if (current == end)
                {
                    reconstruct_path();
                    return path;
                }

                auto neighbors = getNeighbors(current, false);
                for (auto neighbor : neighbors)
                {
                    if (closedSet.find(neighbor) != closedSet.end())
                        continue;
                    const Grid2DPoint& prev = cameFrom.find(current) != cameFrom.end() ? unravel_index(cameFrom[current]) : Grid2DPoint{ -1,-1 };
                    const Grid2DPoint& curr = unravel_index(current);
                    const Grid2DPoint& next = unravel_index(neighbor);
                    int tentative_gScore = gScore[current] + w(*this, prev, curr, next);

                    if (openSet.find(neighbor) == openSet.end()) { openSet[neighbor] = open.push({ neighbor,std::numeric_limits<int>::infinity() }); }
                    else if (gScore.find(neighbor) != gScore.end() && tentative_gScore >= gScore[neighbor])
                        continue;

                    cameFrom[neighbor] = current;
                    gScore[neighbor] = tentative_gScore;

                    const Grid2DPoint& final = unravel_index(end);
                    int fscore = gScore[neighbor] + h(*this, next, final);
                    open.update(openSet[neighbor], { neighbor, fscore });
                }
            }
            return{};
        }

    private:
        Eigen::Vector2i m_shape; /// shape (rows, cols) of the grid
        MatrixXt m_grid;
        MatrixXt m_gridBackup;
        CellType m_unoccupiedValue;
    };
}
