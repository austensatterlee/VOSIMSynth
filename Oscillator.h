#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include <vector>
#include "tables.h"
#include "SourceUnit.h"

//#define USEBLEPS

using namespace std;

enum OSC_MODE
{
  SAW_WAVE = 0,
  SINE_WAVE
} ;

class Oscillator : public SourceUnit {
public:
  Oscillator() :
  SourceUnit()
  {
#ifdef USEBLEPS
    memset(mBlepBuf, 0, BLEPBUFSIZE);
#endif
  };
  int getSamplesPerPeriod() const { return 1. / m_Step; }
  void sync() { m_Phase = 0; };
  void setWaveform(OSC_MODE mode) { m_Waveform = mode; };
  bool isSynced() const { return m_Phase + m_Step >= 1.0; };
  double getPhase() const { return m_Phase; };
  bool isActive() { return getParam("gain")!=0; };
protected:
  double m_Phase = 0;
  double m_Step = 1;
  virtual double process();
private:
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
#endif