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
	mTargetGain = gain;
}

void Oscillator::tick() {
	mPhase += mStep;
}

void Oscillator::setFs(double fs) {
	mFs = fs;
	setFreq(mTargetFreq);
}

double Oscillator::getOutput() {
	return mGain*((double)mPhase * (4.656612875245797e-10));
}
/******************************
* VOSIM methods
*
******************************/
void VOSIM::setFreq(double freq) {
	Oscillator::setFreq(freq);
	refreshPhaseScale();
}
double VOSIM::getOutput() {
	double innerPhase = ((double)mPhase * (4.656612875245797e-10) + 1) * mPhaseScale;
	//double innerPhase = ((double)mPhase * (4.656612875245797e-10)) / (mWidth) * 0.5;
	uint32_t N = (uint32_t)innerPhase;
	mGain = mTargetGain;
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
void Envelope::setPeriod(int segment, double period) {
	mTargetPeriod[segment] = period;
	mStep[segment] = 1.0 / (mFs * (period + MIN_ENV_PERIOD));
	mMult[segment] = 1 + (log(mPoint[segment + 1]+MIN_ENV_PERIOD)-log(mPoint[segment]+MIN_ENV_PERIOD)) * mStep[segment];
}

/*
 * Sets the gain of the specified node
 */
void Envelope::_setPoint(int node, double gain) {
	mPoint[node] = gain;
	if (node) {
		setPeriod(node - 1, mTargetPeriod[node - 1]);
	}
	setPeriod(node, mTargetPeriod[node]);
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
		setPeriod(i,mTargetPeriod[i]);
	}
}

void Envelope::trigger() {
	mCurrSegment = 0;
	mPhase = 0;
	mOutput = MIN_ENV_PERIOD;
	mIsDone = false;
}

void Envelope::release() {
	mCurrSegment = mNumSegments - 1;
	mMult[mCurrSegment] = 1 + (log(mPoint[mCurrSegment + 1] + MIN_ENV_PERIOD) - log(mPoint[mCurrSegment] + MIN_ENV_PERIOD)) * mStep[mCurrSegment];
	mPhase = 0;
}

void Envelope::tick() {
	if (mIsDone) {
		return;
	}
	mPhase += mStep[mCurrSegment];
	if (mPhase >= 1) {
		if (mCurrSegment < mNumSegments - 2) {
			mPhase = 0.0;
			mCurrSegment++;
		} else {
			if (mCurrSegment == mNumSegments-1)
				mIsDone = true;
			mPhase = 1.0;
		}
	}else{
		mOutput *= mMult[mCurrSegment];
	}
}