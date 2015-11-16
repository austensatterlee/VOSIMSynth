#ifndef __OSCILLOSCOPE__
#define __OSCILLOSCOPE__
#include "IControl.h"
#include "Unit.h"
#include "SourceUnit.h"
#include "GallantSignal.h"
#include <vector>
#include <deque>
#include <array>


using std::vector;
using std::deque;
using std::array;
using Gallant::Signal1;

namespace syn
{
 
  struct OscilloscopeConfig;
  typedef void (TransformFunc)(OscilloscopeConfig& oscconfig, IPlugBase* pPlug, const vector<double>& process);
  struct OscilloscopeConfig
  {
    const string name;
    TransformFunc* transform;
    const bool useAutoSync;
    const int defaultBufSize;
    string xunits;
    string yunits;
    int argmin,argmax;
    vector<double> xaxisticks;
    vector<string> xaxislbls;
    vector<double> yaxisticks;
    vector<string> yaxislbls;    
    vector<double> outputbuf;
    void doTransform(IPlugBase* pPlug, const vector<double>& process)
    {
      transform(*this,pPlug,process);
    }
  };

  class Oscilloscope : public IControl
  {
  private:
    WDL_Mutex m_mutex;
  protected:
    IRECT m_InnerRect;
    SourceUnit* m_currTriggerSrc = nullptr;
    Unit* m_currInput = nullptr;
    bool m_isActive;
    double m_minY, m_maxY;
    int m_Padding;
    int m_displayIndex;
    double m_syncDelayEst;
    int m_currSyncDelay;
    int m_periodCount;
    int m_displayPeriods;
    int m_BufInd;
    int m_BufSize;
    OscilloscopeConfig* m_config;
    vector<double> m_inputRingBuffer, m_inputBuffer;
    IPopupMenu m_menu;
    double toScreenX(double val)
    {
      int bufreadlength = getBufReadLength();
      double normalxval = (val - m_config->xaxisticks.front()) / (m_config->xaxisticks.back() - m_config->xaxisticks.front());
      return  normalxval*m_InnerRect.W() + m_InnerRect.L;
    };
    double toScreenY(double val)
    {
      double normalyval = (val - m_minY) / (m_maxY - m_minY);
      return m_InnerRect.B - m_InnerRect.H()*normalyval;
    }
    void setBufSize(int s);
    int getBufReadLength();
  public:
    Oscilloscope(IPlugBase *pPlug, IRECT pR);
    ~Oscilloscope() {};

    bool Draw(IGraphics *pGraphics);
    bool IsDirty() { return m_isActive && mDirty; }
    int getPeriod() { return m_BufSize/m_displayPeriods; }
    void OnMouseWheel(int x, int y, IMouseMod* pMod, int d);
    void connectInput(Unit& comp);
    void connectTrigger(SourceUnit& comp);
    void OnMouseUp(int x, int y, IMouseMod* pMod);
    void OnMouseDown(int x, int y, IMouseMod* pMod);
    void disconnectInput();
    void disconnectInput(Unit& srccomp);
    void disconnectTrigger();
    void disconnectTrigger(SourceUnit& srccomp);
    void process();
    void setPeriod(int nsamp) {
      while (m_displayPeriods>1 && m_displayPeriods*nsamp > 16384)
      {
        m_displayPeriods-=1;
      }
      setBufSize(m_displayPeriods*nsamp); 
    }
    void setConfig(OscilloscopeConfig* config);
  };

  /**
  * Computes the DFT for real inputbuf and stores the magnitude spectrum (dB) in outputbuf.
  */
  TransformFunc magnitudeTransform;
  TransformFunc inverseTransform;
  TransformFunc passthruTransform;
  static array<OscilloscopeConfig,2> OSCILLOSCOPE_CONFIGS = {{
    { "Spectral Magnitude", magnitudeTransform, false, 256, "Hz", "dB" },
    { "Time Domain", passthruTransform, true, 1, "seconds", "" }
  }};
}
#endif