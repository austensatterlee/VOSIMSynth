#ifndef __OSCILLATOR__
#define __OSCILLATOR
#include <cstdint>

#define MIN_ENV_PERIOD 0.001

class Oscillator {
protected:
	double mFs;
	int32_t mPhase;
	int32_t mStep;
	double mTargetFreq;
	double mGain;
	double mTargetGain;
public:
	void tick();
	void setFreq(double freq);
	void setFs(double fs);
	void setGain(double gain);
	double getOutput();
	Oscillator() :
		mFs(44100.0),
		mTargetFreq(1.0),
		mPhase(0),
		mStep(0),
		mTargetGain(0.0),
		mGain(0.0)
	{
			setFreq(mTargetFreq);
	};
};

class VOSIM : public Oscillator {
private:
	void refreshPhaseScale() { mPhaseScale = 2.0 / (mTargetFreq * mWidth); };
	double mDecay;
	double mWidth;
	uint8_t mNumber;
	double mAttenuation;
	uint8_t mLastN;
	double mPhaseScale;
public:
	void setDecay(double decay) { mDecay = decay; };
	void setWidth(double width) { mWidth = width; refreshPhaseScale(); };
	void setNumber(uint8_t number) { mNumber = number; };
	void setFreq(double freq);
	double getOutput();
	VOSIM() :
		Oscillator(),
		mDecay(0.9),
		mWidth(0.01),
		mNumber(5),
		mLastN(0),		
		mAttenuation(1)
	{
		setFreq(mTargetFreq);
	};
};

class Envelope {
private:
	double mFs;
	double mPhase;
	int mNumSegments;
	int mCurrSegment;
	double *mTargetPeriod;
	double *mStep;
	double *mPoint;
	double *mMult;
	void _setPoint(int segment, double gain);
public:
	double mOutput;
	bool mIsDone;
	void	tick();
	void	setPeriod(int segment, double period);
	void	setPoint_dB(int segment, double dB);
	void	setPoint(int segment, double gain);
	void	setFs(double fs);
	void	trigger();
	void	release();
	double	getOutput();
	Envelope(int numsteps = 0) :
		mFs(44100),
		mPhase(0),
		mCurrSegment(0),
		mIsDone(false),
		mOutput(0.0)
	{
		mNumSegments = numsteps+3;
		mTargetPeriod = new double[mNumSegments];
		mPoint = new double[mNumSegments+1];
		mStep = new double[mNumSegments];
		mMult = new double[mNumSegments];
		for (int i = 0; i < mNumSegments; i++) {
			_setPoint(i, 1);
			setPeriod(i, 0);
		}
		_setPoint(0, 0);setPeriod(0, 1);
		_setPoint(1, 1);
		_setPoint(mNumSegments, 0); setPeriod(mNumSegments-1, 1);
	};
};

#endif