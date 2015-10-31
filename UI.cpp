#include "UI.h"
#include "UnitParameter.h"
#define LBL_H 10
#define X_PAD 15
#define Y_PAD 10
namespace syn
{
  void attachKnob(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, string name, IBitmap *pBmp)
  {
    int x = (pBmp->W + X_PAD)*(c)+X_PAD;
    int y = (pBmp->frameHeight() + LBL_H + Y_PAD)*(r)+Y_PAD;
    ITextControl* knobLabel;
    IText textprops(10, &COLOR_YELLOW, "Arial");
    IRECT textrect = IRECT(x, y + pBmp->frameHeight(), x + pBmp->W, y + pBmp->frameHeight() + LBL_H);
    knobLabel = new ITextControl(pPlug, textrect, &textprops, name.c_str());

    pGraphics->AttachControl(new IKnobMultiControl(pPlug, x, y, paramIdx, pBmp));
    pGraphics->AttachControl(knobLabel);
  }

  void attachSwitch(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, string name, IBitmap *pBmp)
  {
    int x = (pBmp->W + X_PAD)*(c)+X_PAD;
    int y = (pBmp->frameHeight() + LBL_H + Y_PAD)*(r)+Y_PAD;
    ITextControl* knobLabel;
    IText textprops(10, &COLOR_YELLOW, "Arial");
    IRECT textrect = IRECT(x, y + pBmp->frameHeight(), x + pBmp->W, y + pBmp->frameHeight() + LBL_H);
    knobLabel = new ITextControl(pPlug, textrect, &textprops, name.c_str());

    pGraphics->AttachControl(new ISwitchControl(pPlug, x, y, paramIdx, pBmp));
    pGraphics->AttachControl(knobLabel);
  }
}