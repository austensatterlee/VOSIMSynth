#ifndef __SourceUnit__
#define __SourceUnit__
#include "Unit.h"
#include "GallantSignal.h"

using Gallant::Signal0;

namespace syn
{
	class SourceUnit :
	  public Unit
	{
	public:
    SourceUnit()
    {
      addParam("gain",new UnitParameter(1.0));
    }
	  virtual ~SourceUnit() {};
	  virtual void noteOn(int pitch, int vel)
	  {
	  };
	  virtual void noteOff(int pitch, int vel) {
	  };
    virtual int getSamplesPerPeriod() const
    {
      return m_Fs;
    };
	  Signal0<> m_extSyncPort;
	};
}

#endif // __SourceUnit__