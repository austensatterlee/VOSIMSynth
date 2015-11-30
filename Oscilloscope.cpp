#include "Oscilloscope.h"
#include "SourceUnit.h"
#include "UI.h"

namespace syn
{
  Oscilloscope::Oscilloscope(IPlugBase * pPlug, IRECT pR) :
    IControl(pPlug, pR),
    m_periodCount(0),
    m_BufInd(0),
    m_minY(0),
    m_maxY(1),
    m_Padding(25),
    m_displayPeriods(1),
    m_isActive(false),
    m_currSyncDelay(0),
    m_syncDelayEst(1)
  {
    m_InnerRect = pR.GetPadded(-m_Padding);

    /* Build context menu */
    for (int i = 0; i < OSCILLOSCOPE_CONFIGS.size(); i++)
    {
      const OscilloscopeConfig* currconf = &OSCILLOSCOPE_CONFIGS[i];
      m_menu.AddItem(currconf->name.c_str(), i);
    }
    m_config = &OSCILLOSCOPE_CONFIGS[0];
    m_menu.CheckItemAlone(0);
  }

  void Oscilloscope::setConfig(OscilloscopeConfig* config)
  {
    m_config = config;
    m_minY = 1.0;
    m_maxY = -1.0;
  }

  void Oscilloscope::process()
  {
    if (!m_isActive || !m_currInput) return;
    const vector<double>& srcbuffer = m_currInput->getLastOutputBuffer();
    if (m_inputRingBuffer.size() <= srcbuffer.size())
    {
      setPeriod(srcbuffer.size());
    }
    int period = getPeriod();
    for (int i = 0; i < srcbuffer.size(); i++)
    {
      m_BufInd++;
      if (m_BufInd >= m_inputRingBuffer.size() - 1)
      {
        m_BufInd = 0;
      }
      m_inputRingBuffer[m_BufInd] = srcbuffer[i];
      m_currSyncDelay++;

      if (
        (!m_config->useAutoSync && m_currSyncDelay > m_config->defaultBufSize) ||
        (m_currTriggerSrc && m_currSyncDelay > m_currTriggerSrc->getSamplesPerPeriod())
        )
      {
        int correctPeriod;
        if (m_config->useAutoSync)
        {
          correctPeriod = m_currTriggerSrc->getSamplesPerPeriod();
        }
        else
        {
          correctPeriod = m_config->defaultBufSize;
        }
        if (period != correctPeriod)
        {
          setPeriod(correctPeriod);
        }

        m_periodCount++;
        if (m_periodCount >= m_displayPeriods)
        {
          m_periodCount = 0;
          m_displayIndex = m_BufInd;
          // Copy input chunk from circular input buffer to a non-circular buffer
          m_mutex.Enter();
          m_inputBuffer.resize(getPeriod()*m_displayPeriods);
          int inputBufInd = m_displayIndex;
          for (int i = m_inputBuffer.size() - 1; i >= 0; i--)
          {
            if (inputBufInd < 0) inputBufInd = m_inputRingBuffer.size() - 1;
            m_inputBuffer[i] = m_inputRingBuffer[inputBufInd];
            inputBufInd--;
          }
          m_mutex.Leave();
          SetDirty();
        }
        m_syncDelayEst += 0.1*((double)m_currSyncDelay - m_syncDelayEst);
        m_currSyncDelay = 0;
      }
    }
  }

  void Oscilloscope::setBufSize(int s)
  {
    if (s <= 0) return;
    if (m_inputRingBuffer.size() <= s)
    {
      m_inputRingBuffer.resize(s);
    }
    m_BufSize = s;
    while (m_BufInd > s)
    {
      m_BufInd -= s;
    }
  }

  void Oscilloscope::disconnectInput()
  {
    m_currInput = nullptr;
  }

  void Oscilloscope::disconnectTrigger()
  {
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
    m_currInput = &comp;
    m_currSyncDelay = 0;
    m_periodCount = 0;
  }

  void Oscilloscope::connectTrigger(SourceUnit& comp)
  {
    // connect
    disconnectTrigger();
    m_currTriggerSrc = &comp;
    m_currSyncDelay = 0;
    m_periodCount = 0;
  }

  void Oscilloscope::OnMouseUp(int x, int y, IMouseMod* pMod)
  {
  }

  void Oscilloscope::OnMouseDown(int x, int y, IMouseMod* pMod)
  {
    if (pMod->L)
    {
      m_isActive ^= true;
    }
    else if (pMod->R)
    {
      IPopupMenu* selectedMenu = mPlug->GetGUI()->CreateIPopupMenu(&m_menu, x, y);
      if (selectedMenu == &m_menu)
      {
        int itemChosen = selectedMenu->GetChosenItemIdx();
        setConfig(&OSCILLOSCOPE_CONFIGS[itemChosen]);
        selectedMenu->CheckItemAlone(itemChosen);
        DBGMSG("item chosen, main menu %i\n", itemChosen);
      }
    }
    m_BufInd = 0;
    m_currSyncDelay = 0;
    m_periodCount = 0;
  }

  void Oscilloscope::OnMouseWheel(int x, int y, IMouseMod* pMod, int d)
  {
    int change = 0;
    if (d > 0)
    {
      change = 1;
    }
    else if (d < 0)
    {
      change = -1;
    }
    if (m_displayPeriods + change > 0)
    {
      m_displayPeriods = m_displayPeriods + change;
      m_periodCount = 0;
    }
  }

