#include "Oscillator.h"

extern double VOSIM_PULSE_COS[65536];
extern double MINBLEP[BLEPSIZE];
#define LERP(A,B,F) ((B-A)*F+A)

/******************************
 * Oscillator methods
 *
 ******************************/
void Oscillator::setFreq(double freq) {
	mTargetFreq = freq;
	mStep = (freq / mFs);
}

void Oscillator::applyMods() {
    updateMods();
    resetMods();
}

void Oscillator::updateMods() {
    mFreq = mTargetFreq*( 1 + mFreqMod );
    mStep = ( mFreq / mFs );
    mGain = mTargetGain*mGainMod;
}

void Oscillator::resetMods() {      
	mFreqMod = 0;
	mGainMod = 1;
}

void Oscillator::tick() {
	mPhase += mStep;
	if (mPhase > 1)
		mPhase -= 1;
    if ( nInit ) {
        mBlepBufInd++;
        if ( mBlepBufInd >= BLEPBUF )
            mBlepBufInd = 0;
        nInit--;
    }
}

void Oscillator::setFs(double fs) {
	mFs = fs;
	setFreq(mTargetFreq);
}

double Oscillator::getOutput() {
	switch (mWaveform) {
	case SAW_WAVE:
		return mGain*(mPhase*2-1);
		break;
	case SINE_WAVE:
		return mGain*2*(0.5-VOSIM_PULSE_COS[(uint32_t)(0xFFFF*mPhase)]);
		break;
	default:
		return 0;
		break;
	}	
}

void Oscillator::addBlep( double offset, double ampl ) {
    int i;
    int lpIn = BLEPOS*offset;
    int lpOut = mBlepBufInd;
    int cBlep = BLEPBUF - 1;
    double frac = BLEPOS*offset - lpIn;
    double f;
    for ( i = 0; i < nInit; i++,lpIn+=BLEPOS,lpOut++ ) {
        if ( lpOut >= cBlep )
            lpOut = 0;
        f = LERP( MINBLEP[lpIn], MINBLEP[lpIn + 1], frac );
        mBlepBuf[lpOut] += ampl*( 1 - f );
    }
    for ( ; i < cBlep; i++, lpIn += BLEPOS, lpOut++ ) {
        if ( lpOut >= cBlep )
            lpOut = 0;
        f = LERP( MINBLEP[lpIn], MINBLEP[lpIn + 1], frac );
        mBlepBuf[lpOut] = ampl*( 1 - f );
    }
    nInit = cBlep;
}

/******************************
* VOSIM methods
*
******************************/
void VOSIM::applyMods() {
	Oscillator::updateMods();
    //mNumber = 0.5*mTargetNumber * mPhaseScale;
    mNumber = (16 * mTargetNumber);
	if (mUseRelativeWidth) {
		mPFreq = (mNumber + mTargetPFreq * (1 + mPFreqMod)) * mFreq;
	} else {
		mPFreq = 440 * pow(2.0, 1 + 5 * mTargetPFreq  * (1 + mPFreqMod));
	}
    //mPFreq = std::fmax(mPFreq, mFreq);
	mDecay = std::fmin(0.9999,std::fmax(0,std::fmin(1,((mTargetDecay-1)*mDecayMod)+1)));
    refreshPhaseScale();
    resetMods();
}

void VOSIM::resetMods() {
    Oscillator::resetMods();
    mDecayMod = 1;
    mPFreqMod = 0;
}

void VOSIM::refreshPhaseScale() {
	mPhaseScale = mPFreq / mFreq;
}

double VOSIM::getOutput() {
    static uint32_t N, lastN;
    static double pulsePhase,lastPulsePhase;
	static double innerPhase;
    static double vout;
	innerPhase = mPhase * mPhaseScale;
	N = std::floor(innerPhase);
    lastN = std::floor( mLastInnerPhase );
	if (!N)
		mAttenuation = mGain;
	else if (N != lastN)
		mAttenuation *= mDecay;
    pulsePhase = ( innerPhase - N );
    lastPulsePhase = ( mLastInnerPhase - lastN );
	if (innerPhase >= mNumber) {
        if ( mLastInnerPhase <= mNumber) {
            vout = mAttenuation * VOSIM_PULSE_COS[(int)( 0xFFFF * lastPulsePhase )];
            addBlep( (innerPhase) * mPFreq / (mFs), vout );
        }
		vout = 0;
    } else {   
        vout = mAttenuation * VOSIM_PULSE_COS[(int)( 0xFFFF * pulsePhase )];
    }
    if ( mPhase+mStep >= 1.0 ) {
        addBlep( mPhase*mFreq / mFs, vout );
    }
    mLastInnerPhase = innerPhase;
    return vout + mBlepBuf[mBlepBufInd];
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
	if (segment < mNumSegments - 1) {
		mMult[segment] = exp(-log((abs(mPoint[segment + 1] - mPoint[segment]) + shape) / shape) / period);
		mBase[segment] = (mPoint[segment + 1] - mPoint[segment] + shape)*(1.0 - mMult[segment]);
	}
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
	mMult[mCurrSegment] = exp(-log((abs(mPoint[mCurrSegment + 1] - mOutput)+ mShape[mCurrSegment]) / mShape[mCurrSegment]) / (mTargetPeriod[mCurrSegment]*mFs));
	mBase[mCurrSegment] = (mPoint[mCurrSegment + 1] - mOutput + mShape[mCurrSegment])*(1.0 - mMult[mCurrSegment]);
}

void Envelope::tick() {
	if ((mBase[mCurrSegment]>=0 && mOutput >= mPoint[mCurrSegment + 1]) || 
		(mBase[mCurrSegment]<=0 && mOutput <= mPoint[mCurrSegment + 1])) {
		mOutput = mPoint[mCurrSegment + 1];
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