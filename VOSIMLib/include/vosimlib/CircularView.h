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
#include "DSPMath.h"

namespace syn {
    /**
     * \brief Provides an offset, circular view around a buffer of data.
     * 
     * For example, 
     * \code{.cpp}
     * double mybuf[10];
     * CircularView myview(mybuf, 10, 5, 9);
     * myview[0]; // same as mybuf[9]
     * myview[1]; // same as mybuf[0]
     * myview[2]; // same as mybuf[1]
     * myview[3]; // same as mybuf[2]
     * myview[4]; // same as mybuf[3]
     * myview[5]; // same as mybuf[9]
     * \endcode
     * 
     * \tparam T The type of data stored in the buffer.
     */
    template <typename T>
    class CircularView {
    public:
        CircularView()
            : CircularView(nullptr, 0, 0, 0) { }

        CircularView(const T* a_buf, int a_bufSize, int a_viewSize, int a_viewOffset)
            : m_buf(a_buf),
              m_bufSize(a_bufSize),
              m_viewSize(a_viewSize),
              m_viewOffset(a_viewOffset) { }

        int size() const { return m_viewSize; }

        const T& operator[](int a_index) const {
            int index = WRAP(m_viewOffset + WRAP(a_index, m_viewSize), m_bufSize);
            return m_buf[index];
        }

        std::pair<int, int> argMinMax() const {
            T currMin = (*this)[0], currMax = (*this)[0];
            int minIndex = 0, maxIndex = 0;
            for (int i = 1; i < m_viewSize; i++) {
                T currValue = (*this)[i];
                if (currValue > currMax) {
                    currMax = currValue;
                    maxIndex = i;
                } else if (currValue < currMin) {
                    currMin = currValue;
                    minIndex = i;
                }
            }
            return std::make_pair(minIndex, maxIndex);
        }

    private:
        const T* m_buf;
        int m_bufSize;
        int m_viewSize;
        int m_viewOffset;
    };
}
