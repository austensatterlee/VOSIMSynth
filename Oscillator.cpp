#include "Oscillator.h"

namespace syn{

  Oscillator::Oscillator(const Oscillator& osc) :
  Oscillator(osc.m_name)
  {
    m_Phase = osc.m_Phase;
    m_Step = osc.m_Step;
    m_Waveform = osc.m_Waveform;
  }

  void Oscillator::noteOn(int pitch, int vel)
{
  m_pitch.mod(pitch,SET);
  m_velocity = vel/255.0; /// \todo user and noteon must both be able to scale the gain and have it last for multiple sample
}

void Oscillator::noteOff(int pitch, int vel)
{

}

/******************************
 * Oscillator methods
 *
 ******************************/
double Oscillator::process()
{
  tick_phase();
  double output;
  switch (m_Waveform) {
  case SAW_WAVE:
    output = (m_Phase * 2 - 1);
    break;
  case SINE_WAVE:
    output = 2 * (0.5 - lut_vosim_pulse_cos.getlinear(m_Phase));
    break;
  default:
    output = 0;
    break;
  }
  return m_velocity*m_gain*output;
}

/**
 * \brief 
 * \todo let Unit inheritors implement a function that gets called only when some parameters have changed
 */
void Oscillator::tick_phase()
{
  m_Step = pitchToFreq(m_pitch + m_pitchshift) / m_Fs;  
  
  m_Phase += m_Step;
  if (m_Phase > 1)
    m_Phase -= 1;

  if (isSynced())
  {
    m_isSynced = true;
  }
}

}