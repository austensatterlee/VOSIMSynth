#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include "tables.h"

#define MIN_ENV_PERIOD	.0001
#define MIN_ENV_SHAPE	.000000001
//#define USEBLEPS

#define LERP(A,B,F) (((B)-(A))*(F)+(A))
inline double pitchToFreq(double pitch) { return 440.0 * pow(2.0, (pitch - 69.0) / 12.0); }
inline double dbToAmp(double db) { return pow(10, 0.05*db); }
inline double ampToDb(double amp) { return 20 * log10(amp); }

using namespace std;

typedef enum OSC_MDOE {
  SAW_WAVE = 0,
  SINE_WAVE
} OSC_MODE;

template <typename T>
class Modifiable {
protected:
public:
  T base, offset, scale;
  T curr;
  Modifiable() :base(0), offset(0), curr(0), scale(1.0) {};
  void set(T val) {
    base = val;
  };
  void mod(T val) {
    offset += val;
  };
  void scale(T val) {
    scale *= val;
  }
  const T& operator()(void) {
    curr = scale*(base + offset);
    offset = 0.0;
    scale = 1.0;
    return curr;
  };
};

class Oscillator {
protected:
  double mFs = 44100;
  double mPhase = 0;

  OSC_MODE mWaveform = SINE_WAVE;

  /* Blep state */
#ifdef USEBLEPS
  bool useMinBleps;
  double mBlepBuf[BLEPBUFSIZE];
  int mBlepInd = 0;
  int mBlepCurrSize = 0;
  void addBlep(double offset, double ampl);
#endif
public:
  void update();
  void tick();
  void sync(){mPhase=0;}
  void setFs(double fs);
  void setWaveform(OSC_MODE mode) { mWaveform = mode; };
  double getOutput();

  Modifiable<double> mpPitch;
  Modifiable<double> mpGain;
  double mLastOutput = 0.0;
  double mStep = 0;

  Oscillator() {
    mpPitch.set(1.0);
    mpGain.set(1.0);
#ifdef USEBLEPS
    memset(mBlepBuf, 0, BLEPBUFSIZE);
#endif
  };
};

class VOSIM : public Oscillator {
private:
  /* internal state */
  double		mCurrPulseGain = 1;
  double		mLastPulsePhase = 0;
  double		mPhaseScale = 1;
  bool		mUseRelativeWidth;
  double    mMaxAmp = 1.0;
public:
  Modifiable<double>	mpDecay;
  Modifiable<double>	mpPulsePitch;
  Modifiable<double>	mpNumber;
  void update();
  void toggleRelativePFreq(bool b) { mUseRelativeWidth = b; };
  double getOutput();
  VOSIM() :
    Oscillator(),
    mUseRelativeWidth(true) {
    mpDecay.set(1);
    mpPulsePitch.set(1);
    mpNumber.set(1);
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
    mGate(0) {
    mNumSegments = numsteps + 3;
    mTargetPeriod = new double[mNumSegments];
    mPoint = new double[mNumSegments + 1];
    mShape = new double[mNumSegments];
    mBase = new double[mNumSegments];
    mMult = new double[mNumSegments];
    for (int i = 0; i < mNumSegments; i++) {
      mPoint[i] = 1;
    }
    for (int i = 0; i < mNumSegments; i++) {
      setPeriod(i, MIN_ENV_PERIOD, MIN_ENV_SHAPE);
    }
    _setPoint(0, 0); setPeriod(0, 1, MIN_ENV_SHAPE);
    _setPoint(1, 1);
    _setPoint(mNumSegments, 0); setPeriod(mNumSegments - 1, 1, MIN_ENV_SHAPE);
  };
};

#endif