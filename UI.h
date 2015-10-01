#ifndef __UI_H__
#define __UI_H__
#include "IControl.h"
#include "IParam.h"
#include <vector>


using namespace std;

class UI {
private:

public:
  UI() {};
  ~UI() {};
};

class Panel {
private:
public:
  Panel() {};
  ~Panel() {};
};

class Oscilloscope : public IControl {
protected:
  vector<double> mBuffer;
  int mBufSize;
  int mBufInd = 0;
  double mMinY = 1.0;
  double mMaxY = 0.0;
  int mPadding = 50;
  IRECT mInnerRect;

  double toScreenX(double val) {
    return val*mInnerRect.W() + mInnerRect.L;
  }
  double toScreenY(double val) {
    return -(val - mMinY) / (mMaxY - mMinY)*mInnerRect.H() + mInnerRect.B;
  }
public:
  Oscilloscope(IPlugBase *pPlug, IRECT pR, int bufSize) : IControl(pPlug, pR) {
    mInnerRect = IRECT(pR.L+mPadding,pR.T+mPadding,pR.R-mPadding,pR.B-mPadding);
    mBufSize = bufSize;
    mBuffer = vector<double>(mBufSize, 0.0);
  };
  ~Oscilloscope() {};

  bool IsDirty() { return true; };

  void setBufSize(int s) {
    if (s < 1)
      s = 1;
    if (s > 100000) {
      s = 100000;
    }
    if (s>mBufSize)
      mBuffer.resize(s);
    mBufSize = s;
    mBufInd = 0;
    mMinY = mMaxY;
    mMaxY = 0.9*mMinY;
  }
  void input(double y) {
    if (y > mMaxY)
      mMaxY = y;
    if (y < mMinY)
      mMinY = y;
    mBuffer[mBufInd] = y;
    mBufInd++;
    if (mBufInd == mBufSize) {
      mBufInd = 0;
      this->SetDirty();
    }
  }

  bool Draw(IGraphics *pGraphics) {
    if (mBufInd == 0)
      return true;
    IColor fgcolor(255, 255, 255, 255);
    IColor gridcolor(150, 255, 25, 25);
    IColor bgcolor(150, 25, 25, 255);
    IText  txtstyle(&gridcolor);
    char gridstr[16];
    double x1, y1;
    double x2, y2;
    double zero = toScreenY(0);
    pGraphics->DrawRect(&bgcolor, &mInnerRect);
    /* zero line */
    pGraphics->DrawLine(&gridcolor,mRECT.L, zero,mRECT.W()+mRECT.L, zero);

    snprintf(gridstr, 16, "%.4f", mMaxY);
    pGraphics->DrawIText(&txtstyle, gridstr,&IRECT(mRECT.L,mRECT.T,mInnerRect.L, mInnerRect.T));
    snprintf(gridstr, 16, "%.2f", 0.0);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, zero-10, mInnerRect.L, zero+10));
    snprintf(gridstr, 16, "%.4f", mMinY);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, mInnerRect.B, mInnerRect.L, mRECT.B));
    for (int i = 1, previ = 0, j = 1; i != mBufSize; i++, j++) {
      x1 = toScreenX((j - 1)*1. / (mBufSize-1));
      y1 = toScreenY(mBuffer[previ]);
      x2 = toScreenX(j*1. / (mBufSize-1));
      y2 = toScreenY(mBuffer[i]);
      pGraphics->DrawLine(&fgcolor, x1, y1, x2, y2, 0, true);
      previ = i;
    }
    return true;
  }

};

void attachKnob(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, IBitmap *pBmp);
void attachSwitch(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, IBitmap *pBmp);
#endif