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
    SourceUnit(string name) :
    Unit(name)
    {
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
    virtual bool isActive() const = 0;
	  Signal0<> m_extSyncPort;
	};
}

#endif // __SourceUnit__