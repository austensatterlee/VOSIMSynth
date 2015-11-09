#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include <vector>
#include "tables.h"
#include "SourceUnit.h"
#include "DSPMath.h"

using namespace std;
namespace syn
{
  enum OSC_MODE
  {
    SAW_WAVE = 0,
    NAIVE_SAW_WAVE,
    SINE_WAVE,
    TRI_WAVE,
    SQUARE_WAVE,
    NAIVE_SQUARE_WAVE,
    NUM_OSC_MODES
  };

  const vector<string> OSC_MODE_NAMES{"Saw","Naive saw","Sine","Tri","Square","Naive square"};

  class Oscillator : public SourceUnit
  {
  public:
    Oscillator(string name) : SourceUnit(name),
      m_velocity(1.0),
      m_gain(addParam("gain", DOUBLE_TYPE, 0, 1)),
      m_pitch(addParam("pitch", DOUBLE_TYPE, 0, 128, true)),
      m_pitchshift(addParam("pitchshift", DOUBLE_TYPE, -1, 1)),
      m_phaseshift(addParam("phaseshift", DOUBLE_TYPE, -0.5, 0.5)),
      m_portamento(addParam("portamento", DOUBLE_TYPE, 0, 1, true))
    {
      m_portamento.mod(0,SET);
      m_Step = 440./m_Fs;
      m_targetStep = m_Step;
    };
    Oscillator(const Oscillator& osc) :
      Oscillator(osc.m_name)
    {
      m_basePhase = osc.m_basePhase;
      m_Step = osc.m_Step;
      m_velocity = osc.m_velocity;
      m_maxPitchShift = osc.m_maxPitchShift;
      m_phase = osc.m_phase;
    }
    int getSamplesPerPeriod() const { return 1. / m_Step; }
    virtual void sync() { m_basePhase = 0; };
    bool isActive() const { return readParam(0) != 0; };
    double getPhase() const { return m_phase; };
    virtual void noteOn(int pitch, int vel);
    virtual void noteOff(int pitch, int vel) {};
    virtual void setFs(double fs);
  protected:
    double m_basePhase = 0;
    double m_phase = 0;
    double m_Step;
    double m_targetStep;
    double m_velocity;
    double m_maxPitchShift = 12;
    UnitParameter& m_portamento;
    UnitParameter& m_gain;
    UnitParameter& m_pitch;
    UnitParameter& m_pitchshift;
    UnitParameter& m_phaseshift;
    void updateSyncStatus() { m_isSynced = m_phase < m_Step; };
    virtual void process() = 0;
    virtual void tick_phase();
  };

  class BasicOscillator : public Oscillator
  {
    public:
    BasicOscillator(string name) : Oscillator(name),
      m_waveform(addEnumParam("waveform", OSC_MODE_NAMES))
    {};
    BasicOscillator(const BasicOscillator& other) : BasicOscillator(other.m_name)
    {};
  protected:
    UnitParameter& m_waveform;
    virtual void process();
  private:
    virtual Unit* cloneImpl() const { return new BasicOscillator(*this); };
  };

  class LFOOscillator : public BasicOscillator
  {
  public:
    LFOOscillator(string name) : BasicOscillator(name)
    {
      m_pitch.mod(-12, SET); 
      m_maxPitchShift = 24;
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