#include "VosimOscillator.h"

/******************************
* VOSIM methods
*
******************************/
namespace syn
{

  VosimOscillator::VosimOscillator(const VosimOscillator& vosc) :
    VosimOscillator(vosc.m_name)      
  {
    m_curr_pulse_gain = vosc.m_curr_pulse_gain;
    m_pulse_step = vosc.m_pulse_step;
    m_pulse_phase = vosc.m_pulse_phase;
    m_unwrapped_pulse_phase = vosc.m_unwrapped_pulse_phase;
  }

  void VosimOscillator::tick_pulse_phase()
  {
    double number = m_number;
    double pitch = m_pitchshift;
    double pitchshift = m_pitchshift;
    double pulse_pitch = m_ppitch;
    m_pulse_step = m_Step*(number + 2 * pulse_pitch);
    m_last_pulse_phase = m_pulse_phase;
    m_pulse_phase += m_pulse_step;
    if (m_pulse_phase >= 1)
    {
      m_pulse_phase -= 1;
      m_curr_pulse_gain *= m_decay;
    }
    m_unwrapped_pulse_phase += m_pulse_step;
  }

  void VosimOscillator::process()
  {
    Oscillator::tick_phase();
    if (m_isSynced)
    {
      m_pulse_phase = m_phase;
      m_unwrapped_pulse_phase = m_pulse_phase;
      m_curr_pulse_gain = 1.0;
    }
    if (m_unwrapped_pulse_phase >= m_number)
    {
      m_output = 0;
    }
    else
    {
      tick_pulse_phase();
      double tableval = lut_vosim_pulse_cos.getlinear(m_pulse_phase);
      m_output = m_gain*m_velocity*m_curr_pulse_gain*tableval;
    }
  }

  void VosimOscillator::sync()
  {
    Oscillator::sync();
    m_pulse_phase = 0;
    m_unwrapped_pulse_phase = 0;
  }

}