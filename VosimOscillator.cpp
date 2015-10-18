#include "VosimOscillator.h"

/******************************
* VOSIM methods
*
******************************/
namespace syn
{
	
  VosimOscillator::VosimOscillator(const VosimOscillator& vosc) :
    Oscillator(vosc)
  {
    m_UseRelativeWidth = vosc.m_UseRelativeWidth;
    m_CurrPulseGain = vosc.m_Step;
    m_PhaseScale = vosc.m_PhaseScale;
    m_LastPulsePhase = vosc.m_LastPulsePhase;
  }

  double VosimOscillator::process()
{
	  Oscillator::tick_phase();
	  double number = readParam(4) * 8;
	  if (m_UseRelativeWidth)
	  {
	    m_PhaseScale = pitchToFreq(readParam(3) * (108 - readParam(1) - 12 * (number - 1)) + readParam(1) + 12 * (number - 1)) / (m_Step*m_Fs);
	  }
	  else
	  {
	    m_PhaseScale = pitchToFreq(readParam(3) * (96 - 69) + 69) / (m_Step*m_Fs);
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
	    m_CurrPulseGain *= readParam(2);
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
      m_extSyncPort.Emit();
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
}