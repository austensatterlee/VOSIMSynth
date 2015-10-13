#ifndef __OSCILLOSCOPE__
#define __OSCILLOSCOPE__
#include "IControl.h"
#include "Unit.h"
#include "SourceUnit.h"
#include <vector>

using std::vector;
namespace syn
{
  class Oscilloscope : public IControl
  {
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
    Unit* m_currInput = NULL;
    SourceUnit* m_currTriggerSrc = NULL;

    double toScreenX(double val)
    {
      return val*m_InnerRect.W() + m_InnerRect.L;
    };
    double toScreenY(double val)
    {
      return -(val - m_minY) / (m_maxY - m_minY)*m_InnerRect.H() + m_InnerRect.B;
    }
  public:
    Oscilloscope(IPlugBase *pPlug, IRECT pR, int size = 1);
    ~Oscilloscope() {};

    bool IsDirty();
    void setPeriod(int nsamp);
    int getPeriod(){return m_BufSize/m_displayPeriods;};
    void sync(); // ends the current capture window and displays the result
    void input(double y);
    void setBufSize(int s);
    void connectInput(Unit* comp);
    void connectTrigger(SourceUnit* comp);
    void OnMouseUp(int x, int y, IMouseMod* pMod);
    void OnMouseWheel(int x, int y, IMouseMod* pMod, int d);
    bool Draw(IGraphics *pGraphics);

  };
}
#endif