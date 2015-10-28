#ifndef __OSCILLOSCOPE__
#define __OSCILLOSCOPE__
#include "IControl.h"
#include "Unit.h"
#include "SourceUnit.h"
#include "GallantSignal.h"
#include <vector>

using std::vector;
using Gallant::Signal1;

namespace syn
{
  class Oscilloscope : public IControl
  {
    typedef void (TransformFunc)(const vector<double>&, vector<double>&, double&, double&);
  protected:
    vector<double> m_inputBuffer;
    vector<double> m_outputBuffer;
    int m_currSyncDelay;
    double m_syncDelayEst;
    int m_periodCount;
    int m_BufInd;
    int m_BufSize;
    int m_displayPeriods;
    double m_minY, m_maxY;
    int m_Padding;
    bool m_isActive;
    IRECT m_InnerRect;
    Unit* m_currInput = nullptr;
    SourceUnit* m_currTriggerSrc = nullptr;
    TransformFunc* m_transformFunc = nullptr;

    double toScreenX(double val)
    {
      return val*m_InnerRect.W() + m_InnerRect.L;
    };
    double toScreenY(double val)
    {
      return -(val - m_minY) / (m_maxY - m_minY)*m_InnerRect.H() + m_InnerRect.B;
    }
    void setBufSize(int s);
  public:
    Oscilloscope(IPlugBase *pPlug, IRECT pR, int size = 1);
    ~Oscilloscope() {};

    bool IsDirty();
    void setPeriod(int nsamp);
    int getPeriod() { return m_BufSize / m_displayPeriods; };
    void sync(); //!< automatically called if a trigger source is set
    void input(double y); //!< automatically called if an input source is set
    void setTransformFunc(TransformFunc* f) { m_transformFunc = f; };
    void disconnectInput();
    void disconnectTrigger(SourceUnit& srccomp);
    void disconnectInput(Unit& srccomp);
    void disconnectTrigger();
    void connectInput(Unit& comp);
    void connectTrigger(SourceUnit& comp);
    void OnMouseUp(int x, int y, IMouseMod* pMod);
    void OnMouseWheel(int x, int y, IMouseMod* pMod, int d);
    bool Draw(IGraphics *pGraphics);

    /**
    * Computes the DFT for real inputbuf and stores the magnitude spectrum (dB) in outputbuf.
    * Can be passed to an Oscilloscope's Oscilloscope::setTransformFunc method.
    * \param inputbuf input vector
    * \param outputbuf output vector ( resized automatically to inputbuf.size()/2+1 )
    * \param minout the minimum magnitude will be stored here
    * \param maxout the maximum magnitude will be stored here
    */
    static void magnitudeTransform(const std::vector<double>& inputbuf, std::vector<double>& outputbuf, double& minout, double& maxout);
    static void inverseTransform(const std::vector<double>& inputbuf, std::vector<double>& outputbuf, double& minout, double& maxout);
    static void passthruTransform(const std::vector<double>& inputbuf, std::vector<double>& outputbuf, double& minout, double& maxout);
  };
}
#endif