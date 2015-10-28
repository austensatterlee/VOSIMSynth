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
    {}
    virtual ~SourceUnit() {};
    virtual void noteOn(int pitch, int vel)
    {};
    virtual void noteOff(int pitch, int vel)
    {};
    virtual int getSamplesPerPeriod() const
    {
      return m_Fs;
    };
    virtual bool isActive() const = 0;
    Signal1<bool> m_extSyncPort;
  protected:
    bool m_isSynced;
    double finishProcessing(double o)
    {
      if (m_isSynced)
      {
        m_extSyncPort.Emit(true);
      }
      else
      {
        m_extSyncPort.Emit(false);
      }
      m_isSynced = false;
      return o;
    }
  };
}

#endif // __SourceUnit__