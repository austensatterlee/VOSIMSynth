#include "UIButton.h"
#include "Theme.h"
#include <array>

namespace syn
{
	UIButton::UIButton(VOSIMWindow* a_window, const string& caption, int icon)
		: UIComponent(a_window), mCaption(caption), mIcon(icon),
		  mIconPosition(IconPosition::LeftCentered), mPushed(false),
		  mFlags(NormalButton), mBackgroundColor(Color(0, 0)),
		  mTextColor(Color(0, 0)), mEnabled(true) {
		m_fontSize = theme()->mButtonFontSize;
	}

	UIComponent* UIButton::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
		UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);

		if (mEnabled) {
			bool pushedBackup = mPushed;
			if (mFlags & RadioButton) {
				if (mButtonGroup.empty()) {
					for (auto child : parent()->children()) {
						UIButton* b = dynamic_cast<UIButton *>(child.get());
						if (b != this && b && (b->flags() & RadioButton) && b->mPushed) {
							b->mPushed = false;
							if (b->mChangeCallback)
								b->mChangeCallback(false);
						}
					}
				} else {
					for (auto b : mButtonGroup) {
						if (b != this && (b->flags() & RadioButton) && b->mPushed) {
							b->mPushed = false;
							if (b->mChangeCallback)
								b->mChangeCallback(false);
						}
					}
				}
			}
			if (mFlags & PopupButton) {
				for (auto child : parent()->children()) {
					UIButton* b = dynamic_cast<UIButton *>(child.get());
					if (b != this && b && (b->flags() & PopupButton) && b->mPushed) {
						b->mPushed = false;
						if (b->mChangeCallback)
							b->mChangeCallback(false);
					}
				}
			}
			if (mFlags & ToggleButton)
				mPushed = !mPushed;
			else
				mPushed = true;

			if (pushedBackup != mPushed && mChangeCallback)
				mChangeCallback(mPushed);
			return this;
		}
		return nullptr;
	}

	bool UIButton::onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
		shared_ptr<UIComponent> self = m_parent->getChild(this);
		if (mEnabled) {
			bool pushedBackup = mPushed;
			if (mPushed) {
				if (contains(a_relCursor) && mCallback)
					mCallback();
				if (mFlags & NormalButton)
					mPushed = false;
			}
			if (pushedBackup != mPushed && mChangeCallback)
				mChangeCallback(mPushed);
			return true;
		}
		return false;
	}

	void UIButton::draw(NVGcontext* ctx) {
		NVGcolor gradTop = theme()->mButtonGradientTopFocused;
		NVGcolor gradBot = theme()->mButtonGradientBotUnfocused;

		if (mPushed) {
			gradTop = theme()->mButtonGradientTopPushed;
			gradBot = theme()->mButtonGradientBotPushed;
		} else if (m_hovered && mEnabled) {
			gradTop = theme()->mButtonGradientTopFocused;
			gradBot = theme()->mButtonGradientBotFocused;
		}

		nvgBeginPath(ctx);

		nvgRoundedRect(ctx, 1, 1.0f, size().x() - 2,
		               size().y() - 2, theme()->mButtonCornerRadius - 1);

		if (mBackgroundColor.w() != 0) {
			nvgFillColor(ctx, Color(mBackgroundColor.head<3>(), 1.f));
			nvgFill(ctx);
			if (mPushed) {
				gradTop.a = gradBot.a = 0.8f;
			} else {
				double v = 1 - mBackgroundColor.w();
				gradTop.a = gradBot.a = mEnabled ? v : v * .5f + .5f;
			}
		}

		NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0,
		                                0 + size().y(), gradTop, gradBot);

		nvgFillPaint(ctx, bg);
		nvgFill(ctx);

		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, 0 + 0.5f, 0 + (mPushed ? 0.5f : 1.5f), size().x() - 1,
		               size().y() - 1 - (mPushed ? 0.0f : 1.0f), theme()->mButtonCornerRadius);
		nvgStrokeColor(ctx, theme()->mBorderLight);
		nvgStroke(ctx);

		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, 0 + 0.5f, 0 + 0.5f, size().x() - 1,
		               size().y() - 2, theme()->mButtonCornerRadius);
		nvgStrokeColor(ctx, theme()->mBorderDark);
		nvgStroke(ctx);

		int fontSize = m_fontSize;
		nvgFontSize(ctx, fontSize);
		nvgFontFace(ctx, "sans-bold");
		float tw = nvgTextBounds(ctx, 0, 0, mCaption.c_str(), nullptr, nullptr);

		Vector2f center = size().cast<float>() * 0.5f;
		Vector2f textPos(center.x() - tw * 0.5f, center.y() - 1);
		NVGcolor textColor =
				mTextColor.w() == 0 ? theme()->mTextColor : mTextColor;
		if (!mEnabled)
			textColor = theme()->mDisabledTextColor;

		if (mIcon) {
			auto icon = utf8(mIcon);

			float iw, ih = fontSize;
			if (nvgIsFontIcon(mIcon)) {
				ih *= 1.5f;
				nvgFontSize(ctx, ih);
				nvgFontFace(ctx, "icons");
				iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
			} else {
				int w, h;
				ih *= 0.9f;
				nvgImageSize(ctx, mIcon, &w, &h);
				iw = w * ih / h;
			}
			if (mCaption != "")
				iw += size().y() * 0.15f;
			nvgFillColor(ctx, textColor);
			nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
			Vector2f iconPos = center;
			iconPos.y() -= 1;

			if (mIconPosition == IconPosition::LeftCentered) {
				iconPos.x() -= (tw + iw) * 0.5f;
				textPos.x() += iw * 0.5f;
			} else if (mIconPosition == IconPosition::RightCentered) {
				textPos.x() -= iw * 0.5f;
				iconPos.x() += tw * 0.5f;
			} else if (mIconPosition == IconPosition::Left) {
				iconPos.x() = 0 + 8;
			} else if (mIconPosition == IconPosition::Right) {
				iconPos.x() = 0 + size().x() - iw - 8;
			}

			if (nvgIsFontIcon(mIcon)) {
				nvgText(ctx, iconPos.x(), iconPos.y() + 1, icon.data(), nullptr);
			} else {
				NVGpaint imgPaint = nvgImagePattern(ctx,
				                                    iconPos.x(), iconPos.y() - ih / 2, iw, ih, 0, mIcon, mEnabled ? 0.5f : 0.25f);

				nvgFillPaint(ctx, imgPaint);
				nvgFill(ctx);
			}
		}

		nvgFontSize(ctx, fontSize);
		nvgFontFace(ctx, "sans-bold");
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
		nvgFillColor(ctx, theme()->mTextColorShadow);
		nvgText(ctx, textPos.x(), textPos.y(), mCaption.c_str(), nullptr);
		nvgFillColor(ctx, textColor);
		nvgText(ctx, textPos.x(), textPos.y() + 1, mCaption.c_str(), nullptr);
	}
}
