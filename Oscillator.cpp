#include "Oscillator.h"

/******************************
 * Oscillator methods
 *
 ******************************/
void Oscillator::updateParams() {
  m_pPitch();
  m_Step = std::fmin(pitchToFreq(m_pPitch.m_curr) / mFs, 0.5);
}

void Oscillator::tick() {
  m_Phase += m_Step;
  if (m_Phase > 1)
    m_Phase -= 1;
#ifdef USEBLEP
  if (useMinBleps && nInit) {
    mBlepBufInd++;
    if (mBlepBufInd >= BLEPBUF)
      mBlepBufInd = 0;
    nInit--;
  }
#endif
}

double Oscillator::process(const double input) {
  tick();
  updateParams();
  double output;
  switch (m_Waveform) {
  case SAW_WAVE:
    output = (m_Phase * 2 - 1);
    break;
  case SINE_WAVE:
    output = 2 * (0.5 - VOSIM_PULSE_COS[(int)(VOSIM_PULSE_COS_SIZE*m_Phase)]);
    break;
  default:
    output = 0;
    break;
  }
  if (isSynced()) {
    triggerOut();
  }
  return finishProcessing(output);
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
/******************************
* VOSIM methods
*
******************************/
void VOSIM::updateParams() {
  Oscillator::updateParams();
  mpNumber.scale(4);
  mpNumber();
  mpPulsePitch();
  if (m_UseRelativeWidth) {
    mPhaseScale = pitchToFreq(mpPulsePitch.m_curr / 128.0 * (108 - m_pPitch.m_curr) + m_pPitch.m_curr) / (m_Step*mFs);
  }
  else {
    mPhaseScale = pitchToFreq(mpPulsePitch.m_curr / 128.0 * (96-69) + 69) / (m_Step*mFs);
  }
  // add compensation for phase scales < 0.5 (i.e. won't be able to reach pulse peak)
  m_CompensationGain = 1.0;
  if (mPhaseScale < 0.5 && mpNumber.m_curr >= mPhaseScale) {
    double lastvalue = lut_vosimpulse.getlinear(mPhaseScale);
    if(lastvalue)
      m_CompensationGain = (1. / lastvalue);
  }
  else if (mpNumber.m_curr < 0.5) {
    double lastvalue = lut_vosimpulse.getlinear(mpNumber.m_curr);
    if (lastvalue)
      m_CompensationGain = (1. / lastvalue);
  }
  mpDecay();
}

double VOSIM::process(const double input) {
  Oscillator::tick();
  updateParams();
  double vout;
  double pulsePhase = m_Phase * mPhaseScale;
  int N = (int)pulsePhase;
  int lastN = (int)m_LastPulsePhase;
  if (!N)
    m_CurrPulseGain = 1.0;
  else if (N != lastN)
    m_CurrPulseGain *= mpDecay.m_curr;
  double wrPulsePhase = (pulsePhase - N);
  if (pulsePhase >= mpNumber.m_curr) {
#ifdef USEBLEP
    double lastWrPulsePhase = (mLastPulsePhase - lastN);
    if (useMinBleps && mLastPulsePhase <= mNumber) {
      int wtindex;
      double wtfrac;
      wtindex = ((VOSIM_PULSE_COS_SIZE - 1) * lastWrPulsePhase);
      wtfrac = ((VOSIM_PULSE_COS_SIZE - 1) * lastWrPulsePhase) - wtindex;
      vout = mCurrPulseGain * LERP(VOSIM_PULSE_COS[wtindex], VOSIM_PULSE_COS[wtindex + 1], wtfrac);
      addBlep((pulsePhase)* mpPulseFreq.m_curr / (mFs), vout);
    }
#endif
    vout = 0;
  }
  else {
    vout = m_CurrPulseGain*m_CompensationGain*lut_vosimpulse.getlinear(wrPulsePhase);
  }
  if (isSynced()) {
    triggerOut();
#ifdef USEBLEP
    if (useMinBleps)
      addBlep((mStep + mPhase - 1)*mFreq / mFs, vout);
#endif
  }
#ifdef USEBLEP
  vout += mBlepBuf[mBlepBufInd];
#endif
  m_LastPulsePhase = pulsePhase;
  return finishProcessing(vout);
}

/******************************
 * Envelope methods
 *
 ******************************/
void Envelope::setPeriod(int segment, double period, double shape) {
  /*
   * period - length of segment in seconds
   * shape - higher shape <=> more linear
   *
   */
  shape = shape == 0 ? m_segments[segment].shape : shape;
  m_segments[segment].period = period;
  m_segments[segment].shape = shape;

  period = period + MIN_ENV_PERIOD;
  shape = LERP(dbToAmp(-240), dbToAmp(0), shape) + MIN_ENV_SHAPE;

  m_segments[segment].mult = exp(-log((1 + shape) / shape) / (mFs*period));
  double prev_amp = segment > 0 ? m_segments[segment - 1].target_amp : m_initPoint;
  if (m_segments[segment].target_amp > prev_amp) {
    m_segments[segment].base = (m_segments[segment].target_amp + shape) * (1 - m_segments[segment].mult);
  }
  else {
    m_segments[segment].base = (m_segments[segment].target_amp - shape) * (1 - m_segments[segment].mult);
  }
}

/* Sets the shape of the specified segment. Sets the shape of all segments if first argument is -1 */
void Envelope::setShape(int segment, double shape) {
  if (segment == -1) {
    for (int i = 0; i < m_numSegments; i++) {
      setPeriod(i, m_segments[i].period, shape);
    }
  }
  else {
    setPeriod(segment, m_segments[segment].period, shape);
  }
}

/*
 * Sets the end point of the specified segment
 */
void Envelope::setPoint(int segment, double target_amp) {
  m_segments[segment].target_amp = target_amp;
  if (segment < m_numSegments - 1) {
    setPeriod(segment + 1, m_segments[segment + 1].period, m_segments[segment + 1].shape);
  }
  setPeriod(segment, m_segments[segment].period, m_segments[segment].shape);
}

double Envelope::process(const double input) {
  double prev_amp = m_currSegment > 0 ? m_segments[m_currSegment - 1].target_amp : m_initPoint;
  bool isIncreasing = m_segments[m_currSegment].target_amp > prev_amp;
  double output;
  if ((isIncreasing && m_lastRawOutput >= m_segments[m_currSegment].target_amp) || (!isIncreasing && m_lastRawOutput <= m_segments[m_currSegment].target_amp)) {
    output = m_segments[m_currSegment].target_amp;
    if (m_currSegment < m_numSegments - 2) {
      m_currSegment++;
    }
    else {
      if (m_isRepeating && m_currSegment == m_numSegments - 2) {
        trigger();
      }
      if (m_currSegment == m_numSegments - 1) {
        m_isDone = true;
      }
    }
  }
  else {
    output = m_segments[m_currSegment].base + m_segments[m_currSegment].mult * m_lastRawOutput;
  }
  m_lastRawOutput = output;
  return finishProcessing(output);
}

void Envelope::setFs(const double fs) {
  mFs = fs;
  for (int i = 0; i < m_numSegments; i++) {
    setPeriod(i, m_segments[i].period, m_segments[i].shape);
  }
}

void Envelope::trigger() {
  m_currSegment = 0;
  m_lastRawOutput = m_initPoint;
  m_isDone = false;
}

void Envelope::release() {
  m_RelPoint = m_lastRawOutput;
  m_currSegment = m_numSegments - 1;
}

int Envelope::getSamplesPerPeriod() const {
  double approx = 0;
  for (int i = 0; i < m_numSegments; i++) {
    approx += m_segments[i].period*mFs;
  }
  return (int)approx;
}