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
		return mGain*((mPhase * (2.3283064370807974e-10))*2-1);
		break;
	case SINE_WAVE:
		return mGain*2*(0.5-VOSIM_PULSE_COS[(uint16_t)(0xFFF*(mPhase * (2.3283064370807974e-10)))]);
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
	if (mUseRelativeWidth) {
		mPFreq = (mNumber+64*mTargetPFreq) * mFreq * (2 + mPFreqMod);
	} else {
		mPFreq = 1000 * pow(2.0, 1 + 3 * mTargetPFreq) * (2 + mPFreqMod);
	}
	mDecay = std::fmin(0.9999,std::fmax(0,std::fmin(1,((mTargetDecay-1)*mDecayMod)+1)));
	mDecayMod = 1;
	mPFreqMod = 0.5;
	refreshPhaseScale();
}

void VOSIM::refreshPhaseScale() {
	mPhaseScale = mPFreq / mFreq;
}

double VOSIM::getOutput() {
	double innerPhase = (mPhase * (2.3283064370807974e-10)) * mPhaseScale;
	double N = floor(innerPhase);
	if (!N)
		mAttenuation = mGain;
	else if (N != mLastN)
		mAttenuation *= mDecay;
	uint32_t pulsePhase = 0xFFF*(innerPhase - N);
	if (N >= mNumber) {
		return 0;
	} else {
		return mAttenuation*2*VOSIM_PULSE_COS[pulsePhase];
	}
	mLastN = N;
}

/******************************
 * Envelope methods
 * 
 ******************************/
void Envelope::setPeriod(int segment, double period, double shape) {
	mTargetPeriod[segment] = period;
	mShape[segment] = shape;
	period = std::fmax(MIN_ENV_PERIOD, period);
	period = period * mFs;
	shape = std::fmax(MIN_ENV_SHAPE, shape);
	mMult[segment] = exp(-log((abs(mPoint[segment + 1] - mPoint[segment]) + shape) / shape) / period);
	mBase[segment] = (mPoint[segment+1] - mPoint[segment] + shape)*(1.0 - mMult[segment]);
}

void Envelope::setShape(int segment, double shape) {
	setPeriod(segment, mTargetPeriod[segment], shape);
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
	mOutput = mPoint[0];
	mIsDone = false;
}

void Envelope::release() {
	mCurrSegment = mNumSegments - 1;
	mMult[mCurrSegment] = exp(-log((abs(mPoint[mCurrSegment + 1] - mOutput) + mShape[mCurrSegment]) / mShape[mCurrSegment]) / (mTargetPeriod[mCurrSegment]*mFs));
	mBase[mCurrSegment] = (mPoint[mCurrSegment + 1] - mOutput + mShape[mCurrSegment])*(1.0 - mMult[mCurrSegment]);
}

void Envelope::tick() {
	if ((mBase[mCurrSegment]>0 && mOutput >= mPoint[mCurrSegment + 1]) || 
		(mBase[mCurrSegment]<0 && mOutput <= mPoint[mCurrSegment + 1])) {
		if (mCurrSegment < mNumSegments - 2) {
			mCurrSegment++;
		} else {
			if (mCurrSegment == mNumSegments - 1) {
				mIsDone = true;
			}
		}
	}else{
		mOutput = mBase[mCurrSegment]+mMult[mCurrSegment]*mOutput;
	}
}