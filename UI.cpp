#include "UI.h"
#define LBL_H 10
#define X_PAD 10

UI::UI()
{
}


UI::~UI()
{
}

void attachKnob(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, IBitmap *pBmp) {
	int x = (pBmp->W+X_PAD)*(c)+X_PAD;
	int y = (pBmp->frameHeight() + LBL_H)*(r);
	ITextControl* knobLabel;
	IText textprops(12, &COLOR_YELLOW, "Arial", IText::kStyleItalic);
	IRECT textrect = IRECT(x, y + pBmp->frameHeight(), x + pBmp->W, y + pBmp->frameHeight() + LBL_H);
	knobLabel = new ITextControl(pPlug, textrect, &textprops, pPlug->GetParam(paramIdx)->GetLabelForHost());

	pGraphics->AttachControl(new IKnobMultiControl(pPlug, x, y, paramIdx, pBmp));
	pGraphics->AttachControl(knobLabel);
}