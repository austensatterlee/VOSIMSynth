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
 * \file tables.h
 * \brief
 * \details
 * \author Austen Satterlee
 * \date 02/2016
 */

#ifndef __TABLES__
#define __TABLES__

#define DO_LERP_FOR_SINC true

#include "vosimlib/common.h"
#include "DSPMath.h"
#include "lut_tables.h"
#include <vector>

namespace syn {

    template <class T>
    class VOSIMLIB_API LUT {
    protected:
        const double* m_data;
    public:
        const int size;

        LUT(const double* a_data, int a_size)
            : m_data(a_data),
              size(a_size) {}
        
        virtual ~LUT() {}

        double lerp(double phase) const {
            double index = static_cast<const T*>(this)->index(phase);
            index *= size;

            int intPart = static_cast<int>(index);
            double fracPart = index - intPart;
            return m_data[intPart] * (1 - fracPart) + m_data[intPart + 1] * fracPart;
        }

        double plerp(double phase) const {
            double index = static_cast<const T*>(this)->index(phase);
            index = WRAP<double>(index) * size;

            int intPart = index;
            double fracPart = index - intPart;
            if (intPart < size - 1) {
                return m_data[intPart] + (m_data[intPart + 1] - m_data[intPart]) * fracPart;
            }
            if (intPart == size - 1) {
                return m_data[intPart];
            }
            throw "Phase out of bounds.";
        }

        double operator[](int index) const {
            return m_data[index];
        }
    };

    class VOSIMLIB_API AffineTable : public LUT<AffineTable> {
        double m_min, m_max, m_scale;
    public:
        AffineTable(const double* a_data, int a_size, double a_min = 0.0, double a_max = 1.0)
            : LUT<AffineTable>(a_data, a_size),
              m_min(a_min),
              m_max(a_max),
              m_scale(1. / (a_max - a_min)) {}

        double inputMin() const { return m_min; }
        double inputMax() const { return m_max; }

        double index(double phase) const {
            return (phase - m_min) * m_scale;
        }
    };

    class VOSIMLIB_API NormalTable : public LUT<NormalTable> {
    public:
        NormalTable(const double* a_data, int a_size)
            : LUT<NormalTable>(a_data, a_size) {}

        double index(double phase) const {
            return phase;
        }
    };

    class VOSIMLIB_API BlimpTable : public NormalTable {
    public:
        BlimpTable(const double* a_data, int a_size, int a_taps, int a_res)
            : NormalTable(a_data, a_size),
              taps(a_taps),
              res(a_res) {}

        BlimpTable(const BlimpTable& a_other)
            : BlimpTable(a_other.m_data, a_other.size, a_other.taps, a_other.res) {}

        const int taps;
        const int res;
    };

    /**
     * Lookup table that can resample itself with sinc interpolation.
     * Upon construction, the table computes and caches downsampled versions
     * of itself. Each downsampled table is half the size of the previous,
     * so log2(N) tables are created, where N is the size of the initial
     * table.
     */
    class VOSIMLIB_API ResampledTable : public NormalTable {
    public:
        ResampledTable(const double* a_table, int a_size, const BlimpTable& a_blimp_table_online, const BlimpTable& a_blimp_table_offline);

        ResampledTable(const ResampledTable& a_o)
            : ResampledTable(a_o.m_data, a_o.size, a_o.m_blimp_table_online, a_o.m_blimp_table_offline) {}

        void resample_tables();

        /// Retrieve a single sample from the table at the specified phase, as if the table were resampled to have the given period.
        double getresampled(double phase, double period) const;
    protected:
        int m_num_resampled_tables;
        std::vector<int> m_resampled_sizes;
        std::vector<std::vector<double>> m_resampled_tables;
        const BlimpTable& m_blimp_table_online;
        const BlimpTable& m_blimp_table_offline;
    };

    /**
     * Retrieve a single sample from table as if it were resampled to have
     * the specified period, using fractional sinc interpolation/decimation.
     *
     * \param size the size of the input table
     * \param phase the desired phase to sample at, in the range [0,1).
     * \param newSize the desired period to resample at (in fractional number of samples)
     */
    double VOSIMLIB_API getresampled_single(const double* table, int size, double phase, double newSize, const BlimpTable& blimp_table);
    /**
     * Resample an entire table to have the specified period and store the
     * result in resampled_table (which should already be allocated), using
     * fractional sinc interpolation/decimation.
     *
     * \param size the size of the input table
     * \param resampled_table a pointer to the output table
     * \param period the desired period to resample at (in fractional number
     *        of samples). The allocated size of the output table should be
     *        ceil(period).
     */
    void VOSIMLIB_API resample_table(const double* table, int size, double* resampled_table, double period, const BlimpTable& blimp_table, bool normalize = true);

    /**
     * \todo
     */
    void fft_resample_table(const double* table, int size, double* resampled_table, double period);
}
#endif
