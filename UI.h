#ifndef __UI__
#define __UI__
#include "IControl.h"
#include "IParam.h"
#include "Unit.h"
#include "NDPoint.h"
#include "VoiceManager.h"
#include "UnitFactory.h"
#include "ITextSlider.h"
#include <vector>
#define SCREEN_PAD_L 25
#define X_PAD 10
#define Y_PAD 32
#define CELL_SIZE 32

using namespace std;
namespace syn
{
  class ColorPoint : public NDPoint<4>
  {
  protected:
    unsigned int m_word;
  public:
    ColorPoint(unsigned int word) :
      m_word(word),
      NDPoint<4>((double)(word >> 24), (double)((word >> 16) & 0xFF), (double)((word >> 8) & 0xFF), (double)(word & 0xFF))
    {}
    ColorPoint(const NDPoint<4>& pt) :
      NDPoint<4>(pt)
    {
    }
    operator IColor() const
    {
      return IColor((int)m_pvec[0], (int)m_pvec[1], (int)m_pvec[2], (int)m_pvec[3]);
    }
  };

  const vector<ColorPoint> palette = {
    {0xFF06070E},
    {0xFF49524A},
    {0xFF94A187},
    {0xFFC5AFA0},
    {0xFFE9BCB7}
  };

  template<class U>
  void attachKnob(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, string name, IBitmap *pBmp)
  {
    int x = (CELL_SIZE + X_PAD)*(c)+X_PAD + SCREEN_PAD_L;
    int y = (CELL_SIZE + Y_PAD)*(r)+Y_PAD;
    IText textprops(12, &COLOR_WHITE, "Helvetica");
    IParam* param = pPlug->GetParam(paramIdx);
    IRECT lbltextrect = IRECT(x, y + CELL_SIZE, x + CELL_SIZE, y + CELL_SIZE + Y_PAD / 2);
    IRECT valtextrect = IRECT(x, y + CELL_SIZE + Y_PAD / 2, x + CELL_SIZE, y + CELL_SIZE + Y_PAD);
    ICaptionControl* knobTextCtrl = new ICaptionControl(pPlug, lbltextrect, paramIdx, &textprops, false);
    ITextControl* knobLbl = new ITextControl(pPlug, valtextrect, &textprops, name.c_str());
    knobTextCtrl->DisablePrompt(false);
    U* knobCtrl = new U(pPlug, x, y, paramIdx, pBmp);

    pGraphics->AttachControl(knobCtrl);
    pGraphics->AttachControl(knobLbl);
    pGraphics->AttachControl(knobTextCtrl);
  }
}
#endif