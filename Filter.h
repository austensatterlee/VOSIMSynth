#ifndef __FILTER__
#define __FILTER__
#include <cstring>
#include <cstdlib>
#include "DSPComponent.h"
using namespace std;
class Filter : public DSPComponent<double>{
protected:
  int numYCoefs,numXCoefs;
  double *YCoefs,*XCoefs;
  double *YBuf,*XBuf;
  int xBufInd,yBufInd;
public:
  Filter(double *X, double *Y, int nX, int nY) {
    freezeParams();
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
  int getSamplesPerPeriod() const { return std::fmax(numYCoefs,numXCoefs); };
};
#endif