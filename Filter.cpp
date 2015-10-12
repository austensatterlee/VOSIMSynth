#include "Filter.h"

double Filter::process(double input) {
  XBuf[xBufInd] = input;
  YBuf[yBufInd] = 0.0;
  double *output = &YBuf[yBufInd];
  int i,j;
  for (i = 0, j = xBufInd; i < numXCoefs; i++,j--) {
    if (j < 0)
      j = numXCoefs-1;
    YBuf[yBufInd] += XBuf[j] * XCoefs[i];
  }
  for (i = 0, j = yBufInd-1; i < numYCoefs; i++, j--) {
    if (j < 0)
      j = numYCoefs;
    YBuf[yBufInd] += YBuf[j] * YCoefs[i];
  }
  xBufInd++;
  if (xBufInd == numXCoefs)
    xBufInd = 0;
  yBufInd++;
  if (yBufInd == numYCoefs+1)
    yBufInd = 0;
  return *output;
}
