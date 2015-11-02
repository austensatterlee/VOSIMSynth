#include "tables.h" 
namespace syn
{
  double LookupTable::getlinear(double phase) const
  {
    if(m_normalizePhase){
      phase = (phase - m_norm_bias)*m_norm_scale;
    }
    phase*=m_size;
    if (phase > m_size - 1)
    {
      phase = m_size-1;
    }
    else if (phase < 0)
    {
      phase = 0;
    }
    int int_index = (int)(phase);
    double frac_index = phase - int_index;
    return LERP(m_table[int_index], m_table[int_index + 1], frac_index);
  }
}