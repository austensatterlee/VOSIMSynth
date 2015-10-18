#include "Oscilloscope.h"
#include "SourceUnit.h"
namespace syn
{
  Oscilloscope::Oscilloscope(IPlugBase * pPlug, IRECT pR, int size) :
    IControl(pPlug, pR),
    m_syncIndex(0),
    m_BufInd(0),
    m_minY(0),
    m_maxY(1),
    m_currWindowMinY(0),
    m_currWindowMaxY(0),
    m_Padding(50),
    m_displayPeriods(1),
    m_isActive(false)
  {
    m_InnerRect = IRECT(pR.L + m_Padding, pR.T + m_Padding, pR.R - m_Padding, pR.B - m_Padding);
    setBufSize(size);
    m_Buffer = vector<double>(m_BufSize, 0.0);
  }

  inline bool Oscilloscope::IsDirty()
  {
    return m_currTriggerSrc && m_currInput && m_isActive && mDirty;
  }

  inline void Oscilloscope::setPeriod(int nsamp)
  {
    setBufSize(m_displayPeriods*nsamp);
  }

  void Oscilloscope::sync()
  {
    if (m_isActive && m_currTriggerSrc)
    {
      m_syncIndex = m_BufInd;
      m_currWindowMaxY = 0.99*m_currWindowMaxY;
      m_currWindowMinY = 0.99*m_currWindowMinY;


      if (getPeriod() != m_currTriggerSrc->getSamplesPerPeriod())
      {
        setPeriod(m_currTriggerSrc->getSamplesPerPeriod());
      }
      SetDirty();
    }
  }

  void Oscilloscope::input(double y)
  {
    if (m_isActive && m_currInput)
    {
      if (y > m_currWindowMaxY)
      {
        m_maxY = y;
        m_currWindowMaxY = m_maxY;
      }
      if (y < m_currWindowMinY)
      {
        m_minY = y;
        m_currWindowMinY = m_minY;
      }

      m_BufInd++;
      if (m_BufInd >= m_BufSize)
      {
        m_BufInd = 0;
      }
      else if (m_BufInd >= m_Buffer.size())
        m_Buffer.push_back(y);
      else
        m_Buffer[m_BufInd] = y;
    }
  }

  void Oscilloscope::setBufSize(int s)
  {
    if (m_Buffer.size() < s)
      m_Buffer.resize(s);
    m_BufSize = s;
    while (m_BufInd > m_BufSize)
    {
      m_BufInd -= m_BufSize;
    }
    while (m_syncIndex > m_BufSize)
    {
      m_syncIndex -= m_BufSize;
    }
  }

  void Oscilloscope::disconnectInput()
  {
    if (m_currInput)
    {
      m_currInput->m_extOutPort.Disconnect(this, &Oscilloscope::input);
    }
    m_currInput = nullptr;
  }

  void Oscilloscope::disconnectTrigger()
  {
    if (m_currTriggerSrc)
    {
      m_currTriggerSrc->m_extSyncPort.Disconnect(this, &Oscilloscope::sync);
    }
    m_currTriggerSrc = nullptr;
  }

  void Oscilloscope::disconnectInput(Unit& srccomp)
  {
    if (m_currInput && m_currInput==&srccomp)
    {
      m_currInput->m_extOutPort.Disconnect(this, &Oscilloscope::input);
    }
    m_currInput = nullptr;
  }

  void Oscilloscope::disconnectTrigger(SourceUnit& srccomp)
  {
    if (m_currTriggerSrc && m_currInput == &srccomp)
    {
      m_currTriggerSrc->m_extSyncPort.Disconnect(this, &Oscilloscope::sync);
    }
    m_currTriggerSrc = nullptr;
  }

  void Oscilloscope::connectInput(Unit& comp)
  {
    // connect 
    disconnectInput();
    comp.m_extOutPort.Connect(this, &Oscilloscope::input);
    m_currInput = &comp;
  }

  void Oscilloscope::connectTrigger(SourceUnit& comp)
  {
    // connect
    disconnectTrigger();
    comp.m_extSyncPort.Connect(this, &Oscilloscope::sync);
    m_currTriggerSrc = &comp;
  }

  void Oscilloscope::OnMouseUp(int x, int y, IMouseMod* pMod)
  {
    m_isActive ^= true;
  }

  void Oscilloscope::OnMouseWheel(int x, int y, IMouseMod* pMod, int d)
  {
    m_displayPeriods = m_displayPeriods + d;
    if (m_displayPeriods <= 0)
      m_displayPeriods = 1;
  }

  bool Oscilloscope::Draw(IGraphics *pGraphics)
  {
    if (!m_currTriggerSrc || !m_currInput || !m_isActive)
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
    sprintf(gridstr, "Displaying %d periods\nBuffer size: %d\nInput frequency: %.2f Hz\nSync: %d", m_displayPeriods, m_BufSize, GetPlug()->GetSampleRate() / m_BufSize * m_displayPeriods, m_syncIndex);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.R - 200, mRECT.T, mRECT.R, mRECT.B));
    double x1, y1;
    double x2, y2;
    int i = m_syncIndex + 1 >= m_BufSize ? 0 : m_syncIndex + 1;
    int previ = i - 1 < 0 ? m_BufSize - 1 : i - 1;
    for (int j = 1; j < m_BufSize - 1; j++)
    {
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
    return true;
  }
}