#ifndef __VosimOscillator__
#define __VosimOscillator__

#include "Oscillator.h"

class VosimOscillator : public Oscillator
{
private:
  /* internal state */
  double		m_CurrPulseGain = 1;
  double		m_LastPulsePhase = 0;
  bool		  m_UseRelativeWidth;
  double    m_CompensationGain = 1.0;
public:
  double    m_PhaseScale = 1;
  double  getPulsePhase() { return m_Phase*m_PhaseScale; };
  virtual double process();
  void toggleRelativePFreq(bool b) { m_UseRelativeWidth = b; };
  VosimOscillator() :
    Oscillator(),
    m_UseRelativeWidth(true)
  {
    addParams({"decay","pulsepitch","number"});
    m_params["number"].mod(SET,1.0);
  };
};

#endif // __VosimOscillator__