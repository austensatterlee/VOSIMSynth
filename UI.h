#ifndef __UI__
#define __UI__
#include "IControl.h"
#include "IParam.h"
#include "DSPComponent.h"
#include <vector>
#include <list>

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
  int m_displayPeriods;
  double m_currWindowMinY;
  double m_currWindowMaxY;
  double m_minY;
  double m_maxY;
  int m_Padding;
  bool m_isActive;
  IRECT m_InnerRect;
  DSPComponent<double>* m_currInput = NULL;
  DSPComponent<double>* m_currTriggerSrc = NULL;

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
    m_Padding(50),
    m_displayPeriods(1),
    m_isActive(false)
  {
    m_InnerRect = IRECT(pR.L + m_Padding, pR.T + m_Padding, pR.R - m_Padding, pR.B - m_Padding);
    setBufSize(size);
    m_Buffer = vector<double>(m_BufSize, 0.0);
  };
  ~Oscilloscope() {};

  bool IsDirty() { return m_isActive && mDirty; };

  void setNumDisplayPeriods(int displayPeriods) {
    m_displayPeriods = displayPeriods;
  }

  /* ends the current capture window and displays the result */
  void sync() {
    m_syncIndex = m_BufInd;
    //m_BufInd = 0;
    m_minY = m_currWindowMinY;
    m_maxY = m_currWindowMaxY;
  }

  void setBufSize(int s) {
      if(m_Buffer.size()<s)
        m_Buffer.resize(s);
      m_BufSize = s;
      while (m_BufInd > m_BufSize) {
        m_BufInd -= m_BufSize;
      }
      while (m_syncIndex > m_BufSize) {
        m_syncIndex -= m_BufSize;
      }
  }

  void connectInput(DSPComponent<double>* comp) {
    if (m_currInput) {
      m_currInput->disconnectOutputTo(this,&Oscilloscope::input);
    }
    m_currInput = comp;
    m_currInput->connectOutputTo(this, &Oscilloscope::input);
  }

  void connectTrigger(DSPComponent<double>* comp) {
    if (m_currTriggerSrc) {
      m_currTriggerSrc->triggerOut.Disconnect(this, &Oscilloscope::sync);
    }
    m_currTriggerSrc = comp;
    m_currTriggerSrc->triggerOut.Connect(this, &Oscilloscope::sync);
  }

  void input(double y) {
    if (y > m_currWindowMaxY)
      m_currWindowMaxY = y + (y - m_currWindowMaxY);
    if (y < m_currWindowMinY)
      m_currWindowMinY = y + (y - m_currWindowMinY);
    m_BufInd++;
    if (m_BufInd >= m_BufSize){
      m_BufInd = 0;
      m_currWindowMinY = 0.99*(m_currWindowMinY)-0.001*1;
      m_currWindowMaxY = 0.99*(m_currWindowMaxY)+0.001*1;
      SetDirty();
    }
    else if (m_BufInd >= m_Buffer.size())
      m_Buffer.push_back(y);
    else
      m_Buffer[m_BufInd] = y;
  }

  void OnMouseUp(int x, int y, IMouseMod* pMod) {
    m_isActive ^= true;
  }

  bool Draw(IGraphics *pGraphics) {
    if(m_currInput==NULL)
      return false;
    if (m_BufSize != m_displayPeriods*m_currTriggerSrc->getSamplesPerPeriod()) {
      setBufSize(m_displayPeriods*m_currTriggerSrc->getSamplesPerPeriod() );
    }
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
    sprintf(gridstr, "Displaying %d periods\nBuffer size: %d\nInput frequency: %.2f Hz\nSync: %d", m_displayPeriods, m_BufSize, GetPlug()->GetSampleRate()/m_currTriggerSrc->getSamplesPerPeriod(),m_syncIndex);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.R-200, mRECT.T, mRECT.R, mRECT.B));
    double x1, y1;
    double x2, y2;
    int i = m_syncIndex;
    int previ = i-1 < 0 ? m_BufSize-1 : i-1;
    for (int j=1; j < m_BufSize-1; j++) {
      x1 = toScreenX((j - 1) * 1. / (m_BufSize - 1));
      x2 = toScreenX(j * 1. / (m_BufSize - 1));
      y1 = toScreenY(m_Buffer[previ]);
      y2 = toScreenY(m_Buffer[i]);
      pGraphics->DrawLine(&fgcolor, x1, y1, x2, y2, 0, true);
      previ = i;
      i++;
      if (i >= m_BufSize - 1)
        i = 0;
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