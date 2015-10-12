#ifndef __SinkUnit__
#define __SinkUnit__

#include "Unit.h"

class SinkUnit :
  public Unit
{
public:
  virtual ~SinkUnit(){};
};

class AccumulatingSink :
  public SinkUnit
{
  public:
    AccumulatingSink() :
    SinkUnit()
    {
      addParams({"output"});
    }    
  virtual ~AccumulatingSink(){};
protected:
  double process()
  {
    return getParam("output");
  }

};
#endif // __SinkUnit__