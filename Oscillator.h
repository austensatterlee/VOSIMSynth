#ifndef __OSCILLATOR__
#define __OSCILLATOR__
#include <cstdint>
#include <cmath>
#include <vector>
#include "SourceUnit.h"

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
                              m_gain(addParam("gain", DOUBLE_TYPE, -1, 1, 0.5)),
                              m_pitch(addParam("pitch", DOUBLE_TYPE, 0, 128, 0, true)),
                              m_finetune(addParam("tune", DOUBLE_TYPE, -12, 12, 0)),
                              m_phaseshift(addParam("phaseshift", DOUBLE_TYPE, -0.5, 0.5, 0.0)),
                              m_velocity(1.0)
    {
      m_Step = 440. / m_Fs;
      m_Step = m_Step;
    };

    Oscillator(const Oscillator& osc) :
      Oscillator(osc.m_name)
    {
      m_basePhase = osc.m_basePhase;
      m_Step = osc.m_Step;
      m_velocity = osc.m_velocity;
      m_phase = osc.m_phase;
    }

    virtual int getSamplesPerPeriod() const override
    {
      return (int)(floor(1. / m_Step));
    }

    virtual void sync()
    {
      m_basePhase = 0;
    };

    virtual bool isActive() const override
    {
      return m_gain != 0;
    };

    virtual void noteOn(int pitch, int vel) override;

    virtual void noteOff(int pitch, int vel) override
    {
    };

    virtual void setFs(double fs) override;
    UnitParameter& m_gain;
    UnitParameter& m_pitch;
    UnitParameter& m_finetune;
    UnitParameter& m_phaseshift;
  protected:
    double m_basePhase = 0;
    double m_phase = 0;
    double m_Step;
    double m_velocity;

    void updateSyncStatus()
    {
      m_isSynced = m_phase < m_Step;
    };

    virtual void tick_phase();
    virtual void update_step();
  };

  class BasicOscillator : public Oscillator
  {
  public:
    BasicOscillator(string name) : Oscillator(name),
                                   m_waveform(addEnumParam("waveform", OSC_MODE_NAMES))
    {
    };

    BasicOscillator(const BasicOscillator& other) : BasicOscillator(other.m_name)
    {
    };

    UnitParameter& m_waveform;
  protected:
    virtual void process(int bufind) override;
  private:
    virtual Unit* cloneImpl() const override
    {
      return new BasicOscillator(*this);
    };

    virtual string getClassName() const override
    {
      return "BasicOscillator";
    };
  };

  class LFOOscillator : public BasicOscillator
  {
  public:
    LFOOscillator(string name) : BasicOscillator(name),
    m_reset(addParam("Reset",BOOL_TYPE,0,1,0))
    {
      m_pitch.setDefault(-12);
      m_pitch.setIsHidden(false);
      m_pitch.setMin(-64);
      m_pitch.setMax(16);
      m_finetune.setMax(24);
      m_finetune.setMin(-24);
    }

    LFOOscillator(const LFOOscillator& other) : LFOOscillator(other.m_name)
    {
    };

    virtual void noteOn(int pitch, int vel) override
    {
      if(m_reset) 
        sync();
    }

    UnitParameter& m_reset;
  private:
    virtual Unit* cloneImpl() const override
    {
      return new LFOOscillator(*this);
    };

    virtual string getClassName() const override
    {
      return "LFOOscillator";
    };
  };
};
#endif

