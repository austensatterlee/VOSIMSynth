#include "Oscillator.h"

namespace syn
{


  /******************************
  * Oscillator methods
  *
  ******************************/

  void Oscillator::noteOn(int pitch, int vel)
  {
    sync();
    m_pitch.mod(pitch, SET);
    m_velocity = vel / 255.0;
  }

  void Oscillator::setFs(double fs)
  {
    m_targetStep = m_targetStep*m_Fs / fs; m_Fs = fs;
  }
  
  /**
   * \brief
   * \todo Maybe let Unit inheritors implement a function that gets called only when some parameters have changed
   */
  void Oscillator::tick_phase()
  {
    if (m_pitch.isDirty() || m_pitchshift.isDirty())
    {
      m_targetStep = pitchToFreq(m_pitch + m_pitchshift) / m_Fs;
    }
    m_Step = m_targetStep;

    m_basePhase += m_Step;
    if (m_basePhase >= 1)
      m_basePhase -= 1;
    m_phase = m_basePhase + m_phaseshift;
    while (m_phase >= 1) m_phase -= 1;
    while (m_phase < 0) m_phase += 1;
    updateSyncStatus();
  }

  void BasicOscillator::process(int bufind)
  {
    tick_phase();
    double output;
    switch ((int)m_waveform)
    {
    case SAW_WAVE:
      output = lut_bl_saw.getlinear(m_phase);
      break;
    case NAIVE_SAW_WAVE:
      output = 2 * (m_phase - 0.5);
      break;
    case SINE_WAVE:
      output = 2 * (0.5 - lut_vosim_pulse_cos.getlinear(m_phase));
      break;
    case TRI_WAVE:
      output = m_phase <= 0.5 ? 4 * m_phase - 1 : -4 * (m_phase - 0.5) + 1;
      break;
    case SQUARE_WAVE:
      output = lut_bl_saw.getlinear(m_phase - 0.5) - lut_bl_saw.getlinear(m_phase);
      break;
    case NAIVE_SQUARE_WAVE:
      output = m_phase <= 0.5 ? -1 : 1;
      break;
    default:
      output = 0;
      break;
    }
    m_output[bufind] = m_velocity*m_gain*output;
  }

}