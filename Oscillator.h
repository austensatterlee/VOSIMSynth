#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include <vector>
#include "tables.h"
#include "SourceUnit.h"

//#define USEBLEPS

using namespace std;
namespace syn
{
  enum OSC_MODE
  {
    SAW_WAVE = 0,
    SINE_WAVE
  };

  class Oscillator : public SourceUnit
  {
  public:
    Oscillator() : SourceUnit()
    {
      addParam(new UnitParameter("pitch"));
#ifdef USEBLEPS
      memset(mBlepBuf, 0, BLEPBUFSIZE);
#endif
    };
    virtual int getSamplesPerPeriod() const { return 1. / m_Step; }
    void sync() { m_Phase = 0; };
    void setWaveform(OSC_MODE mode) { m_Waveform = mode; };
    bool isSynced() const { return m_Phase + m_Step >= 1.0; };
    virtual bool isActive() const { return getParam("gain")!=0; };
    double getPhase() const { return m_Phase; };
    virtual void noteOn(int pitch, int vel);
    virtual void noteOff(int pitch, int vel);
    virtual Unit* cloneImpl() const { return new Oscillator(); };
  protected:
    double m_Phase = 0;
    double m_Step = 1;
    virtual double process();
    virtual void tick_phase();
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
}
#endif