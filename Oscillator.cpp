#include "Oscillator.h"

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
#ifdef USE_MINBLEPS
    if ( useMinBleps && nInit ) {
        mBlepBufInd++;
        if ( mBlepBufInd >= BLEPBUF )
            mBlepBufInd = 0;
        nInit--;
    }
#endif
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
		return mGain*2*(0.5-VOSIM_PULSE_COS[(int)(VOSIM_PULSE_COS_SIZE*mPhase)]);
		break;
	default:
		return 0;
		break;
	}	
}

void Oscillator::addBlep( double offset, double ampl ) {
    int i;
	int blepos = BLEPOS / mOversampling;
    int lpIn = blepos*offset;
    int lpOut = mBlepBufInd;
    int cBlep = BLEPBUF - 1;
    double frac = blepos*offset - lpIn;
    double f;
    for ( i = 0; i < nInit; i++,lpIn+= blepos,lpOut++ ) {
        if ( lpOut >= cBlep )
            lpOut = 0;
        f = LERP( MINBLEP[lpIn], MINBLEP[lpIn + 1], frac );
        mBlepBuf[lpOut] += ampl*( 1 - f );
    }
    for ( ; i < cBlep; i++, lpIn += blepos, lpOut++ ) {
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
	mNumber = (4 * mTargetNumber);
	if (mUseRelativeWidth) {
		mPFreq = (mNumber + 32*mTargetPFreq * (1 + mPFreqMod)) * mFreq;
	} else {
		mPFreq = 440 * pow(2.0, 4 * mTargetPFreq  * (1 + mPFreqMod));
	}
	mPFreq = std::fmin(mPFreq, mFs / 2);
	mDecay = std::fmin(0.9999,mTargetDecay*mDecayMod);
    resetMods();
	refreshPhaseScale();
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
    int N, lastN;
    double pulsePhase,lastPulsePhase;
	double innerPhase;
    double vout;
	int wtindex;
	double wtfrac;
	innerPhase = mPhase * mPhaseScale;
	N = innerPhase;
    lastN = mLastInnerPhase;
	if (!N)
		mAttenuation = mGain;
	else if (N != lastN)
		mAttenuation *= mDecay;
    pulsePhase = ( innerPhase - N );
	if (innerPhase >= mNumber) {
#ifdef USE_MINBLEPS
		lastPulsePhase = (mLastInnerPhase - lastN);
        if (useMinBleps && mLastInnerPhase <= mNumber) {
			wtindex = ((VOSIM_PULSE_COS_SIZE-1) * lastPulsePhase);
			wtfrac = ((VOSIM_PULSE_COS_SIZE-1) * lastPulsePhase) - wtindex;
            vout = mAttenuation * LERP(VOSIM_PULSE_COS[wtindex], VOSIM_PULSE_COS[wtindex + 1], wtfrac);
            addBlep( (innerPhase) * mPFreq / (mFs), vout );
        }
#endif
		vout = 0;
    } else {
		wtindex = ((VOSIM_PULSE_COS_SIZE - 1) * pulsePhase);
		wtfrac = ((VOSIM_PULSE_COS_SIZE - 1) * pulsePhase) - wtindex;
        vout = mAttenuation * LERP(VOSIM_PULSE_COS[wtindex],VOSIM_PULSE_COS[wtindex+1],wtfrac);
    }
#ifdef USE_MINBLEPS
    if (useMinBleps && mPhase+mStep >= 1.0 ) {
        addBlep( (mStep+mPhase-1)*mFreq / mFs, vout );
    }
	vout += mBlepBuf[mBlepBufInd];
#endif
	mLastInnerPhase = innerPhase;
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
	 * y[k] = f(kT) = a*b^(kT) + c
	 * y[k+1] = (y[k]-c)*b^T + c
	 * a = shape
	 * b = exp( 1/(N*T) * log(1/a+1) )
	 * c = -a
	 */
	double b;
	mTargetPeriod[segment] = period;
	mShape[segment] = shape;
	period = MIN_ENV_PERIOD + 4*period;
	period = std::floor(period * mFs);
	shape = MIN_ENV_SHAPE + 4*shape;
	mBase[segment] = -shape;
	b = std::exp(std::log(1.0 / shape + 1)*1.0 / (period));
	mMult[segment] = b;
}

void Envelope::setShape(int segment, double shape) {
	if (segment == -1) {
		for (int i = 0; i < mNumSegments; i++) {
			setPeriod(i, mTargetPeriod[i], shape);
		}
	} else {
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
	if (mCurrSegment == mNumSegments - 1) {
		return -(1 - mOutput)*(mPoint[mCurrSegment + 1] - mRelpoint) + mPoint[mCurrSegment + 1];
	} else {
		return -(1 - mOutput)*(mPoint[mCurrSegment + 1] - mPoint[mCurrSegment]) + mPoint[mCurrSegment+1];
	}
}

void Envelope::setFs(double fs) {
	mFs = fs;
	for (int i = 0; i < mNumSegments; i++) {
		setPeriod(i,mTargetPeriod[i],mShape[i]);
	}
}

void Envelope::trigger() {
	mCurrSegment = 0;
	mOutput = 0;
	mGate = 1;
	mIsDone = false;
}

void Envelope::release() {
	mRelpoint = getOutput();
	mOutput = 0; 
	mGate = 0;
	mCurrSegment = mNumSegments - 1;
}

void Envelope::tick() {
	if (mOutput>=1.0) {
		mOutput = 1.0;
		if (mCurrSegment < mNumSegments - 2) {
			mCurrSegment++;
			mOutput = 0;
		} else {
			if (mRepeat && mCurrSegment == mNumSegments - 2) {
				mCurrSegment = 0;
				mOutput = 0;
			}
			if (mCurrSegment == mNumSegments - 1) {
				mIsDone = true;
			}
		}
	}else{
		mOutput = mBase[mCurrSegment]+mMult[mCurrSegment]*(mOutput-mBase[mCurrSegment]);
	}
}