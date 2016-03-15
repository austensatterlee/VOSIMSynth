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
#include "table_data.h"

/*::macro_defs::*/
/*::/macro_defs::*/
namespace syn
{
	class LookupTable
	{
	public:
		LookupTable(const double* table, int size, double input_min = 0, double input_max = 1, bool isPeriodic = true);

		virtual ~LookupTable() {
			delete[] m_diff_table;
		}
		double getlinear(double phase) const;
		double getraw(int index) const;
		int size() const { return m_size; }
	protected:
		int m_size;
		bool m_normalizePhase, m_isperiodic;
		double m_norm_bias;
		double m_norm_scale;
		const double* m_table;
		double* m_diff_table;
	};

	class BlimpTable : public LookupTable
	{
	public:
		BlimpTable(const double* table, int size, int a_num_intervals, int a_resolution)
			: LookupTable(table, size, 0.0, 1.0, false),
			  m_num_intervals(a_num_intervals),
			  m_resolution(a_resolution) {}
		const int m_num_intervals;
		const int m_resolution;		
	};

	class ResampledLookupTable : public LookupTable
	{
	public:
		/// Construct a table that resamples itself with the specified blimp table
		ResampledLookupTable(const double* table, int size, const BlimpTable& blimp_table_online, const BlimpTable& blimp_table_offline);
		void resample_tables(const BlimpTable& blimp_table_offline);

		virtual ~ResampledLookupTable() {
			for (int i = 0; i < m_num_resampled_tables; i++) {
				delete[] m_resampled_tables[i];
			}
			delete[] m_resampled_tables;
			delete[] m_resampled_sizes;
		}

		/// Retrieve a single sample from the table at the specified phase, as if the table were resampled to have the given period.
		double getresampled(double phase, double period) const;
	protected:
		int m_num_resampled_tables;
		int* m_resampled_sizes;
		double** m_resampled_tables;
		const BlimpTable& m_blimp_table;
	};

	/*::lut_defs::*/	
	const BlimpTable lut_blimp_table_offline(BLIMP_TABLE_OFFLINE, 32833, 513, 128);
	const BlimpTable lut_blimp_table_online(BLIMP_TABLE_ONLINE, 20481, 5, 8192);
	const LookupTable lut_pitch_table(PITCH_TABLE, 1024, -128, 128, false);
	const ResampledLookupTable lut_bl_saw(BL_SAW, 2048, lut_blimp_table_online, lut_blimp_table_offline);
	const LookupTable lut_sin(SIN, 1024, 0, 1, true);
	const LookupTable lut_db_table(DB_TABLE, 2048, -120, 0, false);
	/*::/lut_defs::*/

	/**
	 * Retrieve a single sample from table as if it were resampled to have the specified period. 
	 * Uses fractional sinc interpolation/decimation.
	 */
	double getresampled_single(const double* table, int size, double phase, double period, const BlimpTable& blimp_table);
	/**
	 * Resample an entire table to have the specified period and store the result in resampled_table (which should already be allocated).
	 * Uses fractional sinc interpolation/decimation.
	 */
	void resample_table(const double* table, int size, double* resampled_table, double period, const BlimpTable& blimp_table);
}
#endif