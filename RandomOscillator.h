#pragma once
#include "Oscillator.h"
#include "UnitParameter.h"
#include <random>
#include <string>
using namespace std;
namespace syn
{
  static default_random_engine generator;
  static normal_distribution<double> norm_generator(0,1.0);
  class RandomOscillator :
    public Oscillator
  {
  public:
    RandomOscillator(string name) :
      Oscillator(name)
    {}

    virtual ~RandomOscillator()
    {}
  };

  class NormalRandomOscillator : public RandomOscillator
  {
    public:
    NormalRandomOscillator(string name) : RandomOscillator(name),
    m_curr(0.0),
    m_next(0.0)
    {}
    NormalRandomOscillator(const NormalRandomOscillator& other) : NormalRandomOscillator(other.m_name)
    {
      m_curr = other.m_curr;
      m_next = other.m_next;
    }
    virtual ~NormalRandomOscillator(){}
    double m_curr,m_next;
    protected:
      void process(int bufind)
      {
        Oscillator::tick_phase();
        if (m_isSynced)
        {
          m_curr = m_next;
          m_next = norm_generator(generator);
        }
        m_output[bufind] = LERP(m_curr,m_next,m_phase);
      }
    private:
    virtual Unit* cloneImpl() const {return new NormalRandomOscillator(*this); }
  };
};