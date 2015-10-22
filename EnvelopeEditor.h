#ifndef __ENVELOPEEDITOR__
#define __ENVELOPEEDITOR__

#include "VOSIMSynth.h"
#include "IControl.h"
#include "NDPoint.h"
#include "Envelope.h"
#include "VoiceManager.h"
#include <vector>

using std::vector;
namespace syn
{
  class EnvelopeEditor :
    public IControl
  {
  protected:
    VOSIMSynth* m_VOSIMPlug;
    double m_timeScale;
    double m_ampScale;
    double m_maxTimeScale;
    double m_minAmpScale, m_maxAmpScale;
    int m_Padding;
    IRECT m_InnerRect;
    vector<NDPoint<2> > m_points;
    NDPoint<2> m_ltpt; // left, top coords
    NDPoint<2> m_whpt; // width, height
    NDPoint<2> m_lastMouse;
    IMouseMod m_lastMouseMod;
    int m_lastSelectedIdx;
    VoiceManager* m_voiceManager;
    int m_targetEnvId;
    bool m_isMouseDown;
    int getSelected(double x, double y);
    NDPoint<2>& getPoint(int index);
    NDPoint<2> toScreen(const NDPoint<2>& a_pt) const;
    NDPoint<2> toModel(const NDPoint<2>& a_pt) const;
    /*!
    * \brief Re-syncs the display points to match the targeted envelope parameters.
    */
    void resyncPoints();
    /*!
     * \brief Normalizes the x-axis of each point to be between 0 and 1.
     */
    void renormalizePoints();
  public:
    EnvelopeEditor(VOSIMSynth *pPlug, IRECT pR, double maxTimeScale, double minAmpScale, double maxAmpScale, int numpoints = 2);
    virtual ~EnvelopeEditor();
    void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod);

    void OnMouseUp(int x, int y, IMouseMod * pMod);
    void OnMouseDblClick(int x, int y, IMouseMod* pMod);
    void OnMouseWheel(int x, int y, IMouseMod* pMod, int d);
    bool Draw(IGraphics* pGraphics);
    bool IsDirty();
    void setEnvelope(VoiceManager* vm, string targetEnvName);
    void setTimeScale(double timescale) { m_timeScale = timescale; resyncPoints(); };
    void resyncEnvelope();
  };
}

#endif // __ENVELOPEEDITOR__
