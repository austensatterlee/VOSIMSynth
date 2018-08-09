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
        _resample_tables();
    }

    void ResampledTable::_resample_tables() {
        /* Construct resampled tables at ratios of powers of K */
        m_num_resampled_tables = MIN<int>(6, MAX<int>(1, log2(1.0*m_size) - 2));
        m_resampled_sizes.resize(m_num_resampled_tables);
        m_resampled_tables.resize(m_num_resampled_tables);
        m_resampled_sizes[0] = m_size;
        m_resampled_tables[0] = std::vector<double>(m_data, m_data + m_size);
        int currsize = m_size/2;
        for (int i = 1; i < m_num_resampled_tables; i++) {
            m_resampled_sizes[i] = currsize;
            m_resampled_tables[i].resize(currsize);
            resample_table(m_data, m_size, &m_resampled_tables[i][0], m_resampled_sizes[i], m_blimp_table_offline);
            currsize /= 2;
        }
    }

    double ResampledTable::getResampled(double a_phase, double a_period) const {
        auto ratio = a_period / m_size;
        int table_index = CLAMP<int>(floor(-log2(ratio)), 0, m_num_resampled_tables-1);
        return getresampled_single(&m_resampled_tables[table_index][0], m_resampled_sizes[table_index], a_phase, a_period, m_blimp_table_online);
    }

    void resample_table(const double* a_table, int a_size, double* a_new_table, double a_new_period, const BlimpTable& a_blimp_table, bool a_preserve_amplitude) {
        double phase_step = 1. / a_new_period;
        double input_max = 0.0, input_min = 0.0;
        double output_max = 0.0, output_min = 0.0;
        int new_size = ceil(a_new_period);
        for (int i = 0; i < new_size; i++) {
            a_new_table[i] = getresampled_single(a_table, a_size, phase_step*i, a_new_period, a_blimp_table);
            if (a_preserve_amplitude) {
                output_min = i == 0 ? a_new_table[i] : syn::MIN(output_min, a_new_table[i]);
                output_max = i == 0 ? a_new_table[i] : syn::MAX(output_max, a_new_table[i]);
            }
        }
        /* normalize */
        if (a_preserve_amplitude) {
            for (int i = 0; i < a_size; i++)
            {
                input_min = i == 0 ? a_table[i] : syn::MIN(input_min, a_table[i]);
                input_max = i == 0 ? a_table[i] : syn::MAX(input_max, a_table[i]);
            }
            double scale = (output_max - output_min) / (input_max - input_min);
            for (int i = 0; i < new_size; i++) {
                a_new_table[i] = (a_new_table[i] - input_min) * scale + output_min;
            }
        }
    }

    void fft_resample_table(const double* table, int size, double* resampled_table, double period)
    {
    }

    double getresampled_single(const double* table, int size, double phase, double new_period, const BlimpTable& blimp_table) {
        double ratio = new_period / size;
        phase = WRAP(phase, 1.0) * size;

        double blimp_step = blimp_table.res;
        // Lower the cutoff frequency to the new nyquist if we're downsampling
        if (ratio < 1.0)
            blimp_step *= ratio;

        int index = phase;
        double offset = (phase - index) * blimp_step;
        double output = 0.0;
        double filt_sum = 0.0; // used to make the filter unity gain

        double bkwd_filt_phase = offset;
        double fwd_filt_phase = blimp_step - offset;
        int bkwd_table_index = index;
        int fwd_table_index = index == size-1 ? 0 : index + 1;
        int steps = (blimp_table.m_size - fwd_filt_phase) / blimp_step;
        for (int i = 0; i < steps; i++) {
#if DO_LERP_FOR_SINC
            double bkwd_filt_sample = blimp_table.lerp(bkwd_filt_phase);
            double fwd_filt_sample = blimp_table.lerp(fwd_filt_phase);
#else
            double bkwd_filt_sample = blimp_table[static_cast<int>(bkwd_filt_phase)];
            double fwd_filt_sample = blimp_table[static_cast<int>(fwd_filt_phase)];
#endif
            output += bkwd_filt_sample * table[bkwd_table_index] + fwd_filt_sample * table[fwd_table_index];
            filt_sum += bkwd_filt_sample + fwd_filt_sample;

            bkwd_filt_phase += blimp_step;
            fwd_filt_phase += blimp_step;

            bkwd_table_index = bkwd_table_index == 0 ? size - 1 : bkwd_table_index - 1;
            fwd_table_index = fwd_table_index == size - 1 ? 0 : fwd_table_index + 1;
        }
        if (bkwd_filt_phase < blimp_table.m_size - blimp_step) {
            // Backward pass computes one more sample than forward pass
            double bkwd_filt_sample = blimp_table.lerp(bkwd_filt_phase);
            output += bkwd_filt_sample * table[bkwd_table_index];
            filt_sum += bkwd_filt_sample;
        } else if (fwd_filt_phase < blimp_table.m_size - blimp_step) {
            // Backward pass computes one more sample than forward pass
            double fwd_filt_sample = blimp_table.lerp(fwd_filt_phase);
            output += fwd_filt_sample * table[fwd_table_index];
            filt_sum += fwd_filt_sample;
        }

        return output / filt_sum;
    }
}