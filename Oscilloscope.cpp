#include "Oscilloscope.h"
#include "SourceUnit.h"
#include "UI.h"

namespace syn
{
  Oscilloscope::Oscilloscope(IPlugBase * pPlug, IRECT pR, VoiceManager* vm) :
    IControl(pPlug, pR),
    m_periodCount(0),
    m_BufInd(0), 
    m_BufSize(0),
    m_minY(0),
    m_maxY(1),
    m_Padding(25), 
    m_displayIndex(0),
    m_displayPeriods(1),
    m_isActive(false),
    m_currSyncDelay(0),
    m_syncDelayEst(1),
    m_vm(vm),
    m_srcUnit_id(-1),
    m_triggerUnit_id(-1)
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
    if (!m_isActive) return;
    const Unit* currInput = getSourceUnit();
    const SourceUnit* currTrigger = getTriggerUnit();
    if(currInput==nullptr || currTrigger==nullptr) return;
    const vector<double>& srcbuffer = currInput->getLastOutputBuffer();
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
        (m_currSyncDelay > currTrigger->getSamplesPerPeriod())
        )
      {
        int correctPeriod;
        if (m_config->useAutoSync)
        {
          correctPeriod = currTrigger->getSamplesPerPeriod();
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
          for (int j = m_inputBuffer.size() - 1; j >= 0; j--)
          {
            if (inputBufInd < 0) inputBufInd = m_inputRingBuffer.size() - 1;
            m_inputBuffer[j] = m_inputRingBuffer[inputBufInd];
            inputBufInd--;
          }
          m_mutex.Leave();
          SetDirty();
        }
        m_syncDelayEst += 0.1*(double(m_currSyncDelay) - m_syncDelayEst);
        m_currSyncDelay = 0;
      }
    }
  }

  void Oscilloscope::setPeriod(int nsamp)
  {
    while (m_displayPeriods > 1 && m_displayPeriods*nsamp > 16384)
    {
      m_displayPeriods -= 1;
    }
    setBufSize(m_displayPeriods*nsamp);
  }

  double Oscilloscope::toScreenX(double val) const
  {
    double normalxval = (val - m_config->xaxisticks.front()) / (m_config->xaxisticks.back() - m_config->xaxisticks.front());
    return  normalxval*m_InnerRect.W() + m_InnerRect.L;
  }

  double Oscilloscope::toScreenY(double val) const
  {
    double normalyval = (val - m_minY) / (m_maxY - m_minY);
    return m_InnerRect.B - m_InnerRect.H()*normalyval;
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

  void Oscilloscope::connectInput(int srcUnit_id)
  {
    m_srcUnit_id = srcUnit_id;
    m_currSyncDelay = 0;
    m_periodCount = 0;
  }

  void Oscilloscope::connectTrigger(int triggerUnit_id)
  {
    m_triggerUnit_id = triggerUnit_id;
    m_currSyncDelay = 0;
    m_periodCount = 0;
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

  int Oscilloscope::getBufReadLength() const
  {
    int bufReadLen = getPeriod()*m_displayPeriods;
    if (bufReadLen >= m_config->outputbuf.size())
      bufReadLen = m_config->outputbuf.size() - 1;
    return bufReadLen;
  }

  const SourceUnit* Oscilloscope::getTriggerUnit() const
  {
    SourceUnit* trigger_unit = nullptr;
    if(m_vm->getNewestVoice()->hasUnit(m_triggerUnit_id))
    {
      trigger_unit = &m_vm->getNewestVoice()->getSourceUnit(m_triggerUnit_id);
    }
    return trigger_unit;
  }

  const Unit* Oscilloscope::getSourceUnit() const
  {
    Unit* source_unit = nullptr;
    if (m_vm->getNewestVoice()->hasUnit(m_srcUnit_id))
    {
      source_unit = &m_vm->getNewestVoice()->getUnit(m_srcUnit_id);
    }
    return source_unit;
  }

  bool Oscilloscope::isConnected() const
  {
    bool is_connected = true;
    is_connected &= m_vm->getNewestVoice()->hasUnit(m_srcUnit_id);
    is_connected &= m_vm->getNewestVoice()->hasUnit(m_triggerUnit_id);
    return is_connected;
  }

  bool Oscilloscope::Draw(IGraphics *pGraphics)
  {
    // Local color palette
    IColor outlinecolor = ColorPoint(globalPalette[2] - getUnitv<4>(0)*40.0);
    IColor bgcolor = ColorPoint(globalPalette[1] - getUnitv<4>(0)*127.0);
    IColor axescolor = ColorPoint(globalPalette[1]);
    IColor errorcolor_text{ 255,255,0,0 };
    IColor normalcolor_text{255,255,255,255};
    // Local text palette
    IText errortxtstyle(12, &errorcolor_text, "Helvetica", IText::kStyleNormal, IText::kAlignNear, 0, IText::kQualityClearType);
    IText txtstyle(12, &normalcolor_text, "Helvetica", IText::kStyleNormal, IText::kAlignNear, 0, IText::kQualityClearType);
    IText txtstylecenter(12, &normalcolor_text, "Helvetica", IText::kStyleNormal, IText::kAlignCenter, 0, IText::kQualityClearType);

    // Draw outline
    pGraphics->DrawRect(&outlinecolor, &mRECT);
    if (!m_isActive || !isConnected())
    {
      if (!isConnected())
      {
        pGraphics->DrawIText(&errortxtstyle, "No input!", &m_InnerRect);
      }
      return false;
    }
    // Fill background
    pGraphics->FillIRect(&bgcolor, &m_InnerRect);

    // Draw y=0 line
    int zero = toScreenY(0);
    pGraphics->DrawLine(&axescolor, mRECT.L, zero, mRECT.W() + mRECT.L, zero);

    char gridstr[128];
    // Draw axes tick labels
    IRECT yticklabel_max{m_InnerRect.L - 10, m_InnerRect.T, m_InnerRect.L, m_InnerRect.T + 10};
    IRECT yticklabel_zero{mRECT.L, zero - 10, m_InnerRect.L, zero + 10};
    IRECT yticklabel_min{m_InnerRect.L - 10, m_InnerRect.B - 10, m_InnerRect.L, m_InnerRect.B};
    IRECT xunitslabel{ mRECT.L, m_InnerRect.B + 10, mRECT.R, mRECT.B };
    snprintf(gridstr, 32, "%.4f", m_maxY);
    pGraphics->DrawIText(&txtstyle, gridstr, &yticklabel_max);
    snprintf(gridstr, 32, "%.2f %s", 0.0, m_config->yunits.c_str());
    pGraphics->DrawIText(&txtstyle, gridstr, &yticklabel_zero);
    snprintf(gridstr, 32, "%.4f", m_minY);
    pGraphics->DrawIText(&txtstyle, gridstr, &yticklabel_min);
    sprintf(gridstr, m_config->xunits.c_str());
    pGraphics->DrawIText(&txtstylecenter, gridstr, &xunitslabel);

    // Draw window information text
    int bufsize = m_config->outputbuf.size();
    double estfreq = GetPlug()->GetSampleRate() / m_syncDelayEst;
    double publishedfreq = GetPlug()->GetSampleRate() / getPeriod();
    sprintf(gridstr, "Displaying %d periods | Buffer size: %d (in) %d (out) | Input frequency: %.2f Hz (%.2f Hz)", \
      m_displayPeriods, m_BufSize, bufsize, estfreq, publishedfreq);
    pGraphics->DrawIText(&txtstyle, gridstr, &mRECT);
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
      m_minY = minY;
      m_maxY = maxY;
    }

    // Draw transform output
    int bufreadlen = getBufReadLength();
    int numxticks = 10;
    double xtickdist = m_InnerRect.W() / (double)numxticks; // min distance between xtick labels
    int lastxtick = 0;
    IColor curvecolor{255,255,255,255};
    for (int j = 1; j <= bufreadlen; j += 1)
    {
      x1 = toScreenX(m_config->xaxisticks[j - 1]);
      x2 = toScreenX(m_config->xaxisticks[j]);
      y1 = toScreenY(m_config->outputbuf[j - 1]);
      y2 = toScreenY(m_config->outputbuf[j]);
      curvecolor.A = 255;// - 25* bufreadlen /m_InnerRect.W();
      curvecolor.R *= (1 - (y2 - m_InnerRect.T) / m_InnerRect.H());
      curvecolor.G *= 1;
      curvecolor.B *= (1 - abs(y1 - y2) / m_InnerRect.H());
      pGraphics->DrawLine(&curvecolor, x1, y1, x2, y2, 0, true);
      // Draw x ticks
      if (j == 1 || x2 - lastxtick >= xtickdist)
      {
        IRECT xticklabel(x2, m_InnerRect.B, x2 + 10, m_InnerRect.B + 10);
        snprintf(gridstr, 128, "%s", m_config->xaxislbls[j].c_str());
        pGraphics->DrawIText(&txtstyle, gridstr, &xticklabel);
        lastxtick = x2;
      }
    }
    return true;
  }
}