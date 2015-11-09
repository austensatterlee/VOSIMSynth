#ifndef __VosimOscillator__
#define __VosimOscillator__

#include "Oscillator.h"

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
    void tick_pulse_phase();
    virtual void process();
    virtual void sync();
  private:
    /* internal state */
    virtual Unit* cloneImpl() const { return new VosimOscillator(*this); };
    double		m_curr_pulse_gain;
    double    m_pulse_step;
    double    m_pulse_phase;
    double		m_last_pulse_phase;
    double    m_unwrapped_pulse_phase;
    UnitParameter& m_relativeamt;
    UnitParameter& m_decay;
    UnitParameter& m_ppitch;
    UnitParameter& m_number;
  };
}

#endif // __VosimOscillator__