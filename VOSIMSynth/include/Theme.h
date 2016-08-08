#ifndef __THEME__
#define __THEME__
#include "UI.h"
#include <nanovg.h>

namespace synui
{
	__declspec(align(16)) struct Theme
	{
		Theme(NVGcontext* ctx);

		/* Fonts */
		int mFontLight;
		int mFontNormal;
		int mFontBold;
		int mFontIcons;
		int mFontMono;

		/* Spacing-related parameters */
		int mStandardFontSize;
		int mButtonFontSize;
		int mTextBoxFontSize;
		int mWindowCornerRadius;
		int mWindowHeaderHeight;
		int mWindowDropShadowSize;
		int mButtonCornerRadius;

		/* Tabs */
		int mTabBorderWidth;
		int mTabInnerMargin;
		int mTabMinButtonWidth;
		int mTabMaxButtonWidth;
		int mTabControlWidth;
		int mTabButtonHorizontalPadding;
		int mTabButtonVerticalPadding;

		/* Text slider parameters */
		int mTextSliderFontSize;

		/* Port parameters */
		int mPortFontSize;
		Color mInputPortBG;
		Color mInputPortHighlightedBG;
		Color mOutputPortBG;
		Color mOutputPortHighlightedBG;

		/* Wire parameters */
		Color mSelectedWireColor;
		Color mWireColor;
		Color mWireInnerShadowColor;

		/* Unit selector */
		int mUnitSelectorProtoFontSize;
		int mUnitSelectorGroupFontSize;

		/* Generic colors */
		Color mDropShadow;
		Color mTransparent;
		Color mBorderDark;
		Color mBorderLight;
		Color mBorderMedium;
		Color mTextColor;
		Color mDisabledTextColor;
		Color mTextColorShadow;
		Color mIconColor;

		/* Action colors */
		Color mHighlightedTextColor;

		/* Button colors */
		Color mButtonGradientTopFocused;
		Color mButtonGradientBotFocused;
		Color mButtonGradientTopUnfocused;
		Color mButtonGradientBotUnfocused;
		Color mButtonGradientTopPushed;
		Color mButtonGradientBotPushed;

		/* Window colors */
		Color mWindowFillUnfocused;
		Color mWindowFillFocused;
		Color mWindowTitleUnfocused;
		Color mWindowTitleFocused;

		Color mWindowHeaderGradientTop;
		Color mWindowHeaderGradientBot;
		Color mWindowHeaderSepTop;
		Color mWindowHeaderSepBot;

		Color mWindowPopup;
		Color mWindowPopupTransparent;
	};
}
#endif
