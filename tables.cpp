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
		if (m_isperiodic){
			phase = WRAP(phase, 1.0);
			phase *= m_size;
		}else {
			phase = CLAMP(phase, 0.0, 1.0);
			phase *= m_size-1;
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

	ResampledLookupTable::ResampledLookupTable(const double* table, int size, const BlimpTable& blimp_table_online, const BlimpTable& blimp_table_offline) :
		LookupTable(table, size, 0, 1, true),
		m_blimp_table(blimp_table_online)
	{
		/*m_num_resampled_tables = 1;
		m_resampled_tables = static_cast<double**>(malloc(sizeof(double*)));
		m_resampled_tables[0] = static_cast<double*>(malloc(m_size*sizeof(double)));
		memcpy(m_resampled_tables[0], m_table, m_size*sizeof(double));
		m_resampled_sizes = static_cast<int*>(malloc(sizeof(int) * 1));
		m_resampled_sizes[0] = m_size;*/
		resample_tables(blimp_table_offline);
	}

	void ResampledLookupTable::resample_tables(const BlimpTable& blimp_table_offline)
	{
		/* Construct resampled tables at ratios of powers of two */
		m_num_resampled_tables = MAX(1, log2(m_size) - 3);
		m_resampled_tables = new double*[m_num_resampled_tables];
		m_resampled_sizes = new int[m_num_resampled_tables];
		double currsize = m_size;
		for (int i = 0; i < m_num_resampled_tables; i++) {
			m_resampled_tables[i] = new double[currsize];
			m_resampled_sizes[i] = currsize;
			resample_table(m_table, m_size, m_resampled_tables[i], currsize, blimp_table_offline);
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

	void resample_table(const double* table, int size, double* resampled_table, double period, const BlimpTable& blimp_table) {
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

	double getresampled_single(const double* table, int size, double phase, double period, const BlimpTable& blimp_table) {
		double ratio = period * (1.0 / size);
		phase = WRAP(phase, 1.0);
		phase = phase * size;
		int index = static_cast<int>(phase);
		double frac_index = (phase - index);
		double blimp_step;
		if (ratio < 1.0)
			blimp_step = static_cast<double>(blimp_table.m_resolution) * ratio;
		else
			blimp_step = static_cast<double>(blimp_table.m_resolution);

		double offset = frac_index * blimp_step;

		double filt_sum = 0;
		double output = 0.0;
		double filt_phase = offset;
		double filt_sample;
		int table_index = index;
		while (filt_phase < blimp_table.size()) {
			//filt_sample = blimp_table.getraw(static_cast<int>(filt_phase));
			filt_sample = blimp_table.getlinear(filt_phase/blimp_table.size());
			if (table_index < 0) {
				table_index = size - 1;
			}
			output += filt_sample * table[table_index--];
			filt_sum += filt_sample;
			filt_phase += blimp_step;
		}
		filt_phase = blimp_step - offset;
		table_index = index + 1;
		while (filt_phase < blimp_table.size()) {
			//filt_sample = blimp_table.getraw(static_cast<int>(filt_phase));
			filt_sample = blimp_table.getlinear(filt_phase / blimp_table.size());
			if (table_index >= size) {
				table_index = 0;
			}
			output += filt_sample * table[table_index++];
			filt_sum += filt_sample;
			filt_phase += blimp_step;
		}
		if (ratio < 1)
			output *= 1./filt_sum;
		else
			output *= 1./filt_sum;
		return output;
	}
}

