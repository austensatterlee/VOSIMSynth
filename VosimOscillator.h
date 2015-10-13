#ifndef __VosimOscillator__
#define __VosimOscillator__

#include "Oscillator.h"

namespace syn
{
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
	    m_params["number"]->mod(SET,1.0);
	    m_params["decay"]->mod(SET, 0.5);
	    m_params["pulsepitch"]->mod(SET, 24);
	  };
	};
}

#endif // __VosimOscillator__