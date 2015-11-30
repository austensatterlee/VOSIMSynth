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
    {};
    virtual ~SourceUnit() {};
    virtual void noteOn(int pitch, int vel) = 0;
    virtual void noteOff(int pitch, int vel) = 0;
    virtual int getSamplesPerPeriod() const = 0;
    virtual bool isActive() const = 0;
    bool isSynced() const { return m_isSynced; };
  protected:
    bool m_isSynced;
    virtual void beginProcessing() {
      m_isSynced = false;
    }
  };
}

#endif // __SourceUnit__