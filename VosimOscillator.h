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
      m_decay(addParam("decay", DOUBLE_TYPE, 0, 1)),
      m_ppitch(addParam("pulsepitch", DOUBLE_TYPE, 0, 1)),
      m_number(addParam("number", DOUBLE_TYPE, 0, 1)),
      m_relativeamt(addParam("relative", DOUBLE_TYPE, 0, 1))
    {};
    VosimOscillator(const VosimOscillator& vosc);
    double    m_PhaseScale = 1;
    double  getPulsePhase() { return m_basePhase*m_PhaseScale; };
    virtual double process();
  private:
    /* internal state */
    virtual Unit* cloneImpl() const { return new VosimOscillator(*this); };
    double		m_CurrPulseGain = 1;
    double		m_LastPulsePhase = 0;
    double    m_CompensationGain = 1.0;
    UnitParameter& m_relativeamt;
    UnitParameter& m_decay;
    UnitParameter& m_ppitch;
    UnitParameter& m_number;
  };
}

#endif // __VosimOscillator__