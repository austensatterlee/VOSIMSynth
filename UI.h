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
  vector<double> m_Buffer;
  int m_syncIndex;
  int m_BufInd;
  int m_BufSize;
  double m_currWindowMinY;
  double m_currWindowMaxY;
  double m_minY;
  double m_maxY;
  int m_Padding;
  IRECT m_InnerRect;

  double toScreenX(double val) {
    return val*m_InnerRect.W() + m_InnerRect.L;
  }
  double toScreenY(double val) {
    return -(val - m_minY) / (m_maxY - m_minY)*m_InnerRect.H() + m_InnerRect.B;
  }
public:
  Oscilloscope(IPlugBase *pPlug, IRECT pR, int size=1) :
    IControl(pPlug, pR),
    m_syncIndex(0),
    m_BufInd(0),
    m_currWindowMinY(0),
    m_currWindowMaxY(1),
    m_Padding(50)
  {
    m_InnerRect = IRECT(pR.L + m_Padding, pR.T + m_Padding, pR.R - m_Padding, pR.B - m_Padding);
    setBufSize(size);
    m_Buffer = vector<double>(m_BufSize, 0.0);
  };
  ~Oscilloscope() {};

  bool IsDirty() { return true; };

  /* ends the current capture window and displays the result */
  void sync() {
    m_syncIndex = m_BufInd;
    m_BufInd = 0;
    m_minY = m_currWindowMinY;
    m_maxY = m_currWindowMaxY;
    m_currWindowMinY = 0;
    m_currWindowMaxY = 0;
  }

  void setBufSize(int s) {
      m_Buffer.resize(s);
      m_Buffer.clear();
      m_BufSize = s;
      m_syncIndex = 0;
      m_BufInd = 0;
  }

  void input(double y) {
    if (y > m_currWindowMaxY)
      m_currWindowMaxY = y + (y - m_currWindowMaxY);
    if (y < m_currWindowMinY)
      m_currWindowMinY = y + (y - m_currWindowMinY);
    m_BufInd++;
    if (m_BufInd >= m_BufSize)
      m_BufInd = 0;
    else if (m_BufInd >= m_Buffer.size())
      m_Buffer.push_back(y);
    else
      m_Buffer[m_BufInd] = y;
  }

  bool Draw(IGraphics *pGraphics) {
    if (!m_BufInd || !m_BufSize)
      return false;
    IColor fgcolor(255, 255, 255, 255);
    IColor gridcolor(150, 255, 25, 25);
    IColor bgcolor(150, 25, 25, 255);
    IText  txtstyle(&gridcolor);
    pGraphics->DrawRect(&bgcolor, &m_InnerRect);
    /* zero line */
    double zero = toScreenY(0);
    pGraphics->DrawLine(&gridcolor, mRECT.L, zero, mRECT.W() + mRECT.L, zero);

    char gridstr[128];
    snprintf(gridstr, 16, "%.4f", m_maxY);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, mRECT.T, m_InnerRect.L, m_InnerRect.T));
    snprintf(gridstr, 16, "%.2f", 0.0);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, zero - 10, m_InnerRect.L, zero + 10));
    snprintf(gridstr, 16, "%.4f", m_minY);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, m_InnerRect.B, m_InnerRect.L, mRECT.B));
    sprintf(gridstr, "sync: %d, curr ind: %d\nbuf size:%d/%d", m_syncIndex, m_BufInd, m_BufSize, m_Buffer.size());
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.R-200, mRECT.T, mRECT.R, mRECT.B));
    double x1, y1;
    double x2, y2;
    int i = 1;
    int previ = i-1 < 0 ? m_BufSize-1 : i-1;
    for (int j=1; j < m_BufSize-1; j++, i++) {
      if(i >= m_BufSize-1)
        i=0;
      x1 = toScreenX((j - 1) * 1. / (m_BufSize - 1));
      x2 = toScreenX(j * 1. / (m_BufSize - 1));
      y1 = toScreenY(m_Buffer[previ]);
      y2 = toScreenY(m_Buffer[i]);
      pGraphics->DrawLine(&fgcolor, x1, y1, x2, y2, 0, true);
      previ = i;
    }
    /*x1 = toScreenX((m_BufInd-1)*1. / (m_BufSize - 1));
    x2 = toScreenX(m_BufInd*1. / (m_BufSize - 1));
    y1 = toScreenY(m_Buffer[m_BufInd-1]);
    y2 = toScreenY(m_Buffer[m_BufInd]);
    pGraphics->DrawLine(&fgcolor, x1, y1, x2, y2, 0, true);*/
    return true;
  }

};

void attachKnob(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, IBitmap *pBmp);
void attachSwitch(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, IBitmap *pBmp);
#endif