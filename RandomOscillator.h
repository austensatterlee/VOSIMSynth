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
    mt19937 m_gen;
  public:
    RandomOscillator(string name) :
      Oscillator(name)
    {
      unsigned int seed = m_rd();
      m_gen = mt19937(seed);
    }

    virtual ~RandomOscillator()
    {}


    virtual void noteOn(int pitch, int vel) override {};
  };

  class UniformRandomOscillator : public RandomOscillator
  {
  public:
    UniformRandomOscillator(string name) : RandomOscillator(name),
      m_curr(0.0),
      m_next(0.0),
      m_rand_period(1.0),
      m_exp_gen(1.0),
      m_uni_gen(0.0, 1.0)
    {
    m_pitch.setIsHidden(false);
    m_pitch.setMin(-64);
    m_pitch.setMax(128);
    }
    UniformRandomOscillator(const UniformRandomOscillator& other) : UniformRandomOscillator(other.m_name)
    {
      m_curr = other.m_curr;
      m_next = other.m_next;
      m_rand_period = other.m_rand_period;
    }
    virtual ~UniformRandomOscillator() {}
  protected:
    double m_curr, m_next;
    double m_rand_period;
    exponential_distribution<double> m_exp_gen;
    uniform_real_distribution<double> m_uni_gen;
    virtual void update_step() override
    {
      double freq = pitchToFreq(m_pitch + m_finetune);
      m_exp_gen = exponential_distribution<double>{ freq };
      m_Step = freq / (m_Fs);
    }
    virtual void process(int bufind) override
    {
      tick_phase();
      if (m_isSynced)
      {
        m_curr = m_next;
        m_next = m_gain*m_uni_gen(m_gen);
        m_rand_period = m_exp_gen(m_gen);
      }
      m_output[bufind] = LERP(m_curr, m_next, m_phase)*lut_sin.getlinear(m_phase);
    }
  private:
    virtual Unit* cloneImpl() const override
    { return new UniformRandomOscillator(*this); };
    virtual string getClassName() const override { return "UniformRandomOscillator"; };
  };
};