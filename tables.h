#ifndef __TABLES__
#define __TABLES__

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
	extern const double BLIMP_TABLE_OFFLINE[154625];
	extern const double BLIMP_TABLE_ONLINE[11265];
	extern const double PITCH_TABLE[8192];
	extern const double BL_SAW[2048];
	extern const double SIN[2048];
	const BlimpTable lut_blimp_table_offline(BLIMP_TABLE_OFFLINE, 154625, 151, 2048);
	const BlimpTable lut_blimp_table_online(BLIMP_TABLE_ONLINE, 11265, 11, 2048);
	const LookupTable lut_pitch_table(PITCH_TABLE, 8192, -1, 1, false);
	const ResampledLookupTable lut_bl_saw(BL_SAW, 2048, lut_blimp_table_online, lut_blimp_table_offline);
	const ResampledLookupTable lut_sin(SIN, 2048, lut_blimp_table_online, lut_blimp_table_offline);
	/*::/lut_defs::*/

	inline double pitchToFreq(double pitch)
	{
		double freq = lut_pitch_table.getlinear(pitch*0.0078125);
		if (freq == 0)
			freq = 1;
		return freq;
	}

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