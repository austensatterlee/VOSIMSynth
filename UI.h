#pragma once
#include <cstdint>
#include "IControl.h"

class UI
{
private:

public:
	UI();
	~UI();
};

void attachKnob(IGraphics *pGraphics, IPlugBase *pPlug, uint8_t r, uint8_t c, int paramIdx, IBitmap *pBmp);