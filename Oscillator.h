#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include "tables.h"

#define MIN_ENV_PERIOD	0.001
#define MIN_ENV_SHAPE	0.01
//#define USE_MINBLEPS



typedef enum {
	SAW_WAVE=0,
	SINE_WAVE
} OSC_MODE;

class Oscillator {
protected:
	double mFs;
	double mPhase;
	double mStep;
	double mPhaseOffset;
	double mTargetFreq, mFreq, mFreqMod;
	double mTargetGain, mGain, mGainMod;
    double mBlepBuf[BLEPBUF];
	int mOversampling;
    int mBlepBufInd;
    int nInit;
	bool useMinBleps;
    void addBlep( double offset, double ampl );
public:
	void tick();
	void sync() { mPhase = mPhaseOffset; };
	void setFreq(double freq);
	void setGain(double gain) { mTargetGain = mGain = gain;}
	void modFreq(double modAmt) { mFreqMod = (mFreqMod+1)*modAmt; };
	void modGain(double modAmt) { mGainMod *= modAmt; };
	void setPhaseOffset(uint32_t phaseOffset) { mPhaseOffset = phaseOffset; };
	void setOversampling(int oversampling) { mOversampling = oversampling; };
	void applyMods();
	void updateMods();
	void resetMods();
	void setFs(double fs);
	double getOutput();
	OSC_MODE mWaveform;
	Oscillator() :
		mWaveform(SAW_WAVE),
		mFs(44100.0),
		mTargetFreq(1.0),mFreqMod(0),
		mPhase(0),
		mStep(0),
		mTargetGain(0.0),mGainMod(1),
		mGain(0.0),
		mPhaseOffset(0),
		useMinBleps(false),
		mOversampling(1)
	{
			setFreq(mTargetFreq);
			sync();
            mBlepBufInd = 0;
            for ( int i = 0; i < BLEPBUF; i++ ) {
                mBlepBuf[i] = 0;
            }
            nInit = 0;
	};
};

class VOSIM : public Oscillator {
private:
	void refreshPhaseScale();
	double		mTargetDecay,mDecay,mDecayMod;
	double		mTargetPFreq,mPFreq,mPFreqMod;
	double		mNumber;
	double		mTargetNumber;
	double		mAttenuation;
	double		mLastInnerPhase;
	double		mPhaseScale;
	bool		mUseRelativeWidth;
public:
	void setDecay( double decay ) { mTargetDecay = decay; };
	void scalePFreq(double scale) { mTargetPFreq = scale; };
	void useRelativeWidth(bool b) { mUseRelativeWidth = b; };
	void modPFreq(double modAmt) { mPFreqMod = (mPFreqMod+1)*modAmt; };
	void setNumber(double number) { mTargetNumber = number; };
	void applyMods();
	void resetMods();
	void modDecay(double modAmt) { mDecayMod += modAmt; };
	double getOutput();
	VOSIM() :
		Oscillator(),
		mTargetDecay(0.0),mDecay(0.0),mDecayMod(1.0),
		mTargetPFreq(0.01),mPFreq(0.0),mPFreqMod(0.0),
		mNumber(5),
        mLastInnerPhase(0),
		mAttenuation(1),
		mUseRelativeWidth(false)
	{
	};
};

class Envelope {
private:
	double mFs;
	int mNumSegments;
	int mCurrSegment;
	int mGate;
	bool mRepeat;
	double *mTargetPeriod;
	double *mPoint;
	double *mShape;
	double *mBase;
	double *mMult;
	double mRelpoint;
	void _setPoint(int segment, double gain);
public:
	double	mOutput;
	bool	mIsDone;
	void	tick();
	void	setPeriod(int segment, double period, double shape);
	void	setShape(int segment, double shape);
	void	setPoint_dB(int segment, double dB);
	void	setPoint(int segment, double gain);
	void	setFs(double fs);
	void	trigger();
	void	release();
	double	getOutput();
	Envelope(int numsteps = 0) :
		mFs(44100),
		mCurrSegment(0),
		mIsDone(true),
		mOutput(0.0),
		mRepeat(false),
		mGate(0)
	{
		mNumSegments = numsteps+3;
		mTargetPeriod = new double[mNumSegments];
		mPoint = new double[mNumSegments+1];
		mShape = new double[mNumSegments];
		mBase = new double[mNumSegments];
		mMult = new double[mNumSegments];
		for (int i = 0; i < mNumSegments; i++) {
			mPoint[i] = 1;
		}
		for (int i = 0; i < mNumSegments; i++) {
			setPeriod(i, MIN_ENV_PERIOD, MIN_ENV_SHAPE);
		}
		_setPoint(0, 0);setPeriod(0, 1, MIN_ENV_SHAPE);
		_setPoint(1, 1);
		_setPoint(mNumSegments, 0); setPeriod(mNumSegments-1, 1, MIN_ENV_SHAPE);
	};
};

#endif