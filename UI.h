#ifndef __UI__
#define __UI__
#include "IControl.h"
#include "IParam.h"
#include "Unit.h"
#include <vector>
#include <list>

using namespace std;
namespace syn
{
  class UI
  {
  private:

  public:
    UI() {};
    ~UI() {};
  };

  class Panel
  {
  private:
    vector<double> points;
  public:
    Panel() {};
    ~Panel() {};
  };

  void attachKnob(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, string name, IBitmap *pBmp);
  void attachSwitch(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, string name, IBitmap *pBmp);
}
#endif