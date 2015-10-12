#ifndef __SourceUnit__
#define __SourceUnit__


#include "Unit.h"
class SourceUnit :
  public Unit
{
public:
  virtual ~SourceUnit(){};
  virtual void trigger() = 0;
  virtual void release() = 0;
};

#endif // __SourceUnit__