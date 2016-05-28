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

#include "tables.h"
#include "DSPMath.h"
#include <cstdlib>
#include <math.h>
#include "../../libs/wdl/fft.h"

namespace syn
{
	LookupTable::LookupTable(const double* a_tableptr, int a_size, double a_input_min, double a_input_max, bool a_isPeriodic) :
		m_size(a_size),
		m_isperiodic(a_isPeriodic),
		m_table(a_tableptr),
		m_input_min(a_input_min),
		m_input_max(a_input_max) {
		m_norm_bias = a_input_min;
		m_norm_scale = 1. / (a_input_max - a_input_min);
		m_normalizePhase = !(a_input_min == 0 && a_input_max == 1);
		/* Construct difference table for linear interpolation */
		m_diff_table = new double[a_size];
		for (int i = 0; i < a_size - 1; i++) {
			m_diff_table[i] = a_tableptr[i + 1] - a_tableptr[i];
		}
		/* If table is periodic, the last difference wraps around */
		if (a_isPeriodic)
			m_diff_table[m_size - 1] = m_table[0] - m_table[m_size - 1];
		else
			m_diff_table[m_size - 1] = 0.0;
	}

	double LookupTable::getlinear(double phase) const {
		if (m_normalizePhase) {
			phase = (phase - m_norm_bias) * m_norm_scale;
		}
		if (m_isperiodic) {
			phase = WRAP(phase, 1.0);
			phase *= m_size;
		}
		else {
			phase = CLAMP(phase, 0.0, 1.0);
			phase *= m_size - 1;
		}

		int int_index = int(phase);
		double frac_index = phase - int_index;
		double val1 = m_table[int_index];
		double diff = m_diff_table[int_index];
		return val1 + diff * frac_index;
	}

	double LookupTable::getraw(int index) const {
		index = CLAMP(index, 0, m_size - 1);
		return m_table[index];
	}

	ResampledLookupTable::ResampledLookupTable(const double* a_table, int a_size, const BlimpTable& a_blimp_table_online, const BlimpTable& a_blimp_table_offline) :
		LookupTable(a_table, a_size, 0, 1, true),
		m_blimp_table_online(a_blimp_table_online),
		m_blimp_table_offline(a_blimp_table_offline),
		m_resampled_sizes(nullptr),
		m_resampled_tables(nullptr),
		m_num_resampled_tables(0) {
		resample_tables(a_blimp_table_offline);
	}

	void ResampledLookupTable::resample_tables(const BlimpTable& blimp_table_offline) {
		/* Construct resampled tables at ratios of powers of two */
		m_num_resampled_tables = MAX(1, log2(m_size) - 3);
		m_resampled_tables = static_cast<double**>(malloc(sizeof(double*) * m_num_resampled_tables));
		m_resampled_sizes = new int[m_num_resampled_tables];
		double currsize = m_size;
		for (int i = 0; i < m_num_resampled_tables; i++) {
			int table_size = static_cast<size_t>(currsize);
			m_resampled_tables[i] = static_cast<double*>(malloc(sizeof(double) * table_size));
			m_resampled_sizes[i] = currsize;
			resample_table(m_table, m_size, m_resampled_tables[i], currsize, blimp_table_offline);
			currsize *= 0.5;
		}
	}

	double ResampledLookupTable::getresampled(double phase, double period) const {
		int min_size_diff = -1;
		int min_size_diff_index = 0;
		for (int i = 0; i < m_num_resampled_tables; i++) {
			int curr_size_diff = m_resampled_sizes[i] - static_cast<int>(period);
			if (curr_size_diff < 0) {
				break;
			}
			if (curr_size_diff < min_size_diff || min_size_diff == -1) {
				min_size_diff = curr_size_diff;
				min_size_diff_index = i;
			}
		}
		return getresampled_single(m_resampled_tables[min_size_diff_index], m_resampled_sizes[min_size_diff_index], phase, period, m_blimp_table_online);
	}

	void resample_table(const double* table, int size, double* resampled_table, double period, const BlimpTable& blimp_table, bool normalize) {
		double phase = 0;
		double phase_step = 1. / period;
		double input_energy = 0.0;
		double output_energy = 0.0;

		for (int i = 0; i < period; i++, phase += phase_step) {
			resampled_table[i] = getresampled_single(table, size, phase, period, blimp_table);
			if (normalize)
				output_energy += resampled_table[i] * resampled_table[i];
		}
		/* normalize */
		if (normalize) {
			for (int i = 0; i < size; i++)
			{
				input_energy += table[i] * table[i];
			}
			double input_power = input_energy / size;
			double output_power = output_energy / period;
			for (int i = 0; i < period; i++) {
				resampled_table[i] = resampled_table[i] * sqrt(output_power) / sqrt(input_power);
			}
		}
	}

	void fft_resample_table(const double* table, int size, double* resampled_table, double period)
	{
	}

	double getresampled_single(const double* table, int size, double phase, double period, const BlimpTable& blimp_table) {
		double ratio = period * (1.0 / size);
		phase = WRAP(phase, 1.0)*size;
		double blimp_step;
		if (ratio < 1.0)
			blimp_step = static_cast<double>(blimp_table.m_resolution) * ratio;
		else
			blimp_step = static_cast<double>(blimp_table.m_resolution);

		int index = static_cast<int>(phase);
		double offset = (phase - index) * blimp_step;
		double	output = 0.0;
		double filt_sum = 0.0;

		// Backward pass
		double	bkwd_filt_phase = offset;
		int		bkwd_table_index = index;
		while (bkwd_filt_phase < blimp_table.size()) {
#ifdef DO_LERP_FOR_SINC
			double bkwd_filt_sample = blimp_table.getlinear(filt_phase / blimp_table.size());
#else
			double bkwd_filt_sample = blimp_table.getraw(static_cast<int>(bkwd_filt_phase));
#endif
			if (bkwd_table_index < 0) {
				bkwd_table_index = size - 1;
			}
			output += bkwd_filt_sample * table[bkwd_table_index--];
			bkwd_filt_phase += blimp_step;
			filt_sum += bkwd_filt_sample;
	}

		// Forward pass
		double	fwd_filt_phase = blimp_step - offset;
		int		fwd_table_index = index + 1;
		while (fwd_filt_phase < blimp_table.size()) {
#ifdef DO_LERP_FOR_SINC
			double fwd_filt_sample = blimp_table.getlinear(filt_phase / blimp_table.size());
#else
			double fwd_filt_sample = blimp_table.getraw(static_cast<int>(fwd_filt_phase));
#endif
			if (fwd_table_index >= size) {
				fwd_table_index = 0;
			}
			output += fwd_filt_sample * table[fwd_table_index++];
			fwd_filt_phase += blimp_step;
			filt_sum += fwd_filt_sample;
		}

		return output / filt_sum;
}
}