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

#ifndef __TABLES__
#define __TABLES__

//#define DO_LERP_FOR_SINC
#include <vector>
#include "lut_tables.h"

namespace syn
{
	class LookupTable
	{
	public:
		LookupTable(const double* table, int size, double input_min = 0, double input_max = 1, bool isPeriodic = true);

		LookupTable(const LookupTable& a_o) : 
			LookupTable(a_o.m_table, a_o.m_size, a_o.m_input_min, a_o.m_input_max, a_o.m_isperiodic) 
		{}

		virtual ~LookupTable() {
			
		};

		double getlinear(double phase) const;
		double getlinear_periodic(double phase) const;
		double getraw(int index) const;

		int size() const {
			return m_size;
		}

	protected:
		int m_size;
		double m_input_min, m_input_max;
		bool m_isperiodic;
		double m_norm_bias;
		double m_norm_scale;
		const double* m_table;
		std::vector<double> m_diff_table;
	};

	/**
	 *
	 */
	class BlimpTable : public LookupTable
	{
	public:
		BlimpTable(const double* a_table, int a_size, int a_num_intervals, int a_resolution)
			: LookupTable(a_table, a_size, 0.0, 1.0, false),
			  m_num_intervals(a_num_intervals),
			  m_resolution(a_resolution) {}

		virtual ~BlimpTable() {
			
		}

		BlimpTable(const BlimpTable& a_other)
			: BlimpTable(a_other.m_table, a_other.m_size, a_other.m_num_intervals, a_other.m_resolution) {}

		const int m_num_intervals;
		const int m_resolution;
	};

	/**
	 * Lookup table that can resample itself with sinc interpolation.
	 * Upon construction, the table computes and caches downsampled versions
	 * of itself. Each downsampled table is half the size of the previous,
	 * so log2(N) tables are created, where N is the size of the initial
	 * table.
	 */
	class ResampledLookupTable : public LookupTable
	{
	public:
		ResampledLookupTable(const double* a_table, int a_size, const BlimpTable& a_blimp_table_online, const BlimpTable& a_blimp_table_offline);

		ResampledLookupTable(const ResampledLookupTable& a_o) : 
			ResampledLookupTable(a_o.m_table, a_o.m_size, a_o.m_blimp_table_online, a_o.m_blimp_table_offline) 
		{}

		void resample_tables();

		virtual ~ResampledLookupTable() {
			
		}

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
	 * \param period the desired period to resample at (in fractional number of samples)
	 */
	double getresampled_single(const double* table, int size, double phase, double period, const BlimpTable& blimp_table);
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
	void resample_table(const double* table, int size, double* resampled_table, double period, const BlimpTable& blimp_table, bool normalize = true);

	/**
	 * \todo
	 */
	void fft_resample_table(const double* table, int size, double* resampled_table, double period);
}
#endif
