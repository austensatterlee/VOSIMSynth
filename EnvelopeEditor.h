#ifndef __ENVELOPEEDITOR__
#define __ENVELOPEEDITOR__

#include "IControl.h"
#include "NDPoint.h"
#include <vector>

using std::vector;
namespace syn
{
  class EnvelopeEditor :
    public IControl
  {
  protected:
    vector<NDPoint<2> > m_points;
    NDPoint<2> m_ltpt; // left, top coords
    NDPoint<2> m_whpt; // width, height
    NDPoint<2>* m_lastSelectedPt;
    NDPoint<2>& getPoint(double x, double y);
    NDPoint<2>& getPoint(int index);
    NDPoint<2> toScreen(const NDPoint<2>& a_pt) const;
    NDPoint<2> toModel(const NDPoint<2>& a_pt) const;
  public:
    EnvelopeEditor(IPlugBase *pPlug, IRECT pR, int a_numpts = 1);
    virtual ~EnvelopeEditor();
    void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod);
    void OnMouseUp(int x, int y, int dX, int dY, IMouseMod* pMod);
    void OnMouseOver(int x, int y, IMouseMod* pMod);
    bool Draw(IGraphics* pGraphics);
    bool IsDirty();
  };
}

#endif // __ENVELOPEEDITOR__
