#ifndef __VosimOscillator__
#define __VosimOscillator__

#include "Oscillator.h"
#include "SourceUnit.h"
#include "RandomOscillator.h"
#include <string>
#include <random>
#include <cmath>

using namespace std;

namespace syn
{
  class VosimOscillator : public Oscillator
  {
  public:
    VosimOscillator(string name) :
      Oscillator(name),
      m_decay(addParam("decay", DOUBLE_TYPE, 0, 0.9)),
      m_ppitch(addParam("pulsepitch", DOUBLE_TYPE, 0, 1)),
      m_number(addParam("number", INT_TYPE, 0, 4)),
      m_relativeamt(addParam("relative", DOUBLE_TYPE, 0, 1, true)),
      m_curr_pulse_gain(1.0),
      m_pulse_step(0.0),
      m_pulse_phase(0.0),
      m_unwrapped_pulse_phase(0.0)
    {};
    VosimOscillator(const VosimOscillator& vosc);
    virtual void process(int bufind);
    virtual void sync();
    UnitParameter& m_relativeamt;
    UnitParameter& m_decay;
    UnitParameter& m_ppitch;
    UnitParameter& m_number;
  private:
    /* internal state */
    virtual Unit* cloneImpl() const { return new VosimOscillator(*this); };
    double		m_curr_pulse_gain;
    double    m_pulse_step;
    double    m_pulse_phase;
    double		m_last_pulse_phase;
    double    m_unwrapped_pulse_phase;
  };

  class VosimChoir : public SourceUnit
  {
  public:
    VosimChoir(string name, size_t size=4) :
      SourceUnit(name),
      m_size(size),
      m_gain(addParam("gain", DOUBLE_TYPE, 0, 1)),
      m_decay(addParam("decay", DOUBLE_TYPE, 0, 0.9)),
      m_ppitch(addParam("pulsepitch", DOUBLE_TYPE, 0, 1)),
      m_number(addParam("number", INT_TYPE, 0, 4)),
      m_harmonicdecay(addParam("harmonicdecay", DOUBLE_TYPE, 0, 0.9)),
      m_tune(addParam("tune", DOUBLE_TYPE, -12, 12)),
      m_pitchdrift(addParam("pulse_drift", DOUBLE_TYPE, 0, 0.5)),
      m_driftfreq(addParam("drift_freq", DOUBLE_TYPE, -32, 32))
    {
      assert(size>0);
      m_choir = (VosimOscillator**)malloc(m_size*sizeof(VosimOscillator*));
      m_pulsedrifters = (NormalRandomOscillator**)malloc(m_size*sizeof(NormalRandomOscillator*));
      for (int i = 0; i < m_size; i++)
      {
        m_choir[i] = new VosimOscillator(name);
        m_pulsedrifters[i] = new NormalRandomOscillator(name);
        m_choir[i]->getParam("tune").addConnection(&m_pulsedrifters[i]->getLastOutputBuffer(),ADD);
      }
    }
    VosimChoir(const VosimChoir& other) : VosimChoir(other.m_name, other.m_size)
    {}
    virtual ~VosimChoir()
    {
      for (int i = 0; i < m_size; i++)
      {
        delete m_choir[i];
        delete m_pulsedrifters[i];
      }
      delete m_choir;
      delete m_pulsedrifters;
    }
    //virtual void resizeOutputBuffer(size_t size)
    //{
    //  Unit::resizeOutputBuffer(size);
    //  for (int i = 0; i < m_size; i++)
    //  {
    //    m_choir[i]->resizeOutputBuffer(size);
    //    m_pulsedrifters[i]->resizeOutputBuffer(size);
    //  }
    //}
    bool isActive() const { return m_gain != 0; };
    virtual void noteOn(int pitch, int vel)
    {
      for (int i = 0; i < m_size; i++)
      {
        m_choir[i]->noteOn(pitch,vel);
      }
    }
    virtual void noteOff(int pitch, int vel)
    {
      for (int i = 0; i < m_size; i++)
      {
        m_choir[i]->noteOff(pitch, vel);
      }
    }
    virtual void setFs(double fs)
    {
      for (int i = 0; i < m_size; i++)
      {
        m_choir[i]->setFs(fs);
      }
    }
    virtual int getSamplesPerPeriod() const { return m_choir[0]->getSamplesPerPeriod(); }   
    UnitParameter& m_gain;
    UnitParameter& m_decay;
    UnitParameter& m_harmonicdecay;
    UnitParameter& m_ppitch;
    UnitParameter& m_number;
    UnitParameter& m_tune;
    UnitParameter& m_pitchdrift;
    UnitParameter& m_driftfreq;
  protected:
    virtual void process(int bufind)
    {
      m_output[bufind] = 0;
      double harmonicgain = 1.0;
      for (int i = 0; i < m_size; i++)
      {
        m_choir[i]->m_decay.mod(m_decay, SET);

        m_choir[i]->m_ppitch.mod(m_ppitch*(1+(double)i/m_size), SET);

        m_choir[i]->m_number.mod(m_number+i, SET);

        m_pulsedrifters[i]->m_pitch.mod(m_driftfreq,SET);
        m_pulsedrifters[i]->m_gain.mod(m_pitchdrift, SET);
        m_choir[i]->m_tune.mod(m_tune, SET);

        m_pulsedrifters[i]->tick();
        m_choir[i]->tick();

        if (i & 0x01)
        {
          m_output[bufind] += harmonicgain*m_choir[i]->getLastOutput();
        }
        else
        {
          m_output[bufind] -= harmonicgain*m_choir[i]->getLastOutput();
        }
        harmonicgain*=m_harmonicdecay;
      }
      m_output[bufind] *= m_gain;
    }
  private:
    VosimOscillator** m_choir;
    NormalRandomOscillator** m_pulsedrifters;
    size_t m_size;
    virtual Unit* cloneImpl() const
    {
      return new VosimChoir(*this);
    }
  };
}

#endif // __VosimOscillator__