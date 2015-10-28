#include "EnvelopeEditor.h"
#include <algorithm>
#include <cmath>
namespace syn
{
  EnvelopeEditor::EnvelopeEditor(VOSIMSynth *pPlug, VoiceManager* vm, string envname, IRECT pR, const double maxTimeScale, const double minAmpScale, const double maxAmpScale) :
    IControl(pPlug, pR),
    m_VOSIMPlug(pPlug),
    m_timeScale(1.0),
    m_ampScale((maxAmpScale + minAmpScale) / 2.0),
    m_maxTimeScale(maxTimeScale),
    m_minAmpScale(minAmpScale),
    m_maxAmpScale(maxAmpScale),
    m_Padding(25),
    m_lastMouseMod(0),
    m_lastMouse(0, 0),
    m_isMouseDown(false),
    m_lastSelectedIdx(-1)
  {
    m_InnerRect = IRECT(pR.L + m_Padding, pR.T + m_Padding, pR.R - m_Padding, pR.B - m_Padding);
    m_ltpt = NDPoint<2>((double)m_InnerRect.L, (double)m_InnerRect.T);
    m_whpt = NDPoint<2>((double)m_InnerRect.W(), (double)m_InnerRect.H());

    setEnvelope(vm, envname);
  }

  EnvelopeEditor::~EnvelopeEditor()
  {}

  int EnvelopeEditor::getSelected(double a_screenx, double a_screeny)
  {
    NDPoint<2> mousept(a_screenx, a_screeny);
    mousept = toModel(mousept);

    double min_dist = -1;
    int min_pt_idx;
    for (int i = 0; i < m_points.size(); i++)
    {
      NDPoint<2>& pt = m_points[i];
      double curr_dist = mousept.distFrom(pt);
      if (curr_dist < min_dist || min_dist == -1)
      {
        min_dist = curr_dist;
        min_pt_idx = i;
      }
    }
    return min_pt_idx;
  }

  void EnvelopeEditor::insertPointFromScreen(const NDPoint<2>& screenpt)
  {
    int afterIndex = getSelected(screenpt[0], screenpt[1]);
    NDPoint<2> modelpt = toModel(screenpt);
    m_points.insert(m_points.begin() + afterIndex, modelpt);

    Envelope* env = (Envelope*)(&m_voiceManager->getProtoInstrument()->getUnit(m_targetEnvId));

    resyncEnvelope();
  }

  NDPoint<2>& EnvelopeEditor::getPoint(int index)
  {
    return m_points[index];
  }

  NDPoint<2> EnvelopeEditor::toScreen(const NDPoint<2>& a_modelpt) const
  {
    NDPoint<2> modelpt({ a_modelpt[0],1 - a_modelpt[1] });
    return (modelpt)*m_whpt + m_ltpt;
  }

  NDPoint<2> EnvelopeEditor::toModel(const NDPoint<2>& a_screenpt) const
  {
    NDPoint<2> modelpt = (a_screenpt - m_ltpt) / m_whpt;
    modelpt[0] = modelpt[0];
    modelpt[1] = (1 - modelpt[1]);
    return modelpt;
  }


  void EnvelopeEditor::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
  {
    NDPoint<2> mouse_pt((double)x, (double)y);
    mouse_pt.clamp(NDPoint<2>{(double)m_InnerRect.L, (double)m_InnerRect.T}, NDPoint<2>{(double)m_InnerRect.R, (double)m_InnerRect.B});

    // Only select a new point once per mouse click
    if (!m_isMouseDown)
    {
      m_lastSelectedIdx = getSelected(mouse_pt[0], mouse_pt[1]);
      m_isMouseDown = true;
    }
    if (m_lastSelectedIdx <= 0)
    {
      m_lastSelectedIdx = 1;
    }

    mouse_pt = toModel(mouse_pt);
    // Clamp point to stay between its neighbors
    NDPoint<2> bottomleftBound{ m_points[m_lastSelectedIdx - 1][0], 0.0 };
    NDPoint<2> toprightBound;
    if (m_lastSelectedIdx == m_points.size() - 1)
    {
      toprightBound = NDPoint<2>{ 1.0, 1.0 };
    }
    else
    {
      toprightBound = NDPoint<2>{ m_points[m_lastSelectedIdx + 1][0], 1.0 };
    }
    mouse_pt.clamp(bottomleftBound, toprightBound);
    m_lastMouse = mouse_pt;
    m_lastMouseMod = *pMod;

    NDPoint<2>& oldpoint = m_points[m_lastSelectedIdx];
    m_points[m_lastSelectedIdx] = m_lastMouse;

    // Check that moving the point doesn't push any other points off the grid before applying
    NDPoint<2> diff = (m_points[m_lastSelectedIdx] - oldpoint)*getUnitv<2>(0);
    if (pMod->S)
    {
      if ((m_points.back() + diff)[0] <= 1.0)
      {
        // Push all points to the right to maintain segment lengths
        for (int i = m_lastSelectedIdx + 1; i < m_points.size(); i++)
        {
          m_points[i] += diff;
        }
      }
      else
      {
        m_points[m_lastSelectedIdx][0] = oldpoint[0];
      }
    }
    resyncEnvelope();
  }

