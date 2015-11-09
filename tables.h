#ifndef __TABLES__
#define __TABLES__
#define LERP(A,B,F) (((B)-(A))*(F)+(A))

//todo: add oversampling and bias info to the LookupTable class so that it can still be accessed with a double between some arbitrary min and max.
namespace syn
{

  class LookupTable
  {
  public:
    LookupTable(const double* table, int size, double input_min = 0, double input_max = 1, bool isPeriodic=true) :
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
    double getlinear(const double phase) const;
  private:
    int m_size;
    bool m_normalizePhase, m_isperiodic;
    double m_norm_bias;
    double m_norm_scale;
    const double* m_table;
  };

  /*::automated::*/
  extern const double BL_SAW[1024];
extern const double PITCH_TABLE[65536];
extern const double VOSIM_PULSE_COS[65536];

const LookupTable lut_bl_saw(BL_SAW, 1024);
const LookupTable lut_pitch_table(PITCH_TABLE, 65536, -1, 1, false);
const LookupTable lut_vosim_pulse_cos(VOSIM_PULSE_COS, 65536);

  /*::/automated::*/
}
#endif