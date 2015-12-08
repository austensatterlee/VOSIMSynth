#ifndef __DSPMATH__
#define __DSPMATH__
#include "tables.h"

#define LERP(A,B,F) (((B)-(A))*(F)+(A))
#define INVLERP(A,B,X) (((X)-(A))/((B)-(A)))

namespace syn
{
  inline int gcd(int a, int b)
  {
    int c;
    while (a != 0)
    {
      c = a; a = b%a;  b = c;
    }
    return b;
  }

  inline double pitchToFreq(double pitch)
  {
    double freq = lut_pitch_table.getlinear(pitch*0.0078125);
    if (freq == 0)
      freq = 1;
    return freq;
  }
}
#endif
