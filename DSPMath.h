#ifndef __DSPMATH__
#define __DSPMATH__
#include <cmath>
#include <vector>
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
    double freq = lut_pitch_table.getlinear(pitch / 128.0);
    if (freq == 0)
      freq = 1;
    return freq;
  }
  inline double dbToAmp(double db) { return std::pow(10, 0.05*db); }
  inline double ampToDb(double amp) { return 20 * std::log10(amp); }
}
#endif
