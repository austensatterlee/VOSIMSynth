#ifndef __DSPMATH__
#define __DSPMATH__
#include "IPlugStructs.h"
#include "NDPoint.h"
#include <cmath>

#define LERP(A,B,F) (((B)-(A))*(F)+(A))
#define INVLERP(A,B,X) (((X)-(A))/((B)-(A)))
#define CLAMP(x,lo,hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

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

  template<typename T>
  inline T WRAP(T x, T modulo)
  {
	  T newx = x;
	  while(newx >= modulo)
	  {
		  newx -= modulo;
	  }
	  while(newx<0)
	  {
		  newx += modulo;
	  }
	  return newx;
  }

  inline IRECT shiftIRECT(const IRECT& a_irect, int a_x, int a_y)
  {
    IRECT shifted{a_irect.L + a_x, a_irect.T + a_y, a_irect.R + a_x, a_irect.B + a_y};
    return shifted;
  }

  template <typename T>
  IRECT NDPointToIRECT(const NDPoint<2,T>& pt)
  {
	  return{ pt[0],pt[1],pt[0],pt[1] };
  }

  template <typename T>
  NDPoint<2,T> closestPointOnLine(const NDPoint<2, T>& pt, const NDPoint<2, T>& a, const NDPoint<2, T>& b)
  {
	  
	  double ablength = a.distFrom(b);
	  NDPoint<2, double> abnorm = static_cast<NDPoint<2,double>>(a - b) * (1.0/ ablength);
	  double proj = static_cast<NDPoint<2, double>>(pt - b).dot(abnorm);
	  proj = CLAMP(proj, 0, ablength);
	  return b + abnorm*proj;
  }

  template <typename T>
  double pointLineDistance(const NDPoint<2,T>& pt, const NDPoint<2,T>& a, const NDPoint<2,T>& b)
  {
	  return (pt-closestPointOnLine(pt, a, b)).mag();
  }
}
#endif