  int Oscilloscope::getBufReadLength()
  {
    int bufReadLen = getPeriod()*m_displayPeriods;
    if (bufReadLen >= m_config->outputbuf.size())
      bufReadLen = m_config->outputbuf.size() - 1;
    return bufReadLen;
  }

  bool Oscilloscope::Draw(IGraphics *pGraphics)
  {
    pGraphics->DrawRect(&(IColor)ColorPoint(palette[2] - getUnitv<4>(0)*40.0), &m_InnerRect);
    IText errortxtstyle(12, &IColor{ 255,255,0,0 }, "Helvetica", IText::kStyleNormal, IText::kAlignNear, 0, IText::kQualityClearType);
    IText txtstyle(12, &(IColor)(palette[4]), "Helvetica", IText::kStyleNormal, IText::kAlignNear, 0, IText::kQualityClearType, &(IColor)(palette[1]), &(IColor)(palette[3]));
    IText txtstylecenter(txtstyle); txtstylecenter.mAlign = IText::kAlignCenter;
    if (!m_isActive || !m_currInput)
    {
      pGraphics->DrawRect(&(IColor)ColorPoint(palette[2] - getUnitv<4>(0)*40.0), &mRECT);
      if (!m_currInput)
      {
        pGraphics->DrawIText(&errortxtstyle, "No input!", &m_InnerRect.GetPadded(-10));
      }
      return false;
    }
    ColorPoint bgcolor = palette[1] - getUnitv<4>(0)*127.0;
    pGraphics->FillIRect(&(IColor)(bgcolor), &m_InnerRect);
    IColor cval = (IColor)ColorPoint(getOnes<4>()*255.0 - bgcolor);
    /* zero line */
    double zero = toScreenY(0);
    pGraphics->DrawLine(&(IColor)(palette[1]), mRECT.L, zero, mRECT.W() + mRECT.L, zero);

    char gridstr[128];
    snprintf(gridstr, 32, "%.4f", m_maxY);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(m_InnerRect.L - 10, m_InnerRect.T, m_InnerRect.L, m_InnerRect.T + 10));
    snprintf(gridstr, 32, "%.2f %s", 0.0, m_config->yunits.c_str());
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, zero - 10, m_InnerRect.L, zero + 10));
    snprintf(gridstr, 32, "%.4f", m_minY);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(m_InnerRect.L - 10, m_InnerRect.B - 10, m_InnerRect.L, m_InnerRect.B));
    sprintf(gridstr, "Displaying %d periods | Buffer size: %d (in) %d (out) | Input frequency: %.2f Hz (%.2f Hz)", \
      m_displayPeriods, m_BufSize, m_config->outputbuf.size(), GetPlug()->GetSampleRate() / m_syncDelayEst, GetPlug()->GetSampleRate() / m_currTriggerSrc->getSamplesPerPeriod());
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, mRECT.T, mRECT.R, mRECT.B));
    sprintf(gridstr, m_config->xunits.c_str());
    pGraphics->DrawIText(&txtstylecenter, gridstr, &IRECT(mRECT.L, m_InnerRect.B + 10, mRECT.R, mRECT.B));
    double x1, y1;
    double x2, y2;

    if (m_inputRingBuffer.empty())
      return false;

    // Execute transform using non-circular input buffer
    m_mutex.Enter();
    m_config->doTransform(mPlug, m_inputBuffer);
    m_mutex.Leave();
    // Adjust axis boundaries
    if (m_config->argmin > 0 && m_config->argmax > 0)
    {
      double minY = m_config->outputbuf[m_config->argmin];
      double maxY = m_config->outputbuf[m_config->argmax];
      if (minY < m_minY) m_minY = minY;
      if (maxY > m_maxY) m_maxY = maxY;
    }

    // Draw transform output
    int bufreadlen = getBufReadLength();
    int numxticks = 10;
    double xtickdist = m_InnerRect.W() / (double)numxticks;
    int lastxtick = 0;
    for (int j = 1; j <= bufreadlen; j += 1)
    {
      x1 = toScreenX(m_config->xaxisticks[j - 1]);
      x2 = toScreenX(m_config->xaxisticks[j]);
      y1 = toScreenY(m_config->outputbuf[j - 1]);
      y2 = toScreenY(m_config->outputbuf[j]);
      cval.A = 255;// - 25* bufreadlen /m_InnerRect.W();
      cval.R *= (1 - (y2 - m_InnerRect.T) / m_InnerRect.H());
      cval.G *= 1;
      cval.B *= (1 - abs(y1 - y2) / m_InnerRect.H());
      //pGraphics->DrawLine(&cval, x2, m_InnerRect.B, x2, y2);
      pGraphics->DrawLine(&cval, x1, y1, x2, y2, 0, true);
      if (j == 1 || x2 - lastxtick >= xtickdist)
      {
        snprintf(gridstr, 128, "%s", m_config->xaxislbls[j].c_str());
        pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(x2, m_InnerRect.B, x2 + 10, m_InnerRect.B + 10));
        lastxtick = x2;
      }
    }
    return true;
  }
}