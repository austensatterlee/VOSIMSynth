#include "Oscillator.h"
#include "DSPMath.h"

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
    m_velocity = DBToAmp(LERP(-30,0,vel / 127.0));
  }

	void Oscillator::onSampleRateChange(double newfs)
	{
		m_Step = m_Step * m_Fs / newfs;
		m_Period = 1. / m_Step;
		m_Fs = newfs;
	}

	void Oscillator::onBufferSizeChange(size_t newbuffersize)
	{
	}

	void Oscillator::onTempoChange(double newtempo)
	{
	}

	void Oscillator::update_step()
  {
    if (m_pitch.isDirty() || m_finetune.isDirty())
    {
	  m_Period = m_Fs / pitchToFreq(m_pitch + m_finetune);
	  m_Step = 1./m_Period;
	  m_pitch.setClean();
	  m_finetune.setClean();
    }
  }

  /**
   * \brief
   * \todo Maybe let Unit inheritors implement a function that gets called only when some parameters have changed
   */
  void Oscillator::tick_phase()
  {
    update_step();
    m_basePhase += m_Step;
    if (m_basePhase >= 1)
      m_basePhase -= 1;
    m_phase = m_basePhase + m_phaseshift;
    if (m_phase >= 1) m_phase -= 1;
    else if (m_phase < 0) m_phase += 1;
    updateSyncStatus();
  }

  void BasicOscillator::process(int bufind)
  {
    tick_phase();
    double output;
    switch ((int)m_waveform)
    {
    case SAW_WAVE:
      output = lut_bl_saw.getresampled(m_phase,m_Period);
      break;
    case NAIVE_SAW_WAVE:
      output = 2 * (m_phase - 0.5);
      break;
    case SINE_WAVE:
      output = lut_sin.getresampled(m_phase, m_Period);
      break;
    case TRI_WAVE:
      output = m_phase <= 0.5 ? 4 * m_phase - 1 : -4 * (m_phase - 0.5) + 1;
      break;
    case SQUARE_WAVE:
      output = -lut_bl_saw.getresampled(m_phase+0.5, m_Period) + lut_bl_saw.getresampled(m_phase, m_Period);
      break;
    case NAIVE_SQUARE_WAVE:
      output = m_phase <= 0.5 ? -1 : 1;
      break;
    default:
      output = 0;
      break;
    }
    m_output[bufind] = m_velocity * m_gain * output;
  }
}

