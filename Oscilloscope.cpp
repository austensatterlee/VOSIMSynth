#include "Oscilloscope.h"
#include "SourceUnit.h"
namespace syn
{
  Oscilloscope::Oscilloscope(IPlugBase * pPlug, IRECT pR, int size) :
    IControl(pPlug, pR),
    m_periodCount(0),
    m_BufInd(0),
    m_minY(0),
    m_maxY(1),
    m_Padding(50),
    m_displayPeriods(1),
    m_isActive(false),
    m_currSyncDelay(0),
    m_syncDelayEst(1),
    m_usePeriodEstimate(false)
  {
    m_InnerRect = IRECT(pR.L + m_Padding, pR.T + m_Padding, pR.R - m_Padding, pR.B - m_Padding);
    setBufSize(size);
    m_inputBuffer = vector<double>(m_BufSize, 0.0);
  }

  inline bool Oscilloscope::IsDirty()
  {
    return m_isActive && mDirty;
  }

  inline void Oscilloscope::setPeriod(int nsamp)
  {
    m_numSamplesInPeriod = nsamp;
    setBufSize(m_displayPeriods*nsamp * 2);
  }

  void Oscilloscope::sync(bool isSynced)
  {
    if (m_isActive)
    {
      m_currSyncDelay++;
      if (isSynced)
      {
        m_periodCount++;
        if (m_periodCount >= m_displayPeriods)
        {
          m_syncDelayEst += 0.05*((double)m_currSyncDelay - m_syncDelayEst);
          m_periodCount = 0;
          m_syncIndexQueue.push_back(m_currSyncDelay);
          m_currSyncDelay = 0;

          if (!m_usePeriodEstimate && m_currTriggerSrc && getPeriod() != m_currTriggerSrc->getSamplesPerPeriod())
          {
            setPeriod(m_currTriggerSrc->getSamplesPerPeriod());
          }
          else if (m_usePeriodEstimate && m_BufSize != (int)m_syncDelayEst)
          {
            setPeriod((int)m_syncDelayEst / m_displayPeriods);
          }

          if (!m_transformFunc)
          {
            m_transformFunc = Oscilloscope::passthruTransform;
          }
          double windowMinY, windowMaxY;
          m_transformFunc(m_inputBuffer, m_outputBuffer, windowMinY, windowMaxY);
          m_minY += 1.1*(1.5*windowMinY - m_minY);
          m_maxY += 1.1*(1.5*windowMaxY - m_maxY);
        }
      }
    }
  }

  void Oscilloscope::input(double y)
  {
    if (m_isActive)
    {
      m_BufInd++;
      if (m_BufInd >= m_BufSize)
      {
        m_BufInd = 0;
      }
      m_inputBuffer[m_BufInd] = y;
      if (m_syncIndexQueue.size() && m_syncIndexQueue.front() == m_BufInd)
      {
        m_BufInd = 0;
        m_syncIndexQueue.pop_front();
        SetDirty();
      }
    }
  }

  void Oscilloscope::setBufSize(int s)
  {
    if (s > 0 && m_inputBuffer.size() != s)
      m_inputBuffer.resize(s, 0);
    m_BufSize = s;
    while (m_BufInd > m_BufSize)
    {
      m_BufInd -= m_BufSize;
    }
  }

  void Oscilloscope::disconnectInput()
  {
    if (m_currInput)
    {
      m_currInput->m_extOutPort.Disconnect(this, &Oscilloscope::input);
      m_syncIndexQueue.clear();
    }
    m_currInput = nullptr;
  }

  void Oscilloscope::disconnectTrigger()
  {
    if (m_currTriggerSrc)
    {
      m_currTriggerSrc->m_extSyncPort.Disconnect(this, &Oscilloscope::sync);
      m_syncIndexQueue.clear();
    }
    m_currTriggerSrc = nullptr;
  }

