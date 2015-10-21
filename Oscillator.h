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
    Oscillator(string name) : SourceUnit(name), 
    m_velocity(1.0)
    {
      addParam(UnitParameter("gain", 1, 0.0, 1.0, DOUBLE_TYPE, false));
      addParam(UnitParameter("pitch", 0, -128.0, 128.0, DOUBLE_TYPE));
      addParam(UnitParameter("semitones", 0, -12, 12, DOUBLE_TYPE, false));
#ifdef USEBLEPS
      memset(mBlepBuf, 0, BLEPBUFSIZE);
#endif
    };
    Oscillator(const Oscillator& osc);
    virtual int getSamplesPerPeriod() const { return 1. / m_Step; }
    void sync() { m_Phase = 0; };
    void setWaveform(OSC_MODE mode) { m_Waveform = mode; };
    bool isSynced() const { return m_Phase + m_Step >= 1.0; };
    virtual bool isActive() const { return readParam(0)!=0; };
    double getPhase() const { return m_Phase; };
    virtual void noteOn(int pitch, int vel);
    virtual void noteOff(int pitch, int vel);
  protected:
    double m_Phase = 0;
    double m_Step = 1;
    virtual double process();
    virtual void tick_phase();
  private:
    double m_velocity;
    OSC_MODE m_Waveform = SINE_WAVE;
    virtual Unit* cloneImpl() const { return new Oscillator(*this); };
    virtual double finishProcessing(double o){ return m_velocity*readParam(0)*o; }

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