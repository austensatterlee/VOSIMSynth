#pragma once
#include "Oscillator.h"
#include <random>
#include "DSPMath.h"
using namespace std;
namespace syn
{
  static int NUMRANDOMOSCILLATORS = 0;
  class RandomOscillator :
    public LFOOscillator
  {
  protected:
    random_device m_rd;
    mt19937 m_gen;
  public:
    RandomOscillator(string name) :
      LFOOscillator(name)
    {
      unsigned int seed = m_rd();
      m_gen = mt19937(seed);
    }

    virtual ~RandomOscillator()
    {}
  };

  class NormalRandomOscillator : public RandomOscillator
  {
  public:
    NormalRandomOscillator(string name) : RandomOscillator(name),
      m_curr(0.0),
      m_next(0.0),
      m_rand_period(1.0),
      m_uni_gen(0.0, 1.0),
      m_exp_gen(1.0),
      m_curr_polarity(1)
    {}
    NormalRandomOscillator(const NormalRandomOscillator& other) : NormalRandomOscillator(other.m_name)
    {
      m_curr = other.m_curr;
      m_next = other.m_next;
      m_rand_period = other.m_rand_period;
    }
    virtual ~NormalRandomOscillator() {}
  protected:
    double m_curr, m_next;
    double m_rand_period;
    double m_curr_polarity;
    exponential_distribution<double> m_exp_gen;
    uniform_real_distribution<double> m_uni_gen;
    virtual void update_step() override
    {
      double freq = pitchToFreq(m_pitch + m_tune);
      m_exp_gen = exponential_distribution<double>{ freq };
      m_Step = freq / (m_Fs);
    }
    virtual void process(int bufind) override
    {
      tick_phase();
      if (m_isSynced)
      {
        m_curr = m_next;
        m_curr_polarity *= -1;
        m_next = m_curr_polarity*m_gain*m_uni_gen(m_gen);
        m_rand_period = m_exp_gen(m_gen);
      }
      m_output[bufind] = LERP(m_curr, m_next, m_phase);
    }
  private:
    virtual Unit* cloneImpl() const override
    { return new NormalRandomOscillator(*this); };
  };
};