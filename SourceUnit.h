#ifndef __SourceUnit__
#define __SourceUnit__


#include "Unit.h"
class SourceUnit :
  public Unit
{
public:
  SourceUnit()
  {
    addParams({ "gain" });
    modifyParameter("gain", SET, 0.0);
  }
  virtual ~SourceUnit() {};
  void noteOn(int pitch, int vel)
  {
    modifyParameter("gain", SET, vel/127.0);
    modifyParameter("pitch", SET, pitch);
  };
  void noteOff(int pitch, int vel) {
    modifyParameter("gain", SET, 0.0);
  };
private:
  virtual double finishProcessing(double);
};

inline double SourceUnit::finishProcessing(double o)
{
  return getParam("gain")*o;
}

#endif // __SourceUnit__