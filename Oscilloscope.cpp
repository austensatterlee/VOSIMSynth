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
    WDL_MutexLock lock(&m_mutex);
    m_config = config;
    m_minY = 1.0;
    m_maxY = -1.0;
  }

  void Oscilloscope::process()
  {
    if (m_isActive && m_currInput && m_inputBuffer.size())
    {
      WDL_MutexLock lock(&m_mutex);
      const vector<double>& srcbuffer = m_currInput->getLastOutputBuffer();
      for (int i = 0; i < srcbuffer.size(); i++)
      {
        m_BufInd++;
        if (m_BufInd >= m_BufSize)
        {
          m_BufInd = 0;
        }
        m_inputBuffer[m_BufInd] = srcbuffer[i];

        int period = getPeriod();
        m_currSyncDelay++;
        if ((m_currTriggerSrc && m_currTriggerSrc->isSynced()) || (!m_currTriggerSrc && m_currSyncDelay >= m_config->defaultBufSize))
        {
          m_periodCount++;
          // push wrapped sync offset to display queue
          int wrapped_syncDelay = m_currSyncDelay;
          while (wrapped_syncDelay >= period)
          {
            wrapped_syncDelay -= period;
          }

          int period = getPeriod();
          if (m_config->useAutoSync)
          {
            if (period != (int)(m_syncDelayEst / m_displayPeriods))
            {
              setPeriod((int)(m_syncDelayEst / m_displayPeriods));
            }
          }
          else
          {
            if (period != (int)m_config->defaultBufSize)
            {
              setPeriod(m_config->defaultBufSize);
            }
          }

          if (m_periodCount >= m_displayPeriods)
          {
            m_syncDelayEst += 0.1*((double)m_currSyncDelay - m_syncDelayEst);
            m_periodCount = 0;
            m_currSyncDelay = 0;
            m_BufInd = 0;
            SetDirty();
          }
        }
      }
    }
  }

  void Oscilloscope::setBufSize(int s)
  {
    if (s > 0)
    {
      m_inputBuffer.resize(s);

      m_BufSize = s;
      while (m_BufInd > s)
      {
        m_BufInd -= s;
      }
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
    m_syncIndexQueue.clear();
    m_currInput = &comp;
    m_BufInd = 0;
    m_currSyncDelay = 0;
    m_periodCount = 0;
  }

  void Oscilloscope::connectTrigger(SourceUnit& comp)
  {
    // connect
    disconnectTrigger();
    m_syncIndexQueue.clear();
    m_currTriggerSrc = &comp;
    m_BufInd = 0;
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
      if (m_isActive)
      {
        m_config->outputbuf.resize(m_config->defaultBufSize);
        m_inputBuffer.resize(m_config->defaultBufSize);
      }
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
    if (m_displayPeriods + d > 0)
    {
      WDL_MutexLock lock(&m_mutex);
      m_displayPeriods = m_displayPeriods + change;
      m_syncIndexQueue.clear();
    }
  }

  int Oscilloscope::getBufReadLength()
  {
    int bufReadLen;
    if (m_config->useAutoSync)
    {
      bufReadLen = getPeriod()*m_displayPeriods;
      if (bufReadLen > m_config->outputbuf.size())
        bufReadLen = m_config->outputbuf.size();
    }
    else
    {
      bufReadLen = m_config->outputbuf.size();
    }
    return bufReadLen;
  }

  bool Oscilloscope::Draw(IGraphics *pGraphics)
  {
    WDL_MutexLock lock(&m_mutex);

    pGraphics->DrawRect(&pt2color(palette[2] - getUnitv<4>(0)*40.0), &m_InnerRect);
    if (!m_isActive)
    {
      pGraphics->DrawRect(&pt2color(palette[2] - getUnitv<4>(0)*40.0), &mRECT);
      return false;
    }
    IText  txtstyle(12, &pt2color(palette[4]), "Helvetica", IText::kStyleNormal, IText::kAlignNear, 0, IText::kQualityClearType, &pt2color(palette[1]), &pt2color(palette[3]));
    IText txtstylecenter(txtstyle); txtstylecenter.mAlign = IText::kAlignCenter;
    NDPoint<4> bgcolor = palette[1] - getUnitv<4>(0)*127.0;
    pGraphics->FillIRect(&pt2color(bgcolor), &m_InnerRect);
    IColor cval = pt2color(getOnes<4>()*255.0 - bgcolor);
    /* zero line */
    double zero = toScreenY(0);
    pGraphics->DrawLine(&pt2color(palette[1]), mRECT.L, zero, mRECT.W() + mRECT.L, zero);

    char gridstr[128];
    snprintf(gridstr, 32, "%.4f", m_maxY);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(m_InnerRect.L - 10, m_InnerRect.T, m_InnerRect.L, m_InnerRect.T + 10));
    snprintf(gridstr, 32, "%.2f %s", 0.0, m_config->yunits.c_str());
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, zero - 10, m_InnerRect.L, zero + 10));
    snprintf(gridstr, 32, "%.4f", m_minY);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(m_InnerRect.L - 10, m_InnerRect.B - 10, m_InnerRect.L, m_InnerRect.B));
    sprintf(gridstr, "Displaying %d periods | Buffer size: %d (in) %d (out) | Input frequency: %.2f Hz", \
      m_displayPeriods, m_BufSize, m_config->outputbuf.size(), GetPlug()->GetSampleRate() / m_syncDelayEst * m_displayPeriods);
    pGraphics->DrawIText(&txtstyle, gridstr, &IRECT(mRECT.L, mRECT.T, mRECT.R, mRECT.B));
    sprintf(gridstr, m_config->xunits.c_str());
    pGraphics->DrawIText(&txtstylecenter, gridstr, &IRECT(mRECT.L, m_InnerRect.B + 10, mRECT.R, mRECT.B));
    double x1, y1;
    double x2, y2;

    if (m_inputBuffer.empty())
      return false;
    m_config->doTransform(mPlug, m_inputBuffer);
    if (m_config->argmin > 0 && m_config->argmax > 0)
    {
      double minY = m_config->outputbuf[m_config->argmin];
      double maxY = m_config->outputbuf[m_config->argmax];
      if (minY < m_minY) m_minY = minY;
      if (maxY > m_maxY) m_maxY = maxY;
    }

    int bufreadlen = getBufReadLength();
    int numxticks = 10;
    double xtickdist = m_InnerRect.W() / (double)numxticks;
    int lastxtick = 0;
    for (int j = 1; j < bufreadlen; j += 1, bufreadlen = getBufReadLength())
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