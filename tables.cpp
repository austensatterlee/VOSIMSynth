#include "tables.h"
#include "DSPMath.h"
namespace syn
{
  double LookupTable::getlinear(double phase) const
  {
    if (m_normalizePhase)
    {
      phase = (phase - m_norm_bias)*m_norm_scale;
    }
	if (m_isperiodic)
		phase = WRAP(phase, 1.0);	
    phase *= (m_size - 1);

    int int_index = int(phase);
    int next_int_index = int_index + 1;
    if (m_isperiodic && next_int_index >= m_size) next_int_index -= m_size;
    else if (!m_isperiodic && next_int_index >= m_size) next_int_index = m_size - 1;

    double frac_index = phase - int_index;
    double val1 = m_table[int_index];
    double val2 = m_table[next_int_index];
    return LERP(val1, val2, frac_index);
  }

  double LookupTable::getresampled(double phase, double period) const
  {
	  double ratio = period / m_size;
	  phase = phase*(m_size);
	  int index = static_cast<int>(phase);
	  double frac_index = (phase - index);
	  double blimp_step;
	  if(ratio<1)
		blimp_step = BLIMP_RES*ratio / lut_blimp_table.size();
	  else
		blimp_step = BLIMP_RES * 1.0/ lut_blimp_table.size();
	  double offset = frac_index*blimp_step;

	  double output = 0.0;
	  double filt_phase = offset;
	  int i = 0;
	  while(filt_phase < 1.0)
	  {
		double filt_sample = lut_blimp_table.getlinear(filt_phase);
		output += filt_sample * m_table[WRAP(index-i, m_size)];
		filt_phase += blimp_step;
		i++;
	  }
	  filt_phase = blimp_step - offset;
	  i = 1;
	  while (filt_phase < 1.0)
	  {
		  double filt_sample = lut_blimp_table.getlinear(filt_phase);
		  output += filt_sample * m_table[WRAP(index + i, m_size)];
		  filt_phase += blimp_step;
		  i++;
	  }
	  if (ratio < 1)
		  output *= ratio;
	  return output;
  }
}