  void EnvelopeEditor::OnMouseUp(int x, int y, IMouseMod * pMod)
  {
    m_isMouseDown = false;
    m_lastSelectedIdx = -1;
  }

  void EnvelopeEditor::OnMouseDblClick(int x, int y, IMouseMod* pMod)
  {
    NDPoint<2> mouse_pt((double)x, (double)y);
    mouse_pt.clamp(NDPoint<2>{(double)m_InnerRect.L, (double)m_InnerRect.T}, NDPoint<2>{(double)m_InnerRect.R, (double)m_InnerRect.B});

    m_lastSelectedIdx = getSelected(mouse_pt[0], mouse_pt[1]);

    if (pMod->C)
    {
      if (m_lastSelectedIdx != m_VOSIMPlug->GetInstrParameter(m_targetEnvId, 0))
      {
        m_VOSIMPlug->SetInstrParameter(m_targetEnvId, 0, m_lastSelectedIdx);
      }
      else
      {
        m_VOSIMPlug->SetInstrParameter(m_targetEnvId, 0, -1);
      }
    }
    else
    {
      if (m_lastSelectedIdx != m_VOSIMPlug->GetInstrParameter(m_targetEnvId, 1))
      {
        m_VOSIMPlug->SetInstrParameter(m_targetEnvId, 1, m_lastSelectedIdx);
      }
      else
      {
        m_VOSIMPlug->SetInstrParameter(m_targetEnvId, 1, -1);
      }
    }
    resyncEnvelope();
  }

  void EnvelopeEditor::OnMouseWheel(int x, int y, IMouseMod* pMod, int d)
  {
    const double amp_mod_step = (m_maxAmpScale - m_minAmpScale) / 10.0;
    const double amp_mod_step_fine = amp_mod_step / 10.0;
    const double tc_mod_step = m_maxTimeScale / 10.0;
    const double tc_mod_step_fine = tc_mod_step / 10.0;
    double modamt;
    if (pMod->C)
    {
      if (pMod->S)
        modamt = d * amp_mod_step_fine;
      else
        modamt = d * amp_mod_step;
      m_ampScale += modamt;
      if (m_ampScale > m_maxAmpScale)
        m_ampScale = m_maxAmpScale;
      else if (m_ampScale < m_minAmpScale)
        m_ampScale = m_minAmpScale;
    }
    else
    {
      if (pMod->S)
        modamt = d * tc_mod_step_fine;
      else
        modamt = d * tc_mod_step;
      m_timeScale += modamt;
      if (m_timeScale > m_maxTimeScale)
        m_timeScale = m_maxTimeScale;
      while (m_timeScale <= 0)
        m_timeScale -= modamt;
    }
    resyncEnvelope();
  }

  bool EnvelopeEditor::Draw(IGraphics* pGraphics)
  {
    IColor fgcolor1(255, 200, 200, 200);
    IColor hicolor(220, 220, 250, 200);
    IColor gridcolor(200, 100, 100, 100);
    IColor bgcolor1(255, 50, 50, 75);
    IColor bgcolor2(255, 50, 50, 75);
    IColor fgcolor2(255, 255, 180, 180);
    IColor fgcolor3(255, 180, 180, 255);
    IText  fgtxtstyle(10, &fgcolor1, 0, IText::kStyleNormal, IText::kAlignNear);
    IText  bgtxtstyle(10, &gridcolor, 0, IText::kStyleItalic, IText::kAlignNear);
    NDPoint<2> screenpt1, screenpt2;
    Envelope* env = ((Envelope*)&m_voiceManager->getProtoInstrument()->getUnit(m_targetEnvId));

    char strbuffer[256];
    string envname = m_voiceManager->getProtoInstrument()->getUnit(m_targetEnvId).getName();
    sprintf(strbuffer, "%s", envname.c_str());
    pGraphics->DrawIText(&fgtxtstyle, strbuffer, &mRECT);
    sprintf(strbuffer, "Time scale: %.2f, Amp scale: %.2f", m_timeScale, m_ampScale);
    pGraphics->DrawIText(&fgtxtstyle, strbuffer, &IRECT(mRECT.L, mRECT.T + 10, mRECT.R, mRECT.B));
    double cumuPeriod = 0;
    double currPeriod = 0;
    IColor* currColor;

    pGraphics->DrawRect(&bgcolor1, &m_InnerRect);
    for (int i = 0; i < m_points.size(); i++)
    {
      double ptsize = 3;
      screenpt2 = toScreen(m_points[i]);
      if (i > 0)
      {
        screenpt1 = toScreen(m_points[i - 1]);
        pGraphics->DrawLine(&fgcolor1, screenpt1[0], screenpt1[1], screenpt2[0], screenpt2[1], 0, true);
      }
      if (i == m_lastSelectedIdx)
      {
        currColor = &hicolor;
      }
      else
      {
        if (i == env->readParam(0))
        { // check if point is a loop start
          currColor = &fgcolor3;
        }
        else if (i == env->readParam(1))
        { // check if point is a loop end
          currColor = &fgcolor2;
        }
        else
        {
          currColor = &bgcolor2;
        }
      }
      pGraphics->FillCircle(currColor, screenpt2[0], screenpt2[1], ptsize, 0, true);
      if(i>0){
        currPeriod = env->getPeriod(i-1);
        cumuPeriod += currPeriod;
        sprintf(strbuffer, "%.2f", currPeriod);
        pGraphics->DrawIText(&fgtxtstyle, strbuffer, &IRECT(screenpt2[0], mRECT.B - 10 - 10, mRECT.R, mRECT.B));
        sprintf(strbuffer, "%.2f", cumuPeriod);
        pGraphics->DrawIText(&bgtxtstyle, strbuffer, &IRECT(screenpt2[0], mRECT.B - 10, mRECT.R, mRECT.B));
        pGraphics->DrawLine(&gridcolor, screenpt2[0], mRECT.B - m_Padding, screenpt2[0], screenpt2[1], 0, true);
        if (i == env->readParam(0))
        {
          pGraphics->DrawIText(&fgtxtstyle, "S", &IRECT(screenpt2[0] - 15, screenpt2[1] - 10, mRECT.R, mRECT.B));
        }
        if (i == env->readParam(1))
        {
          pGraphics->DrawIText(&fgtxtstyle, "E", &IRECT(screenpt2[0] + 5, screenpt2[1] - 10, mRECT.R, mRECT.B));
        }
      }
    }
    return true;
  }

