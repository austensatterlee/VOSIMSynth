#include "Oscillator.h"
#include <cmath>
extern double VOSIM_PULSE_COS[1024];

/******************************
 * Oscillator methods
 *
 ******************************/
void Oscillator::setFreq(double freq) {
	mTargetFreq = freq;
	mStep = (int32_t)((freq / mFs) * 0x7FFFFFFF);
}

void Oscillator::setGain(double gain) {
	mTargetGain = mGain = gain;
}

void Oscillator::modFreq(double modAmt) {
	mFreqMod += modAmt;
}

void Oscillator::modGain(double modAmt) {
	mGainMod *= modAmt;
}

void Oscillator::applyMods() {
	mFreq = mTargetFreq*(1 + mFreqMod);
	mStep = (int32_t)((mFreq / mFs) * 0x7FFFFFFF);
	mGain = mTargetGain*mGainMod;
	mFreqMod = 0;
	mGainMod = 1;
}

void Oscillator::tick() {
	mPhase += mStep;
}

void Oscillator::setFs(double fs) {
	mFs = fs;
	setFreq(mTargetFreq);
}

double Oscillator::getOutput() {
	switch (mWaveform) {
	case SAW_WAVE:
		return mGain*((double)mPhase * (4.656612875245797e-10));
		break;
	case SINE_WAVE:
		return mGain*2*(0.5-VOSIM_PULSE_COS[(uint16_t)(0x1FF*(1+(double)mPhase * (4.656612875245797e-10)))]);
		break;
	default:
		return 0;
		break;
	}
	
}
/******************************
* VOSIM methods
*
******************************/
void VOSIM::applyMods() {
	Oscillator::applyMods();
	mPFreq = mTargetPFreq*(1+mPFreqMod);
	mDecay = std::fmax(0,std::fmin(1,((mTargetDecay-1)*mDecayMod)+1));
	mDecayMod = 1;
	mPFreqMod = 0;
	refreshPhaseScale();
}

void VOSIM::refreshPhaseScale() {
	mPhaseScale = 0.5 * mPFreq / mFreq;
}
double VOSIM::getOutput() {
	double innerPhase = ((double)mPhase * (4.656612875245797e-10) + 1) * mPhaseScale;
	uint32_t N = (uint32_t)innerPhase;
	if (!N)
		mAttenuation = mGain;
	else if (N != mLastN)
		mAttenuation *= mDecay;
	uint16_t pulsePhase = 0x3FF*(innerPhase - N);
	if (N >= mNumber) {
		return 0;
	} else {
		return mAttenuation*VOSIM_PULSE_COS[pulsePhase];
	}
	mLastN = N;
}

/******************************
 * Envelope methods
 * 
 ******************************/
void Envelope::setPeriod(int segment, double period, double shape) {
	mTargetPeriod[segment] = period;
	period = period * mFs;
	mShape[segment] = shape;
	mMult[segment] = exp(-log((abs(mPoint[segment + 1] - mPoint[segment]) + shape) / shape) / period);
	mBase[segment] = (mPoint[segment+1] - mPoint[segment] + shape)*(1.0 - mMult[segment]);
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
		setPeriod(i,mTargetPeriod[i],mShape[i]);
	}
}

void Envelope::trigger() {
	mCurrSegment = 0;
	mOutput = 0.0;
	mIsDone = false;
}

void Envelope::release() {
	mCurrSegment = mNumSegments - 1;
	mMult[mCurrSegment] = exp(-log((abs(mPoint[mCurrSegment + 1] - mOutput) + mShape[mCurrSegment]) / mShape[mCurrSegment]) / (mTargetPeriod[mCurrSegment]*mFs));
	mBase[mCurrSegment] = (mPoint[mCurrSegment + 1] - mOutput + mShape[mCurrSegment])*(1.0 - mMult[mCurrSegment]);
}

void Envelope::tick() {
	if (mIsDone) {
		return;
	}
	if ((mBase[mCurrSegment]>0 && mOutput >= mPoint[mCurrSegment + 1]) || 
		(mBase[mCurrSegment]<0 && mOutput <= mPoint[mCurrSegment + 1])) {
		if (mCurrSegment < mNumSegments - 2) {
			mCurrSegment++;
		} else {
			if (mCurrSegment == mNumSegments-1)
				mIsDone = true;
		}
	}else{
		mOutput = mBase[mCurrSegment]+mMult[mCurrSegment]*mOutput;
	}
}