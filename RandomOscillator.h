#pragma once
#include "Oscillator.h"
#include <random>
#include "DSPMath.h"
using namespace std;
namespace syn
{
  static int NUMRANDOMOSCILLATORS = 0;
  class RandomOscillator :
    public Oscillator
  {
  protected:
    random_device m_rd;
  public:
    RandomOscillator(string name) :
      Oscillator(name)
    {
    }

    virtual ~RandomOscillator()
    {}


    virtual void noteOn(int pitch, int vel) override {};
  };

  class UniformRandomOscillator : public RandomOscillator
  {
  public:
    UniformRandomOscillator(string name) : RandomOscillator(name),
      m_next(m_rd()),
      m_curr(0)
    {
    m_pitch.setIsHidden(false);
    m_pitch.setMin(-64);
    m_pitch.setMax(128);    
    }
    UniformRandomOscillator(const UniformRandomOscillator& other) : UniformRandomOscillator(other.m_name)
    {
      m_curr = other.m_curr;
    }
    virtual ~UniformRandomOscillator() {}
  protected:
    uint32_t m_curr,m_next;
    virtual void update_step() override
    {
      double freq = pitchToFreq(m_pitch + m_finetune);
      m_Step = freq / (m_Fs);
    }
    virtual void process(int bufind) override
    {
      tick_phase();
      if (m_isSynced)
      {
        m_curr = m_next;
        m_next = 69069*m_next+1;
      }
      m_output[bufind] = m_gain*(LERP((m_curr / double(0x7FFFFFFF)), (m_next/double(0x7FFFFFFF)), m_phase)-1.0);
    }
  private:
    virtual Unit* cloneImpl() const override
    { return new UniformRandomOscillator(*this); };
    virtual string getClassName() const override { return "UniformRandomOscillator"; };
  };
};