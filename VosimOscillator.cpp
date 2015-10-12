#include "VosimOscillator.h"

/******************************
* VOSIM methods
*
******************************/

double VosimOscillator::process()
{
  Oscillator::process();
  double number = getParam("number") * 4;
  if (m_UseRelativeWidth)
  {
    m_PhaseScale = pitchToFreq(getParam("pulsepitch") / 128.0 * (108 - getParam("pitch") - 12 * (number - 1)) + getParam("pitch") + 12 * (number - 1)) / (m_Step*m_Fs);
  }
  else
  {
    m_PhaseScale = pitchToFreq(getParam("pulsepitch") / 128.0 * (96 - 69) + 69) / (m_Step*m_Fs);
  }
  // add compensation for phase scales < 0.5 (i.e. won't be able to reach pulse peak)
  m_CompensationGain = 1.0;
  if (m_PhaseScale < 0.5)
  {
    double lastvalue = lut_vosim_pulse_cos.getlinear(m_PhaseScale);
    if (lastvalue)
      m_CompensationGain = (1. / lastvalue);
  }
  else if (number > 0 && number <= 0.5)
  {
    double lastvalue = lut_vosim_pulse_cos.getlinear(number);
    if (lastvalue)
      m_CompensationGain = (1. / lastvalue);
  }

  double vout;
  double pulsePhase = m_Phase * m_PhaseScale;
  int N = (int)pulsePhase;
  int lastN = (int)m_LastPulsePhase;
  if (!N)
    m_CurrPulseGain = 1.0;
  else if (N != lastN)
    m_CurrPulseGain *= getParam("decay");
  double wrPulsePhase = (pulsePhase - N);
  if (pulsePhase >= number)
  {
#ifdef USEBLEP
    double lastWrPulsePhase = (mLastPulsePhase - lastN);
    if (useMinBleps && mLastPulsePhase <= mNumber)
    {
      int wtindex;
      double wtfrac;
      wtindex = ((VOSIM_PULSE_COS_SIZE - 1) * lastWrPulsePhase);
      wtfrac = ((VOSIM_PULSE_COS_SIZE - 1) * lastWrPulsePhase) - wtindex;
      vout = mCurrPulseGain * LERP(VOSIM_PULSE_COS[wtindex], VOSIM_PULSE_COS[wtindex + 1], wtfrac);
      addBlep((pulsePhase)* mpPulseFreq / (m_Fs), vout);
    }
#endif
    vout = 0;
  }
  else
  {
    double tableval = lut_vosim_pulse_cos.getlinear(wrPulsePhase);
    _ASSERT(1. / m_CompensationGain > tableval);
    vout = m_CurrPulseGain*m_CompensationGain*tableval;
  }
  if (isSynced())
  {
#ifdef USEBLEP
    if (useMinBleps)
      addBlep((mStep + mPhase - 1)*mFreq / m_Fs, vout);
#endif
  }
#ifdef USEBLEP
  vout += mBlepBuf[mBlepBufInd];
#endif
  m_LastPulsePhase = pulsePhase;
  return vout;
}