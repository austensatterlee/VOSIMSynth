#include "Oscillator.h"

/******************************
 * Oscillator methods
 *
 ******************************/
void Oscillator::update() {
  mpPitch();
  mStep = pitchToFreq(mpPitch.curr) / mFs;
  mpGain();
  mpGain.curr = pow(10.0, .05*mpGain.curr);
}

void Oscillator::tick() {
  mPhase += mStep;
  if (mPhase > 1)
    mPhase -= 1;
#ifdef USEBLEP
  if (useMinBleps && nInit) {
    mBlepBufInd++;
    if (mBlepBufInd >= BLEPBUF)
      mBlepBufInd = 0;
    nInit--;
  }
#endif
}

void Oscillator::setFs(double fs) {
  mFs = fs;
}

double Oscillator::getOutput() {
  double output;
  switch (mWaveform) {
  case SAW_WAVE:
    output = mpGain.curr*(mPhase * 2 - 1);
    break;
  case SINE_WAVE:
    output = mpGain.curr * 2 * (0.5 - VOSIM_PULSE_COS[(int)(VOSIM_PULSE_COS_SIZE*mPhase)]);
    break;
  default:
    output = 0;
    break;
  }
  mLastOutput = output;
  return output;
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
void VOSIM::update() {
  Oscillator::update();
  mpNumber();
  mpNumber.curr = mpNumber.curr * 8;
  mpPulsePitch();
  if (mUseRelativeWidth) {
    mPhaseScale = pitchToFreq(mpPulsePitch.curr/128.0*60 + mpPitch.curr + mpNumber.curr) / (mStep*mFs);
  }
  else {
    mPhaseScale = pitchToFreq(mpPulsePitch.curr) / (mStep*mFs);
  }
  mpDecay();
  mpDecay.curr = pow(10, 0.05*mpDecay.curr);
}

double VOSIM::getOutput() {
  double vout;
  double pulsePhase = mPhase * mPhaseScale;
  int N = (int)pulsePhase;
  int lastN = (int)mLastPulsePhase;
  if (!N)
    mCurrPulseGain = 1.0;
  else if (N != lastN)
    mCurrPulseGain *= mpDecay.curr;
  double wrPulsePhase = (pulsePhase - N);
  int wtindex;
  double wtfrac;
  if (pulsePhase >= mpNumber.curr) {
#ifdef USEBLEP
    double lastWrPulsePhase = (mLastPulsePhase - lastN);
    if (useMinBleps && mLastPulsePhase <= mNumber) {
      wtindex = ((VOSIM_PULSE_COS_SIZE - 1) * lastWrPulsePhase);
      wtfrac = ((VOSIM_PULSE_COS_SIZE - 1) * lastWrPulsePhase) - wtindex;
      vout = mCurrPulseGain * LERP(VOSIM_PULSE_COS[wtindex], VOSIM_PULSE_COS[wtindex + 1], wtfrac);
      addBlep((pulsePhase)* mpPulseFreq.curr / (mFs), vout);
    }
#endif
    vout = 0;
  }
  else {
    wtfrac = (VOSIM_PULSE_COS_SIZE - 1) * wrPulsePhase;
    wtindex = (int)wtfrac;
    wtfrac = wtfrac - wtindex;
    vout = mCurrPulseGain * LERP(VOSIM_PULSE_COS[wtindex], VOSIM_PULSE_COS[wtindex + 1], wtfrac);
  }
  if (mPhase + mStep >= 1.0) {
    if (mMaxAmp <= vout) {

    }
#ifdef USEBLEP
    if (useMinBleps)
      addBlep((mStep + mPhase - 1)*mFreq / mFs, vout);
#endif
  }
#ifdef USEBLEP
  vout += mBlepBuf[mBlepBufInd];
#endif
  mLastPulsePhase = pulsePhase;
  if (vout>mMaxAmp)
    mMaxAmp = vout;
  vout = mpGain.curr * vout;
  mLastOutput = vout;
  return vout;
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
  shape = shape == 0 ? mShape[segment] : shape;
  mTargetPeriod[segment] = period;
  mShape[segment] = shape;

  period = period + MIN_ENV_PERIOD;
  shape = shape + MIN_ENV_SHAPE;

  mMult[segment] = exp(-log((1 + shape) / shape) / (mFs*period));
  if (mPoint[segment + 1] > mPoint[segment]) {
    mBase[segment] = (mPoint[segment + 1] + shape) * (1 - mMult[segment]);
  }
  else {
    mBase[segment] = (mPoint[segment + 1] - shape) * (1 - mMult[segment]);
  }
}

void Envelope::setShape(int segment, double shape) {
  if (segment == -1) {
    for (int i = 0; i < mNumSegments; i++) {
      setPeriod(i, mTargetPeriod[i], shape);
    }
  }
  else {
    setPeriod(segment, mTargetPeriod[segment], shape);
  }
}

/*
 * Sets the gain of the specified node
 */
void Envelope::_setPoint(int node, double gain) {
  mPoint[node] = gain;
  if (node) {
    setPeriod(node - 1, mTargetPeriod[node - 1], mShape[node - 1]);
  }
  setPeriod(node, mTargetPeriod[node], mShape[node]);
}

/*
 * Sets the gain of the specified node (ignoring node 0, which should always have a gain of zero)
 */
void Envelope::setPoint(int node, double gain) {
  if (node < mNumSegments - 1) {
    _setPoint(node + 1, gain);
  }
}

/*
 * Same as setPoint, but gain is in decibels.
 */
void Envelope::setPoint_dB(int node, double dB) {
  if (node < mNumSegments - 1) {
    _setPoint(node + 1, pow(10, dB*0.05));
  }
}

double Envelope::getOutput() {
  return mOutput;
}

void Envelope::setFs(double fs) {
  mFs = fs;
  for (int i = 0; i < mNumSegments; i++) {
    setPeriod(i, mTargetPeriod[i], mShape[i]);
  }
}

void Envelope::trigger() {
  mCurrSegment = 0;
  mOutput = mPoint[0];
  mGate = 1;
  mIsDone = false;
}

void Envelope::release() {
  mRelpoint = getOutput();
  mGate = 0;
  mCurrSegment = mNumSegments - 1;
}

void Envelope::tick() {
  bool isIncreasing = mPoint[mCurrSegment + 1]>mPoint[mCurrSegment];
  if ((isIncreasing && mOutput >= mPoint[mCurrSegment + 1]) || (!isIncreasing && mOutput <= mPoint[mCurrSegment + 1])) {
    mOutput = mPoint[mCurrSegment + 1];
    if (mCurrSegment < mNumSegments - 2) {
      mCurrSegment++;
    }
    else {
      if (mRepeat && mCurrSegment == mNumSegments - 2) {
        trigger();
      }
      if (mCurrSegment == mNumSegments - 1) {
        mIsDone = true;
      }
    }
  }
  else {
    mOutput = mBase[mCurrSegment] + mMult[mCurrSegment] * mOutput;
  }
}