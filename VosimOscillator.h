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
      m_UseRelativeWidth(true)
    {
      addParam(UnitParameter("decay",1.0,0,1,DOUBLE_TYPE,false));
      addParam(UnitParameter("pulsepitch",0.5,0,1, DOUBLE_TYPE,false));
      addParam(UnitParameter("number",1,0,4, DOUBLE_TYPE,false));
    };
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
  };
}

#endif // __VosimOscillator__