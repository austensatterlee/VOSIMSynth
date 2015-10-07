#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include <vector>
#include "tables.h"
#include "DSPComponent.h"
#include "GallantSignal.h"

#define MIN_ENV_PERIOD	.0001
#define MIN_ENV_SHAPE	.000000000001
//#define USEBLEPS

using namespace std;



typedef enum OSC_MDOE {
  SAW_WAVE = 0,
  SINE_WAVE
} OSC_MODE;

class Oscillator : public DSPComponent<double> {
public:
  Oscillator() :
    DSPComponent() 
  {
    m_pPitch.set(1.0);
#ifdef USEBLEPS
    memset(mBlepBuf, 0, BLEPBUFSIZE);
#endif
  };
  virtual double process(const double input = 0);
  virtual void updateParams();
  int getSamplesPerPeriod() const { return 1. / m_Step; }
  void sync() { m_Phase = 0; };
  void setWaveform(OSC_MODE mode) { m_Waveform = mode; };
  bool isSynced() const { return m_Phase + m_Step >= 1.0; };
  double getPhase() const { return m_Phase; };
  Modifiable<double> m_pPitch;
protected:
  virtual void tick();
  double m_Phase = 0;
  double m_Step = 1;
  OSC_MODE m_Waveform = SINE_WAVE;

  /* Blep state */
#ifdef USEBLEPS
  bool useMinBleps;
  double mBlepBuf[BLEPBUFSIZE];
  int mBlepInd = 0;
  int mBlepCurrSize = 0;
  void addBlep(double offset, double ampl);
#endif
};

class VOSIM : public Oscillator {
private:
  /* internal state */
  double		m_CurrPulseGain = 1;
  double		m_LastPulsePhase = 0;
  bool		  m_UseRelativeWidth;
  double    m_CompensationGain = 1.0;
public:
  Modifiable<double>	mpDecay;
  Modifiable<double>	mpPulsePitch;
  Modifiable<double>	mpNumber;
  virtual double process(const double input = 0);
  virtual void updateParams();
  double mPhaseScale = 1;
  void toggleRelativePFreq(bool b) { m_UseRelativeWidth = b; };
  VOSIM() :
    Oscillator(),
    m_UseRelativeWidth(true) {
    mpDecay.set(1);
    mpPulsePitch.set(1);
    mpNumber.set(1);
  };
};

struct EnvelopeSegment {
  double period;
  double target_amp;
  double shape;
  double base;
  double mult;
};

class Envelope : public DSPComponent<double> {
private:
  vector<EnvelopeSegment> m_segments;
  double m_initPoint;
  int m_numSegments;
  int m_currSegment;
  bool m_isRepeating;
  bool	m_isDone;
  double m_RelPoint;
  double m_lastRawOutput;
public:
  virtual void setFs(const double fs);
  const bool isDone() { return m_isDone; };
  void	setPeriod(const int segment, double period, double shape=0);
  void	setShape(int segment, double shape);
  void	setPoint(int segment, double target_amp);
  void	trigger();
  void	release();
  int   getSamplesPerPeriod() const;
  double process(const double input);
  Envelope() :
    DSPComponent(),
    m_currSegment(0),
    m_isDone(true),
    m_isRepeating(false),
    m_lastRawOutput(0),
    m_initPoint(0),
    m_numSegments(3) {
    m_segments = vector<EnvelopeSegment>(m_numSegments);
    // set up a standard ADSR envelope
    m_initPoint = 0.0;
    m_segments[0].target_amp = 1.0;
    m_segments[1].target_amp = 1.0;
    m_segments[2].target_amp = 0.0;
    for (int i = 0; i < m_numSegments; i++) {
      setPeriod(i, MIN_ENV_PERIOD, MIN_ENV_SHAPE);
    }
  };
};

#endif