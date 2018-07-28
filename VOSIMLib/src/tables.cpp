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
#include "vosimlib/tables.h"
#include <math.h>

namespace syn
{
    ResampledTable::ResampledTable(const double* a_data, int a_size, const BlimpTable& a_blimp_table_online, const BlimpTable& a_blimp_table_offline) :
        NormalTable(a_data, a_size),
        m_num_resampled_tables(0),
        m_resampled_sizes(0),
        m_resampled_tables(0),
        m_blimp_table_online(a_blimp_table_online),
        m_blimp_table_offline(a_blimp_table_offline)
    {
        resample_tables();
    }

    void ResampledTable::resample_tables() {
        /* Construct resampled tables at ratios of powers of K */
        m_num_resampled_tables = MIN<int>(6, MAX<int>(1, log2(1.0*m_size) - 2));
        m_resampled_sizes.resize(m_num_resampled_tables);
        m_resampled_tables.resize(m_num_resampled_tables);
        double currsize = m_size;
        for (int i = 0; i < m_num_resampled_tables; i++) {
            m_resampled_sizes[i] = currsize;
            m_resampled_tables[i].resize(currsize);
            resample_table(m_data, m_size, &m_resampled_tables[i][0], m_resampled_sizes[i], m_blimp_table_offline);
            currsize *= 0.5;
        }
    }

    double ResampledTable::getresampled(double phase, double period) const {
        auto ratio = period / m_size;
        int table_index = CLAMP<int>(floor(-log2(ratio)), 0, m_num_resampled_tables-1);
        return getresampled_single(&m_resampled_tables[table_index][0], m_resampled_sizes[table_index], phase, period, m_blimp_table_online);
    }

    void resample_table(const double* a_table, int a_size, double* a_newTable, double a_newSize, const BlimpTable& a_blimp_table, bool a_preserve_amplitude) {
        double phase = 0;
        double phase_step = 1. / a_newSize;
        double input_max = 0.0, input_min = 0.0;
        double output_max = 0.0, output_min = 0.0;

        for (int i = 0; i < a_newSize; i++) {
            a_newTable[i] = getresampled_single(a_table, a_size, phase, a_newSize, a_blimp_table);
            if (a_preserve_amplitude) {
                output_min = i == 0 ? a_newTable[i] : syn::MIN(output_min, a_newTable[i]);
                output_max = i == 0 ? a_newTable[i] : syn::MAX(output_max, a_newTable[i]);
            }
            phase += phase_step;
        }
        /* normalize */
        if (a_preserve_amplitude) {
            for (int i = 0; i < a_size; i++)
            {
                input_min = i == 0 ? a_table[i] : syn::MIN(input_min, a_table[i]);
                input_max = i == 0 ? a_table[i] : syn::MAX(input_max, a_table[i]);
            }
            double scale = (output_max - output_min) / (input_max - input_min);
            for (int i = 0; i < a_newSize; i++) {
                a_newTable[i] = (a_newTable[i] - input_min) * scale + output_min;
            }
        }
    }

    void fft_resample_table(const double* table, int size, double* resampled_table, double period)
    {
    }

    double getresampled_single(const double* table, int size, double phase, double newSize, const BlimpTable& blimp_table) {
        double ratio = newSize * (1.0 / size);
        phase = WRAP(phase, 1.0)*size;

        double blimp_step;
        if (ratio < 1.0)
            blimp_step = static_cast<double>(blimp_table.res) * ratio;
        else
            blimp_step = static_cast<double>(blimp_table.res);

        int index = static_cast<int>(phase);
        double offset = (phase - index) * blimp_step;
        double output = 0.0;
        double filt_sum = 0.0; // used to make the filter unity gain

        double bkwd_filt_phase = offset;
        double fwd_filt_phase = blimp_step - offset;
        int bkwd_table_index = index;
        int fwd_table_index = index + 1;
        int steps = blimp_table.m_size / blimp_step;
        for (int i = 0; i < steps; i++) {
#if DO_LERP_FOR_SINC
            double bkwd_filt_sample = blimp_table.plerp(bkwd_filt_phase / blimp_table.m_size);
            double fwd_filt_sample = blimp_table.plerp(fwd_filt_phase / blimp_table.m_size);
#else
            double bkwd_filt_sample = blimp_table[static_cast<int>(bkwd_filt_phase)];
            double fwd_filt_sample = blimp_table[static_cast<int>(fwd_filt_phase)];
#endif
            if (bkwd_table_index < 0) {
                bkwd_table_index = size - 1;
            }
            if (fwd_table_index >= size) {
                fwd_table_index = 0;
            }
            output += bkwd_filt_sample * table[bkwd_table_index--];
            output += fwd_filt_sample * table[fwd_table_index++];
            filt_sum += bkwd_filt_sample;
            filt_sum += fwd_filt_sample;
            bkwd_filt_phase += blimp_step;
            fwd_filt_phase += blimp_step;
        }

        return output / filt_sum;
    }
}