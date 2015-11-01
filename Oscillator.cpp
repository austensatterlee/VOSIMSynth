#include "Oscillator.h"

namespace syn{


  /******************************
  * Oscillator methods
  *
  ******************************/

void Oscillator::noteOn(int pitch, int vel)
{
  sync();
  m_pitch.mod(pitch,SET);
  m_velocity = vel/255.0; /// \todo user and noteon must both be able to scale the gain and have it last for multiple sample
}

void Oscillator::noteOff(int pitch, int vel)
{

}

double BasicOscillator::process()
{
  tick_phase();
  double output;
  switch ((int)m_waveform) {
  case SAW_WAVE:
    output = (m_Phase * 2 - 1);
    break;
  case SINE_WAVE:
    output = 2 * (0.5 - lut_vosim_pulse_cos.getlinear(m_Phase));
    break;
  case TRI_WAVE:
    output = m_Phase<=0.5 ? 4*m_Phase-1 : -4*(m_Phase-0.5)+1;
    break;
  case SQUARE_WAVE:
    output = m_Phase<=0.5 ? -1 : 1;
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
  m_Step = pitchToFreq(m_pitch + m_maxPitchShift*m_pitchshift) / m_Fs;  
  
  m_Phase += m_Step;
  if (m_Phase > 1)
    m_Phase -= 1;

  if (isSynced())
  {
    m_isSynced = true;
  }
}

}