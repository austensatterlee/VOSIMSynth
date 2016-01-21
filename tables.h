#ifndef __TABLES__
#define __TABLES__
/*::macro_defs::*/
#define BLIMP_INTERVALS 11
#define BLIMP_RES 2048
/*::/macro_defs::*/
//todo: add oversampling and bias info to the LookupTable class so that it can still be accessed with a double between some arbitrary min and max.
namespace syn
{
	class LookupTable
	{
	public:
		LookupTable(const double* table, int size, double input_min = 0, double input_max = 1, bool isPeriodic = true) :
			m_table(table),
			m_size(size),
			m_isperiodic(isPeriodic)
		{
			m_norm_bias = input_min;
			m_norm_scale = 1. / (input_max - input_min);
			m_normalizePhase = (m_norm_bias != 0 && m_norm_scale != 1);
		}
		LookupTable() :
			LookupTable(nullptr, 0, 0, 1)
		{}
		double getlinear(double phase) const;
		double getresampled(double phase, double period) const;
		int size() const { return m_size; }
	private:
		int m_size;
		bool m_normalizePhase, m_isperiodic;
		double m_norm_bias;
		double m_norm_scale;
		const double* m_table;
	};

	/*::lut_defs::*/
	extern const double BL_SAW[256];
	extern const double PITCH_TABLE[16384];
	extern const double SIN[256];
	extern const double BLIMP_TABLE[11265];
	const LookupTable lut_bl_saw(BL_SAW, 256);
	const LookupTable lut_pitch_table(PITCH_TABLE, 16384, -1, 1, false);
	const LookupTable lut_sin(SIN, 256);
	const LookupTable lut_blimp_table(BLIMP_TABLE, 11265);
	/*::/lut_defs::*/

	inline double pitchToFreq(double pitch)
	{
		double freq = lut_pitch_table.getlinear(pitch*0.0078125);
		if (freq == 0)
			freq = 1;
		return freq;
	}
}
#endif