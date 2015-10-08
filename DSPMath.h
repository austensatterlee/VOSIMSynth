#ifndef __DSPMATH__
#define __DSPMATH__
#include <cmath>
#include "tables.h"
#define LERP(A,B,F) (((B)-(A))*(F)+(A))

inline int gcd(int a, int b) {
  int c;
  while (a != 0) {
    c = a; a = b%a;  b = c;
  }
  return b;
}

inline double pitchToFreq(double pitch) { return lut_pitchtable.getlinear(std::fmin(127,std::fmax(pitch,0)) / 128.0); }
inline double dbToAmp(double db) { return std::pow(10, 0.05*db); }
inline double ampToDb(double amp) { return 20 * std::log10(amp); }
#endif