  void Oscilloscope::disconnectInput(Unit& srccomp)
  {
    if (m_currInput && m_currInput == &srccomp)
    {
      disconnectInput();
    }
  }

  void Oscilloscope::disconnectTrigger(SourceUnit& srccomp)
  {
    if (m_currTriggerSrc && m_currTriggerSrc == &srccomp)
    {
      disconnectTrigger();
    }
  }

  void Oscilloscope::connectInput(Unit& comp)
  {
    // connect 
    disconnectInput();
    comp.m_extOutPort.Connect(this, &Oscilloscope::input);
    m_currInput = &comp;
    m_BufInd = 0;
    m_currSyncDelay = 0;
    m_periodCount = 0;
  }

  void Oscilloscope::connectTrigger(SourceUnit& comp)
  {
    // connect
    disconnectTrigger();
    comp.m_extSyncPort.Connect(this, &Oscilloscope::sync);
    m_currTriggerSrc = &comp;
    m_BufInd = 0;
    m_currSyncDelay = 0;
    m_periodCount = 0;
  }

  void Oscilloscope::OnMouseUp(int x, int y, IMouseMod* pMod)
  {
    m_isActive ^= true;
  }

  void Oscilloscope::OnMouseWheel(int x, int y, IMouseMod* pMod, int d)
  {
    m_syncIndexQueue.clear();
    m_displayPeriods = m_displayPeriods + d;
    if (m_displayPeriods <= 0)
      m_displayPeriods = 1;
  }

  bool Oscilloscope::Draw(IGraphics *pGraphics)
  {
    if (!m_isActive)
      return false;
    IColor fgcolor(255, 255, 255, 255);
    IColor gridcolor(150, 255, 25, 25);
    IColor bgcolor(150, 25, 25, 255);
    IColor cval(255 * m_InnerRect.W() / m_outputBuffer.size(), 0, 0, 0);
    IText  txtstyle(&gridcolor);
    pGraphics->DrawRect(&bgcolor, &m_InnerRect);
    /* zero line */
    double zero = toScreenY(0);
    pGraphics->DrawLine(&gridcolor, mRECT.L, zero, mRECT.W() + mRECT.L, zero);

    char gridstr[128];
    snprintf(gridstr, 32, "%.4f", m_maxY);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, mRECT.T, m_InnerRect.L, m_InnerRect.T));
    snprintf(gridstr, 32, "%.2f", 0.0);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, zero - 10, m_InnerRect.L, zero + 10));
    snprintf(gridstr, 32, "%.4f", m_minY);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L + 10, m_InnerRect.B, m_InnerRect.L, mRECT.B));
    sprintf(gridstr, "Displaying %d periods\nBuffer size: %d (in) %d (out)\nInput frequency: %.2f Hz\nSync: %d", \
      m_displayPeriods, m_BufSize, m_outputBuffer.size(), GetPlug()->GetSampleRate() / m_numSamplesInPeriod, m_numSamplesInPeriod);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.R - 200, mRECT.T, mRECT.R, mRECT.B));
    double x1, y1;
    double x2, y2;

    int bufReadLen = m_displayPeriods*m_numSamplesInPeriod;
    for (int j = 1; j < bufReadLen; j += 1)
    {
      x1 = toScreenX((j - 1) * 1. / (bufReadLen));
      x2 = toScreenX((j)* 1. / (bufReadLen));
      y1 = toScreenY(m_outputBuffer[j - 1]);
      y2 = toScreenY(m_outputBuffer[j]);
      cval.R = 255 * (1 - (y2 - m_InnerRect.T) / m_InnerRect.H());
      cval.B = 255 * (1 - abs(y2 - y1) / m_InnerRect.H());
      cval.B = 255 * (1 - abs(-y2 + y1) / m_InnerRect.H());
      pGraphics->DrawLine(&cval, x2, m_InnerRect.B, x2, y2);
      pGraphics->DrawLine(&fgcolor, x1, y1, x2, y2, 0, true);
    }
    return true;
  }
}