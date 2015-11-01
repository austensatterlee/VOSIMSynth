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
    SINE_WAVE,
    TRI_WAVE,
    SQUARE_WAVE,
    NUM_OSC_MODES
  };

  class Oscillator : public SourceUnit
  {
  public:
    Oscillator(string name) : SourceUnit(name),
      m_velocity(1.0),
      m_gain(addParam("gain", DOUBLE_TYPE, 0, 1)),
      m_pitch(addParam("pitch", DOUBLE_TYPE, 0, 128, true)),
      m_pitchshift(addParam("pitchshift", DOUBLE_TYPE, -1, 1))
    {};
    Oscillator(const Oscillator& osc) :
      Oscillator(osc.m_name)
    {
      m_Phase = osc.m_Phase;
      m_Step = osc.m_Step;
      m_velocity = osc.m_velocity;
      m_maxPitchShift = osc.m_maxPitchShift;
    }
    virtual int getSamplesPerPeriod() const { return 1. / m_Step; }
    void sync() { m_Phase = 0; };
    bool isSynced() const { return m_Phase + m_Step >= 1.0; };
    virtual bool isActive() const { return readParam(0) != 0; };
    double getPhase() const { return m_Phase; };
    virtual void noteOn(int pitch, int vel);
    virtual void noteOff(int pitch, int vel);
  protected:
    double m_Phase = 0;
    double m_Step = 1;
    double m_velocity;
    double m_maxPitchShift = 12;
    UnitParameter& m_gain;
    UnitParameter& m_pitch;
    UnitParameter& m_pitchshift;
    virtual double process() = 0;
    virtual void tick_phase();
  };

  class BasicOscillator : public Oscillator
  {
    public:
    BasicOscillator(string name) : Oscillator(name),
      m_waveform(addParam("waveform", INT_TYPE, 0, NUM_OSC_MODES))
    {};
    BasicOscillator(const BasicOscillator& other) : BasicOscillator(other.m_name)
    {};
  protected:
    UnitParameter& m_waveform;
    virtual double process();
  private:
    virtual Unit* cloneImpl() const { return new BasicOscillator(*this); };
  };

  class LFOOscillator : public BasicOscillator
  {
  public:
    LFOOscillator(string name) : BasicOscillator(name)
    {
      m_pitch.mod(-12, SET); 
      m_maxPitchShift = 36;
    }

    LFOOscillator(const LFOOscillator& other) : LFOOscillator(other.m_name)
    {};
    virtual void noteOn(int pitch, int vel)
    {
      sync();
    }
  private:
    virtual Unit* cloneImpl() const { return new LFOOscillator(*this); };
  };
};
#endif