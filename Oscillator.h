#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include <vector>
#include "tables.h"
#include "SourceUnit.h"

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
      m_velocity(1.0),
      m_gain(addParam("gain", DOUBLE_TYPE, 0, 1)),
      m_pitch(addParam("pitch", DOUBLE_TYPE, 0, 128, true)),
      m_pitchshift(addParam("semitones", DOUBLE_TYPE, -12, 12))
    {};
    Oscillator(const Oscillator& osc);
    virtual int getSamplesPerPeriod() const { return 1. / m_Step; }
    void sync() { m_Phase = 0; };
    void setWaveform(OSC_MODE mode) { m_Waveform = mode; };
    bool isSynced() const { return m_Phase + m_Step >= 1.0; };
    virtual bool isActive() const { return readParam(0) != 0; };
    double getPhase() const { return m_Phase; };
    virtual void noteOn(int pitch, int vel);
    virtual void noteOff(int pitch, int vel);
  protected:
    double m_Phase = 0;
    double m_Step = 1;
    double m_velocity;
    UnitParameter& m_gain;
    UnitParameter& m_pitch;
    UnitParameter& m_pitchshift;
    virtual double process();
    virtual void tick_phase();
  private:
    OSC_MODE m_Waveform = SINE_WAVE;
    virtual Unit* cloneImpl() const { return new Oscillator(*this); };
  };
}
#endif