#include "Theme.h"
#include "vosimsynth_resources.h"

namespace synui
{
	Theme::Theme(NVGcontext* ctx) {
		mStandardFontSize = 14;
		mButtonFontSize = 14;
		mTextBoxFontSize = 14;
		mWindowCornerRadius = 2;
		mWindowHeaderHeight = 20;
		mWindowDropShadowSize = 4;
		mButtonCornerRadius = 2;

		mDropShadow = Color(0, 128);
		mTransparent = Color(0, 0);
		mBorderDark = Color(29, 255);
		mBorderLight = Color(92, 255);
		mBorderMedium = Color(35, 255);
		mTextColor = Color(255, 160);
		mDisabledTextColor = Color(255, 80);
		mTextColorShadow = Color(0, 160);
		mIconColor = mTextColor;

		mHighlightedTextColor = Color(255, 250);

		mButtonGradientTopFocused = Color(84, 255);
		mButtonGradientBotFocused = Color(78, 255);
		mButtonGradientTopUnfocused = Color(74, 255);
		mButtonGradientBotUnfocused = Color(58, 255);
		mButtonGradientTopPushed = Color(41, 255);
		mButtonGradientBotPushed = Color(29, 255);

		/* Window-related */
		mWindowFillUnfocused = Color(43, 220);
		mWindowFillFocused = Color(45, 250);
		mWindowTitleUnfocused = Color(220, 160);
		mWindowTitleFocused = Color(255, 190);

		mWindowHeaderGradientTop = mButtonGradientTopUnfocused;
		mWindowHeaderGradientBot = mButtonGradientBotUnfocused;
		mWindowHeaderSepTop = mBorderLight;
		mWindowHeaderSepBot = mBorderDark;

		mWindowPopup = Color(50, 255);
		mWindowPopupTransparent = Color(50, 0);

		/* Text slider */
		mTextSliderFontSize = 16;

		/* Port */
		mPortFontSize = 14;

		mInputPortBG = Color(Vector3f{ 0.55f , 0.775f, 1.f }, 0.32f);
		mInputPortHighlightedBG = Color(Vector3f{ 0.55f , 0.775f, 1.f }, 0.52f);

		mOutputPortBG = Color(Vector3f{ 0.93f , 0.76f, 0.66f }.normalized(), 0.32f);
		mOutputPortHighlightedBG = Color(Vector3f{ 0.93f , 0.76f, 0.66f }.normalized(), 0.52f);

		/* Wire parameters */
		mWireColor = Color(Vector3f(0.5f, 0.0f, 0.0f), 0.9f);
		mWireInnerShadowColor = Color(Vector3f(0.15f, 0.0f, 0.0f), 0.8f);
		mSelectedWireColor = Color(Vector3f(0.8f, 0.3f, 0.3f), 1.0f);

		/* Unit selector */
		mUnitSelectorProtoFontSize = 16;
		mUnitSelectorGroupFontSize = 18;

		/* Unit Control Panel */

		/* Circuit Panel */

		mFontLight = nvgCreateFontMem(ctx, "sans-light", roboto_light_ttf,
			roboto_light_ttf_size, 0);
		mFontNormal = nvgCreateFontMem(ctx, "sans", opensans_regular_ttf,
			opensans_regular_ttf_size, 0);
		mFontBold = nvgCreateFontMem(ctx, "sans-bold", roboto_black_ttf,
			roboto_black_ttf_size, 0);
		mFontIcons = nvgCreateFontMem(ctx, "icons", entypo_ttf,
			entypo_ttf_size, 0);
		mFontMono = nvgCreateFontMem(ctx, "mono", inconsolata_regular_ttf, inconsolata_regular_ttf_size, 0);
		if (mFontNormal == -1 || mFontBold == -1 || mFontIcons == -1)
			throw runtime_error("Could not load fonts!");
	}
};