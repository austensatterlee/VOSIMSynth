#ifndef __FILTER__
#define __FILTER__
#include "Unit.h"
#include <cstring>
#include <cstdlib>
#define AA_FILTER_SIZE 6
const double AA_FILTER_X[1 + AA_FILTER_SIZE] = { 4.760635e-1, 2.856281, 7.140952, 9.521270, 7.140952, 2.856281, 4.760635e-1 };
const double AA_FILTER_Y[AA_FILTER_SIZE] = { -4.522403, -8.676844, -9.007512, -5.328429, -1.702543, -2.303303e-1 };

using namespace std;
class Filter : public Unit{
protected:
  int numYCoefs,numXCoefs;
  double *YCoefs,*XCoefs;
  double *YBuf,*XBuf;
  int xBufInd,yBufInd;
public:
  Filter(const double *X, const double *Y, const int nX, const int nY)
  {
    xBufInd = 0;
    yBufInd = 0;
    numXCoefs = nX;
    numYCoefs = nY;
    XCoefs = (double*)malloc(sizeof(double)*numXCoefs);
    YCoefs = (double*)malloc(sizeof(double)*numYCoefs);
    memcpy(XCoefs, X, numXCoefs*sizeof(double));
    memcpy(YCoefs, Y, numYCoefs*sizeof(double));
    XBuf = (double*)malloc(sizeof(double)*numXCoefs);
    YBuf = (double*)malloc(sizeof(double)*(numYCoefs+1));
    int i;
    for (i = 0; i < numXCoefs; i++) {
      XBuf[i] = 0.0;
    }
    for (i = 0; i < numYCoefs+1; i++) {
      YBuf[i] = 0.0;
    }
  };
  ~Filter() {
    delete[] YCoefs;
    delete[] XCoefs;
    delete[] YBuf;
    delete[] XBuf;
  };
  double process(const double input);
};
#endif