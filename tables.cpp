#include "tables.h" 
namespace syn
{
  double LookupTable::getlinear(const double phase) const
  {
    double normphase = (phase - m_norm_bias)*m_norm_scale*m_size;
    normphase = normphase > m_size - 1 ? m_size - 1 : normphase;
    normphase = normphase < 0 ? 0 : normphase;
    int int_index = (int)(normphase);
    double frac_index = normphase - int_index;
    return LERP(m_table[int_index], m_table[int_index + 1], frac_index);
  }
}