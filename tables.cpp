#include "tables.h"
namespace syn
{
  double LookupTable::getlinear(double phase) const
  {
    if (m_normalizePhase)
    {
      phase = (phase - m_norm_bias)*m_norm_scale;
    }
    phase *= (m_size - 1);
    while (phase >= m_size) {
      if (m_isperiodic) phase -= m_size;
      else phase = m_size - 1;
    }
    while (phase < 0) {
      if (m_isperiodic) phase += m_size;
      else phase = 0;
    }

    int int_index = int(phase);
    int next_int_index = int_index + 1;
    if (m_isperiodic && next_int_index >= m_size) next_int_index -= m_size;
    else if (!m_isperiodic && next_int_index >= m_size) next_int_index = m_size - 1;

    double frac_index = phase - int_index;
    double val1 = m_table[int_index];
    double val2 = m_table[next_int_index];
    return LERP(val1, val2, frac_index);
  }
}