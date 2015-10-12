#include "EnvelopeEditor.h"
#include <algorithm>
#include <cmath>
EnvelopeEditor::EnvelopeEditor(IPlugBase *pPlug, IRECT pR, int a_numpts) :
  IControl(pPlug, pR),
  m_points(a_numpts)
{
  const NDPoint<2>& xunitv = getUnitv<2>(0) / (m_points.size() + 1);
  const NDPoint<2>& yunitv = getUnitv<2>(1) * 0.5;
  for (int i = 0; i < m_points.size(); i++)
  {
    m_points[i] = xunitv*(i + 1) + yunitv;
  }
  m_ltpt = NDPoint<2>((double)pR.L, (double)pR.T);
  m_whpt = NDPoint<2>((double)pR.W(), (double)pR.H());
  m_lastSelectedPt = &m_points[0];
}

EnvelopeEditor::~EnvelopeEditor()
{}

NDPoint<2>& EnvelopeEditor::getPoint(double a_screenx, double a_screeny)
{
  NDPoint<2> mousept(a_screenx, a_screeny);
  mousept = toModel(mousept);

  double min_dist = -1;
  NDPoint<2>* min_pt;
  for (auto it = m_points.begin(); it != m_points.end(); it++)
  {
    double curr_dist = std::abs(((*it)-mousept)[0]);
    if (curr_dist < min_dist || min_dist == -1)
    {
      min_dist = curr_dist;
      min_pt = &(*it);
    }
  }
  return *min_pt;
}

NDPoint<2>& EnvelopeEditor::getPoint(int index)
{
  return m_points[index];
}

NDPoint<2> EnvelopeEditor::toScreen(const NDPoint<2>& a_modelpt) const
{
  return a_modelpt*m_whpt + m_ltpt;
}

NDPoint<2> EnvelopeEditor::toModel(const NDPoint<2>& a_screenpt) const
{
  return (a_screenpt - m_ltpt) / m_whpt;
}


void EnvelopeEditor::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
{
  m_lastSelectedPt = &getPoint(x, y);
  NDPoint<2> mouse_pt((double)x,(double)y);
  mouse_pt = toModel(mouse_pt);
  *m_lastSelectedPt = NDPoint<2>(mouse_pt);
  m_lastSelectedPt->clamp(getZeros<2>(), getOnes<2>());
  IControl::SetDirty();
}

void EnvelopeEditor::OnMouseUp(int x, int y, int dX, int dY, IMouseMod * pMod)
{}

void EnvelopeEditor::OnMouseOver(int x, int y, IMouseMod * pMod)
{
}

bool EnvelopeEditor::Draw(IGraphics* pGraphics)
{
  pGraphics->HandleMouseOver(true);
  IColor fgcolor(255, 255, 255, 255);
  IColor hicolor(255, 255, 0, 0);
  IColor gridcolor(150, 255, 25, 25);
  IColor bgcolor(150, 25, 25, 255);
  IText  txtstyle(&gridcolor);
  pGraphics->DrawRect(&bgcolor, &mRECT);
  NDPoint<2> screenpt1, screenpt2;
  std::sort(m_points.begin(), m_points.end(), [](const NDPoint<2>& lhs, const NDPoint<2>& rhs){return lhs[0]<rhs[0];});
  for (int i = 1; i < m_points.size(); i++)
  {
    double ptsize = 5;
    screenpt1 = toScreen(m_points[i-1]);
    screenpt2 = toScreen(m_points[i]);
    
    if (&m_points[i] == m_lastSelectedPt)
    {
      pGraphics->DrawLine(&hicolor, screenpt1[0], screenpt1[1], screenpt2[0], screenpt2[1], 0, true);
    }
    else
    {
      pGraphics->DrawLine(&fgcolor, screenpt1[0], screenpt1[1], screenpt2[0], screenpt2[1], 0, true);
    }
  }
  return true;
}

bool EnvelopeEditor::IsDirty()
{
  return mDirty;
}