  void EnvelopeEditor::resyncPoints()
  {
    if (m_voiceManager && !m_isMouseDown)
    {
      Envelope* env = (Envelope*)(&m_voiceManager->getProtoInstrument()->getUnit(m_targetEnvId));
      m_points.resize(env->getNumSegments() + 1);
      m_points[0] = NDPoint<2>{ 0,env->getInitPoint() };
      double xtotal = 0;
      for (int i = 0; i < env->getNumSegments(); i++)
      {
        xtotal += m_VOSIMPlug->GetInstrParameter(m_targetEnvId, env->getPeriodId(i));
        double amp = m_VOSIMPlug->GetInstrParameter(m_targetEnvId, env->getPointId(i));
        m_points[i + 1] = NDPoint<2>{ xtotal , amp };
      }
      xtotal = std::ceil(xtotal);
      m_timeScale = xtotal;
      for (int i = 0; i < env->getNumSegments(); i++)
      {
        m_points[i + 1][0] /= m_timeScale;
      }
      SetDirty();
    }
  }

  void EnvelopeEditor::renormalizePoints()
  {
    double xtotal = 0;
    for (int i = 0; i < m_points.size(); i++)
    {
      xtotal += m_points[i][0];
    }
    for (int i = 0; i < m_points.size(); i++)
    {
      m_points[i][0] /= xtotal;
    }
  }

  bool EnvelopeEditor::IsDirty()
  {
    return true;
  }

  void EnvelopeEditor::setEnvelope(VoiceManager* vm, string targetEnvName)
  {
    m_voiceManager = vm;
    if (m_voiceManager)
    {
      m_targetEnvId = vm->getProtoInstrument()->getUnitId(targetEnvName);
      vector<string> paramnames = vm->getProtoInstrument()->getUnit(m_targetEnvId).getParameterNames();
      for (int i = 0; i < paramnames.size(); i++)
      {
        vm->getProtoInstrument()->getUnit(m_targetEnvId).getParam(paramnames[i]).setController(this);
      }
      resyncPoints();
      resyncEnvelope();
    }
  }

  void EnvelopeEditor::resyncEnvelope()
  {
    if (m_voiceManager)
    {
      Envelope* env = (Envelope*)(&m_voiceManager->getProtoInstrument()->getUnit(m_targetEnvId));

      for (int seg = 0; seg < m_points.size() - 1; seg++)
      {
        // set point
        //m_voiceManager->modifyParameter(m_targetEnvId, env->getPointId(seg), m_ampScale*m_points[seg + 1][1], SET);
        m_VOSIMPlug->SetInstrParameter(m_targetEnvId, env->getPointId(seg), m_ampScale*m_points[seg + 1][1]);
        // set period
        double newperiod = m_timeScale*(m_points[seg + 1][0] - m_points[seg][0]);
        //m_voiceManager->modifyParameter(m_targetEnvId, env->getPeriodId(seg), newperiod, SET);
        m_VOSIMPlug->SetInstrParameter(m_targetEnvId, env->getPeriodId(seg), newperiod);
      }
      SetDirty();
    }
  }

}