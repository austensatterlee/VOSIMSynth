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
      m_UseRelativeWidth(true),
      m_decay(addParam("decay", DOUBLE_TYPE, 0, 1)),
      m_ppitch(addParam("pulsepitch", DOUBLE_TYPE, 0, 1)),
      m_number(addParam("number", DOUBLE_TYPE, 0, 4))
    {};
    VosimOscillator(const VosimOscillator& vosc);
    double    m_PhaseScale = 1;
    double  getPulsePhase() { return m_Phase*m_PhaseScale; };
    virtual double process();
    void toggleRelativePFreq(bool b) { m_UseRelativeWidth = b; };
  private:
    /* internal state */
    virtual Unit* cloneImpl() const { return new VosimOscillator(*this); };
    double		m_CurrPulseGain = 1;
    double		m_LastPulsePhase = 0;
    bool		  m_UseRelativeWidth;
    double    m_CompensationGain = 1.0;
    UnitParameter& m_decay;
    UnitParameter& m_ppitch;
    UnitParameter& m_number;
  };
}

#endif // __VosimOscillator__