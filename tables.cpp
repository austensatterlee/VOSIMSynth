#include "tables.h"
#include "DSPMath.h"

namespace syn
{
	LookupTable::LookupTable(const double* table, int size, double input_min, double input_max, bool isPeriodic):
		m_table(table),
		m_size(size),
		m_isperiodic(isPeriodic) {
		m_norm_bias = input_min;
		m_norm_scale = 1. / (input_max - input_min);
		m_normalizePhase = (m_norm_bias != 0 && m_norm_scale != 1);
		/* Construct difference table for linear interpolation */
		m_diff_table = new double[size];
		for (int i = 0; i < size - 1; i++) {
			m_diff_table[i] = table[i + 1] - table[i];
		}
		/* If table is periodic, the last difference wraps around */
		if (isPeriodic)
			m_diff_table[m_size - 1] = m_table[0] - m_table[m_size - 1];
		else
			m_diff_table[m_size - 1] = 0.0;
	}

	double LookupTable::getlinear(double phase) const {
		if (m_normalizePhase) {
			phase = (phase - m_norm_bias) * m_norm_scale;
		}
		if (m_isperiodic)
			phase = WRAP(phase, 1.0);
		else
			phase = CLAMP(phase, 0.0, 1.0);

		phase *= m_size;

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

	ResampledLookupTable::ResampledLookupTable(const double* table, int size, const LookupTable& blimp_table) :
		LookupTable(table, size, 0, 1, true),
		m_blimp_table(blimp_table) {
		/* Construct resampled tables at ratios of powers of two */
		m_num_resampled_tables = MAX(1,log2(m_size)-4);
		m_resampled_tables = new double*[m_num_resampled_tables];
		m_resampled_sizes = new int[m_num_resampled_tables];
		double currsize = m_size;
		for (int i = 0; i < m_num_resampled_tables; i++) {
			m_resampled_tables[i] = new double[currsize];
			m_resampled_sizes[i] = currsize;
			resample_table(m_table, m_size, m_resampled_tables[i], currsize, m_blimp_table);
			currsize *= 0.5;
		}
	}

	double ResampledLookupTable::getresampled(double phase, double period) const {
		int min_size_diff = -1;
		int min_size_diff_index = 0;
		int curr_size_diff;
		phase = WRAP(phase, 1.0);
		for (int i = 0; i < m_num_resampled_tables; i++) {
			curr_size_diff = m_resampled_sizes[i] - static_cast<int>(period);
			if (curr_size_diff < 0) {
				break;
			} 
			if (curr_size_diff < min_size_diff || min_size_diff == -1) {
				min_size_diff = curr_size_diff;
				min_size_diff_index = i;
			}
		}
		return getresampled_single(m_resampled_tables[min_size_diff_index], m_resampled_sizes[min_size_diff_index], phase, period, m_blimp_table);
	}

	void resample_table(const double* table, int size, double* resampled_table, double period, const LookupTable& blimp_table) {
		double phase = 0;
		double phase_step = 1. / period;
		double maxval = 0;
		for (int i = 0; i < period; i++ , phase += phase_step) {
			resampled_table[i] = getresampled_single(table, size, phase, period, blimp_table);
			maxval = MAX(resampled_table[i], maxval);
		}
		/* normalize */
		if (maxval != 0) {
			for (int i = 0; i < period; i++) {
				resampled_table[i] /= maxval;
			}
		}
	}

	double getresampled_single(const double* table, int size, double phase, double period, const LookupTable& blimp_table) {
		double ratio = period * (1.0 / size);
		phase = WRAP(phase, 1.0);
		phase = phase * size;
		int index = static_cast<int>(phase);
		double frac_index = (phase - index);
		double blimp_step;
		if (ratio < 1.0)
			blimp_step = static_cast<double>(BLIMP_RES) * ratio;
		else
			blimp_step = static_cast<double>(BLIMP_RES);

		double offset = frac_index * blimp_step;

		double output = 0.0;
		double filt_phase = offset;
		double filt_sample;
		int table_index = index;
		while (filt_phase < blimp_table.size()) {
			filt_sample = blimp_table.getraw(static_cast<int>(filt_phase));
			if (table_index < 0) {
				table_index = size - 1;
			}
			output += filt_sample * table[table_index--];
			filt_phase += blimp_step;
		}
		filt_phase = blimp_step - offset;
		table_index = index + 1;
		while (filt_phase < blimp_table.size()) {
			filt_sample = blimp_table.getraw(static_cast<int>(filt_phase));
			if (table_index >= size) {
				table_index = 0;
			}
			output += filt_sample * table[table_index++];
			filt_phase += blimp_step;
		}
		if (ratio < 1)
			output *= 2.66 * 0.5 * 0.83 * ratio;
		else
			output *= 2.66 * 0.5 * 0.83;
		return output;
	}
}

