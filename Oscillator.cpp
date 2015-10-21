#include "Oscillator.h"

namespace syn{

  Oscillator::Oscillator(const Oscillator& osc) :
  Oscillator(osc.m_name)
  {
    m_Phase = osc.m_Phase;
    m_Step = osc.m_Step;
    m_Waveform = m_Waveform;
  }

  void Oscillator::noteOn(int pitch, int vel)
{
  modifyParameter(1,pitch,SET);
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
  if(isSynced())
    m_extSyncPort.Emit();
  return output;
}

void Oscillator::tick_phase()
{
  if(getParam(1).wasDirty() || getParam(2).wasDirty()){
    m_Step = pitchToFreq(readParam(1) + readParam(2)) / m_Fs;
  }
  
  m_Phase += m_Step;
  if (m_Phase > 1)
    m_Phase -= 1;
#ifdef USEBLEP
  if (useMinBleps && nInit)
  {
    mBlepBufInd++;
    if (mBlepBufInd >= BLEPBUF)
      mBlepBufInd = 0;
    nInit--;
  }
#endif

}

#ifdef USEBLEP
void Oscillator::addBlep(double offset, double ampl) {
  int i;
  int blepos = BLEPOS;
  int lpIn = blepos*offset;
  int lpOut = mBlepInd;
  int cBlep = BLEPBUFSIZE - 1;
  double frac = blepos*offset - lpIn;
  double f;
  for (i = 0; i < mBlepCurrSize; i++, lpIn += blepos, lpOut++) {
    if (lpOut >= cBlep)
      lpOut = 0;
    f = LERP(MINBLEP[lpIn], MINBLEP[lpIn + 1], frac);
    mBlepBuf[lpOut] += ampl*(1 - f);
  }
  for (; i < cBlep; i++, lpIn += blepos, lpOut++) {
    if (lpOut >= cBlep)
      lpOut = 0;
    f = LERP(MINBLEP[lpIn], MINBLEP[lpIn + 1], frac);
    mBlepBuf[lpOut] = ampl*(1 - f);
  }
  mBlepCurrSize = cBlep;
}

#endif
}