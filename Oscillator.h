#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include <vector>
#include "tables.h"
#include "SourceUnit.h"

//#define USEBLEPS

using namespace std;

typedef enum OSC_MDOE {
  SAW_WAVE = 0,
  SINE_WAVE
} OSC_MODE;

class Oscillator : public SourceUnit {
public:
  Oscillator() :
  SourceUnit(),
  m_isActive(false)
  {
  addParams({"pitch"});
#ifdef USEBLEPS
    memset(mBlepBuf, 0, BLEPBUFSIZE);
#endif
  };
  int getSamplesPerPeriod() const { return 1. / m_Step; }
  void sync() { m_Phase = 0; };
  void setWaveform(OSC_MODE mode) { m_Waveform = mode; };
  bool isSynced() const { return m_Phase + m_Step >= 1.0; };
  double getPhase() const { return m_Phase; };
  virtual void trigger() { m_isActive = true; };
  virtual void release() { m_isActive = false; };
  bool isActive() const { return m_isActive; };
protected:
  double m_Phase = 0;
  double m_Step = 1;
  bool m_isActive;
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