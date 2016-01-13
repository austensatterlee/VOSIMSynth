#ifndef __OSCILLOSCOPE__
#define __OSCILLOSCOPE__
#include "IControl.h"
#include "Unit.h"
#include "SourceUnit.h"
#include "VoiceManager.h"
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
  typedef void (TransformFunc)(OscilloscopeConfig& oscconfig, IPlugBase* pPlug, vector<double>& process);
  struct OscilloscopeConfig
  {
    const string name;
    TransformFunc* transform;
    const bool useAutoSync;
    const int defaultBufSize;
    string xunits;
    string yunits;
    int argmin, argmax;
    vector<double> xaxisticks;
    vector<string> xaxislbls;
    vector<double> yaxisticks;
    vector<string> yaxislbls;
    vector<double> outputbuf;
    void doTransform(IPlugBase* pPlug, vector<double>& process)
    {
      transform(*this, pPlug, process);
    }
  };

  class Oscilloscope : public IControl
  {
  private:
    WDL_Mutex m_mutex;
  protected:
    IRECT m_InnerRect;
    VoiceManager* m_vm;
    int m_srcUnit_id, m_triggerUnit_id;
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
    double toScreenX(double val) const;;
    double toScreenY(double val) const;
    void setBufSize(int s);
    int getBufReadLength() const;
    const SourceUnit* getTriggerUnit() const;
    const Unit* getSourceUnit() const;
    bool isConnected() const;
  public:
    Oscilloscope(IPlugBase *pPlug, IRECT pR, VoiceManager* vm);
    ~Oscilloscope() {}

    int getInputId() const {return m_srcUnit_id; };
    int getTriggerId() const { return m_triggerUnit_id; };

    virtual bool Draw(IGraphics *pGraphics) override;

	  bool IsDirty() override
    { return m_isActive && mDirty; }
    int getPeriod() const
    { return m_BufSize / m_displayPeriods; }

    void connectInput(int srcUnit_id);
    void connectTrigger(int triggerUnit_id);
    virtual void OnMouseWheel(int x, int y, IMouseMod* pMod, int d) override;
    virtual void OnMouseDown(int x, int y, IMouseMod* pMod) override;
    void process();
    void setPeriod(int nsamp);
    void setConfig(OscilloscopeConfig* config);
  };

  /**
  * Computes the DFT for real inputbuf and stores the magnitude spectrum (dB) in outputbuf.
  */
  TransformFunc magnitudeTransform;
  TransformFunc inverseTransform;
  TransformFunc passthruTransform;
  static array<OscilloscopeConfig, 2> OSCILLOSCOPE_CONFIGS = { {
    { "Spectral Magnitude", magnitudeTransform, false, 1024, "Hz", "dB" },
    { "Time Domain", passthruTransform, true, 1, "seconds", "" }
  } };
}
#endif