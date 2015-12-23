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

  template<typename T>
  inline T max(T a1, T a2)
  {
    return a1 > a2 ? a1 : a2; 
  }

  template<typename T>
  inline T min(T a1, T a2)
  {
    return a1 < a2 ? a1 : a2;
  }

  inline IRECT shiftIRECT(const IRECT& a_irect, int a_x, int a_y)
  {
    IRECT shifted{a_irect.L + a_x, a_irect.T + a_y, a_irect.R + a_x, a_irect.B + a_y};
    return shifted;
  }
}
#endif
