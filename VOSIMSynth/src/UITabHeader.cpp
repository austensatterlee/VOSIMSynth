#include "UITabHeader.h"
#include <Theme.h>
#include <numeric>
#include <array>

namespace synui {

	UITabHeader::TabButton::TabButton(UITabHeader &header, const std::string &label)
		: mHeader(&header), mLabel(label) { }

	Vector2i UITabHeader::TabButton::calcMinSize(NVGcontext *ctx) const {
		// No need to call nvg font related functions since this is done by the tab header implementation
		float bounds[4];
		int labelWidth = nvgTextBounds(ctx, 0, 0, mLabel.c_str(), nullptr, bounds);
		int buttonWidth = labelWidth + 2 * mHeader->theme()->mTabButtonHorizontalPadding;
		int buttonHeight = bounds[3] - bounds[1] + 2 * mHeader->theme()->mTabButtonVerticalPadding;
		return Vector2i(buttonWidth, buttonHeight);
	}

	void UITabHeader::TabButton::calculateVisibleString(NVGcontext *ctx) {
		// The size must have been set in by the enclosing tab header.
		NVGtextRow displayedText;
		nvgTextBreakLines(ctx, mLabel.c_str(), nullptr, mSize.x(), &displayedText, 1);

		// Check to see if the text need to be truncated.
		if (displayedText.next[0]) {
			auto truncatedWidth = nvgTextBounds(ctx, 0.0f, 0.0f,
				displayedText.start, displayedText.end, nullptr);
			auto dotsWidth = nvgTextBounds(ctx, 0.0f, 0.0f, dots, nullptr, nullptr);
			while ((truncatedWidth + dotsWidth + mHeader->theme()->mTabButtonHorizontalPadding) > mSize.x()
				&& displayedText.end != displayedText.start) {
				--displayedText.end;
				truncatedWidth = nvgTextBounds(ctx, 0.0f, 0.0f,
					displayedText.start, displayedText.end, nullptr);
			}

			// Remember the truncated width to know where to display the dots.
			mVisibleWidth = truncatedWidth;
			mVisibleText.last = displayedText.end;
		}
		else {
			mVisibleText.last = nullptr;
			mVisibleWidth = 0;
		}
		mVisibleText.first = displayedText.start;
	}

	void UITabHeader::TabButton::drawAtPosition(NVGcontext *ctx, const Vector2i& position, bool active) {
		int xPos = position.x();
		int yPos = position.y();
		int width = mSize.x();
		int height = mSize.y();
		auto theme = mHeader->theme();

		nvgSave(ctx);
		nvgIntersectScissor(ctx, xPos, yPos, width + 1, height);
		if (!active) {
			// Background gradients
			NVGcolor gradTop = theme->mButtonGradientTopPushed;
			NVGcolor gradBot = theme->mButtonGradientBotPushed;

			// Draw the background.
			nvgBeginPath(ctx);
			nvgRoundedRect(ctx, xPos + 1, yPos + 1, width - 1, height + 1,
				theme->mButtonCornerRadius);
			NVGpaint backgroundColor = nvgLinearGradient(ctx, xPos, yPos, xPos, yPos + height,
				gradTop, gradBot);
			nvgFillPaint(ctx, backgroundColor);
			nvgFill(ctx);
		}

		if (active) {
			nvgBeginPath(ctx);
			nvgStrokeWidth(ctx, 1.0f);
			nvgRoundedRect(ctx, xPos + 0.5f, yPos + 1.5f, width,
				height + 1, theme->mButtonCornerRadius);
			nvgStrokeColor(ctx, theme->mBorderLight);
			nvgStroke(ctx);

			nvgBeginPath(ctx);
			nvgRoundedRect(ctx, xPos + 0.5f, yPos + 0.5f, width,
				height + 1, theme->mButtonCornerRadius);
			nvgStrokeColor(ctx, theme->mBorderDark);
			nvgStroke(ctx);
		}
		else {
			nvgBeginPath(ctx);
			nvgRoundedRect(ctx, xPos + 0.5f, yPos + 1.5f, width,
				height, theme->mButtonCornerRadius);
			nvgStrokeColor(ctx, theme->mBorderDark);
			nvgStroke(ctx);
		}
		nvgRestore(ctx);

		// Draw the text with some padding
		int textX = xPos + mHeader->theme()->mTabButtonHorizontalPadding;
		int textY = yPos + mHeader->theme()->mTabButtonVerticalPadding;
		NVGcolor textColor = mHeader->theme()->mTextColor;
		nvgBeginPath(ctx);
		nvgFillColor(ctx, textColor);
		nvgText(ctx, textX, textY, mVisibleText.first, mVisibleText.last);
		if (mVisibleText.last != nullptr)
			nvgText(ctx, textX + mVisibleWidth, textY, dots, nullptr);
	}

	void UITabHeader::TabButton::drawActiveBorderAt(NVGcontext *ctx, const Vector2i &position,
		float offset, const Color &color) {
		int xPos = position.x();
		int yPos = position.y();
		int width = mSize.x();
		int height = mSize.y();
		nvgBeginPath(ctx);
		nvgLineJoin(ctx, NVG_ROUND);
		nvgMoveTo(ctx, xPos + offset, yPos + height + offset);
		nvgLineTo(ctx, xPos + offset, yPos + offset);
		nvgLineTo(ctx, xPos + width - offset, yPos + offset);
		nvgLineTo(ctx, xPos + width - offset, yPos + height + offset);
		nvgStrokeColor(ctx, color);
		nvgStrokeWidth(ctx, mHeader->theme()->mTabBorderWidth);
		nvgStroke(ctx);
	}

	void UITabHeader::TabButton::drawInactiveBorderAt(NVGcontext *ctx, const Vector2i &position,
		float offset, const Color& color) {
		int xPos = position.x();
		int yPos = position.y();
		int width = mSize.x();
		int height = mSize.y();
		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, xPos + offset, yPos + offset, width - offset, height - offset,
			mHeader->theme()->mButtonCornerRadius);
		nvgStrokeColor(ctx, color);
		nvgStroke(ctx);
	}


	UITabHeader::UITabHeader(MainWindow* a_window)
		: UIComponent(a_window), mFont("sans-bold") { }

	void UITabHeader::setActiveTab(int tabIndex) {
		assert(tabIndex < tabCount());
		mActiveTab = tabIndex;
		if (mCallback)
			mCallback(tabIndex);
	}

	int UITabHeader::activeTab() const {
		return mActiveTab;
	}

	bool UITabHeader::isTabVisible(int index) const {
		return index >= mVisibleStart && index < mVisibleEnd;
	}

	void UITabHeader::addTab(const std::string & label) {
		addTab(tabCount(), label);
	}

	void UITabHeader::addTab(int index, const std::string &label) {
		assert(index <= tabCount());
		mTabButtons.insert(std::next(mTabButtons.begin(), index), TabButton(*this, label));
		setActiveTab(index);
	}

	int UITabHeader::removeTab(const std::string &label) {
		auto element = std::find_if(mTabButtons.begin(), mTabButtons.end(),
			[&](const TabButton& tb) { return label == tb.label(); });
		int index = std::distance(mTabButtons.begin(), element);
		if (element == mTabButtons.end())
			return -1;
		mTabButtons.erase(element);
		if (index == mActiveTab && index != 0)
			setActiveTab(index - 1);
		return index;
	}

	void UITabHeader::removeTab(int index) {
		assert(index < tabCount());
		mTabButtons.erase(std::next(mTabButtons.begin(), index));
		if (index == mActiveTab && index != 0)
			setActiveTab(index - 1);
	}

	const std::string& UITabHeader::tabLabelAt(int index) const {
		assert(index < tabCount());
		return mTabButtons[index].label();
	}

	int UITabHeader::tabIndex(const std::string &label) {
		auto it = std::find_if(mTabButtons.begin(), mTabButtons.end(),
			[&](const TabButton& tb) { return label == tb.label(); });
		if (it == mTabButtons.end())
			return -1;
		return it - mTabButtons.begin();
	}

	void UITabHeader::ensureTabVisible(int index) {
		auto visibleArea = visibleButtonArea();
		auto visibleWidth = visibleArea.second.x() - visibleArea.first.x();
		int allowedVisibleWidth = size().x() - 2 * theme()->mTabControlWidth;
		assert(allowedVisibleWidth >= visibleWidth);
		assert(index >= 0 && index < (int)mTabButtons.size());

		auto first = visibleBegin();
		auto last = visibleEnd();
		auto goal = tabIterator(index);

		// Reach the goal tab with the visible range.
		if (goal < first) {
			do {
				--first;
				visibleWidth += first->size().x();
			} while (goal < first);
			while (allowedVisibleWidth < visibleWidth) {
				--last;
				visibleWidth -= last->size().x();
			}
		}
		else if (goal >= last) {
			do {
				visibleWidth += last->size().x();
				++last;
			} while (goal >= last);
			while (allowedVisibleWidth < visibleWidth) {
				visibleWidth -= first->size().x();
				++first;
			}
		}

		// Check if it is possible to expand the visible range on either side.
		while (first != mTabButtons.begin()
			&& std::next(first, -1)->size().x() < allowedVisibleWidth - visibleWidth) {
			--first;
			visibleWidth += first->size().x();
		}
		while (last != mTabButtons.end()
			&& last->size().x() < allowedVisibleWidth - visibleWidth) {
			visibleWidth += last->size().x();
			++last;
		}

		mVisibleStart = std::distance(mTabButtons.begin(), first);
		mVisibleEnd = std::distance(mTabButtons.begin(), last);
	}

	std::pair<Vector2i, Vector2i> UITabHeader::visibleButtonArea() const {
		if (mVisibleStart == mVisibleEnd)
			return{ Vector2i::Zero(), Vector2i::Zero() };
		auto topLeft = getRelPos() + Vector2i(theme()->mTabControlWidth, 0);
		auto width = std::accumulate(visibleBegin(), visibleEnd(), theme()->mTabControlWidth,
			[](int acc, const TabButton& tb) {
			return acc + tb.size().x();
		});
		auto bottomRight = getRelPos() + Vector2i(width, size().y());
		return{ topLeft, bottomRight };
	}

	std::pair<Vector2i, Vector2i> UITabHeader::activeButtonArea() const {
		if (mVisibleStart == mVisibleEnd || mActiveTab < mVisibleStart || mActiveTab >= mVisibleEnd)
			return{ Vector2i::Zero(), Vector2i::Zero() };
		auto width = std::accumulate(visibleBegin(), activeIterator(), theme()->mTabControlWidth,
			[](int acc, const TabButton& tb) {
			return acc + tb.size().x();
		});
		auto topLeft = getRelPos() + Vector2i(width, 0);
		auto bottomRight = getRelPos() + Vector2i(width + activeIterator()->size().x(), size().y());
		return{ topLeft, bottomRight };
	}

	void UITabHeader::performLayout(NVGcontext* ctx) {
		UIComponent::performLayout(ctx);

		Vector2i currentPosition = Vector2i::Zero();
		// Place the tab buttons relative to the beginning of the tab header.
		for (auto& tab : mTabButtons) {
			auto tabPreferred = tab.calcMinSize(ctx);
			if (tabPreferred.x() < theme()->mTabMinButtonWidth)
				tabPreferred.x() = theme()->mTabMinButtonWidth;
			else if (tabPreferred.x() > theme()->mTabMaxButtonWidth)
				tabPreferred.x() = theme()->mTabMaxButtonWidth;
			tab.setSize(tabPreferred);
			tab.calculateVisibleString(ctx);
			currentPosition.x() += tabPreferred.x();
		}
		calculateVisibleEnd();
		if (mVisibleStart != 0 || mVisibleEnd != tabCount())
			mOverflowing = true;
	}

	Vector2i UITabHeader::calcMinSize(NVGcontext* ctx) const {
		// Set up the nvg context for measuring the text inside the tab buttons.
		nvgFontFace(ctx, mFont.c_str());
		nvgFontSize(ctx, theme()->mStandardFontSize);
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		Vector2i size = Vector2i(2 * theme()->mTabControlWidth, 0);
		for (auto& tab : mTabButtons) {
			auto tabPreferred = tab.calcMinSize(ctx);
			if (tabPreferred.x() < theme()->mTabMinButtonWidth)
				tabPreferred.x() = theme()->mTabMinButtonWidth;
			else if (tabPreferred.x() > theme()->mTabMaxButtonWidth)
				tabPreferred.x() = theme()->mTabMaxButtonWidth;
			size.x() += tabPreferred.x();
			size.y() = std::max(size.y(), tabPreferred.y());
		}
		return size;
	}

	UIComponent* UITabHeader::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
		UIComponent* ret = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
		if (ret) return ret;
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			switch (locateClick(a_relCursor)) {
			case ClickLocation::LeftControls:
				onArrowLeft();
				return this;
			case ClickLocation::RightControls:
				onArrowRight();
				return this;
			case ClickLocation::TabButtons:
				auto first = visibleBegin();
				auto last = visibleEnd();
				int currentPosition = theme()->mTabControlWidth;
				int endPosition = a_relCursor.localCoord(this).x();
				auto firstInvisible = std::find_if(first, last,
					[&currentPosition, endPosition](const TabButton& tb) {
					currentPosition += tb.size().x();
					return currentPosition > endPosition;
				});

				// Did not click on any of the tab buttons
				if (firstInvisible == last)
					return this;

				// Update the active tab and invoke the callback.
				setActiveTab(std::distance(mTabButtons.begin(), firstInvisible));
				return this;
			}
		}
		return nullptr;
	}

	void UITabHeader::draw(NVGcontext* ctx) {
		// Draw controls.
		if (mOverflowing)
			drawControls(ctx);

		// Set up common text drawing settings.
		nvgFontFace(ctx, mFont.c_str());
		nvgFontSize(ctx, theme()->mStandardFontSize);
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

		auto current = visibleBegin();
		auto last = visibleEnd();
		auto active = std::next(mTabButtons.begin(), mActiveTab);
		Vector2i currentPosition = Vector2i(theme()->mTabControlWidth, 0);

		// Flag to draw the active tab last. Looks a little bit better.
		bool drawActive = false;
		Vector2i activePosition = Vector2i::Zero();

		// Draw inactive visible buttons.
		while (current != last) {
			if (current == active) {
				drawActive = true;
				activePosition = currentPosition;
			}
			else {
				current->drawAtPosition(ctx, currentPosition, false);
			}
			currentPosition.x() += current->size().x();
			++current;
		}

		// Draw active visible button.
		if (drawActive)
			active->drawAtPosition(ctx, activePosition, true);

		setMinSize(calcMinSize(m_window->getContext()));
	}

	void UITabHeader::notifyChildResized(UIComponent* a_child) {
		setMinSize(calcMinSize(m_window->getContext()));
		performLayout(m_window->getContext());
	}

	void UITabHeader::_onResize() {
		setMinSize(calcMinSize(m_window->getContext()));
		performLayout(m_window->getContext());
	}

	void UITabHeader::calculateVisibleEnd() {
		auto first = visibleBegin();
		auto last = mTabButtons.end();
		int currentPosition = theme()->mTabControlWidth;
		int lastPosition = size().x() - theme()->mTabControlWidth;
		auto firstInvisible = std::find_if(first, last,
			[&currentPosition, lastPosition](const TabButton& tb) {
			currentPosition += tb.size().x();
			return currentPosition > lastPosition;
		});
		mVisibleEnd = std::distance(mTabButtons.begin(), firstInvisible);
	}

	void UITabHeader::drawControls(NVGcontext* ctx) {
		// Left button.
		bool active = mVisibleStart != 0;

		// Draw the arrow.
		nvgBeginPath(ctx);
		auto iconLeft = utf8(ENTYPO_LEFT_BOLD);
		int fontSize = theme()->mButtonFontSize;
		float ih = fontSize;
		ih *= 1.5f;
		nvgFontSize(ctx, ih);
		nvgFontFace(ctx, "icons");
		NVGcolor arrowColor;
		if (active)
			arrowColor = theme()->mTextColor;
		else
			arrowColor = theme()->mButtonGradientBotPushed;
		nvgFillColor(ctx, arrowColor);
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
		float yScaleLeft = 0.5f;
		float xScaleLeft = 0.2f;
		Vector2f leftIconPos = Vector2f(xScaleLeft*theme()->mTabControlWidth, yScaleLeft*size().cast<float>().y());
		nvgText(ctx, leftIconPos.x(), leftIconPos.y() + 1, iconLeft.data(), nullptr);

		// Right button.
		active = mVisibleEnd != tabCount();
		// Draw the arrow.
		nvgBeginPath(ctx);
		auto iconRight = utf8(ENTYPO_RIGHT_BOLD);
		fontSize = theme()->mButtonFontSize;
		ih = fontSize;
		ih *= 1.5f;
		nvgFontSize(ctx, ih);
		nvgFontFace(ctx, "icons");
		float rightWidth = nvgTextBounds(ctx, 0, 0, iconRight.data(), nullptr, nullptr);
		if (active)
			arrowColor = theme()->mTextColor;
		else
			arrowColor = theme()->mButtonGradientBotPushed;
		nvgFillColor(ctx, arrowColor);
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
		float yScaleRight = 0.5f;
		float xScaleRight = 1.0f - xScaleLeft - rightWidth / theme()->mTabControlWidth;
		auto leftControlsPos = Vector2f(size().cast<float>().x() - theme()->mTabControlWidth, 0);
		Vector2f rightIconPos = leftControlsPos + Vector2f(xScaleRight*theme()->mTabControlWidth, yScaleRight*size().cast<float>().y());
		nvgText(ctx, rightIconPos.x(), rightIconPos.y() + 1, iconRight.data(), nullptr);
	}

	UITabHeader::ClickLocation UITabHeader::locateClick(const UICoord& p) {
		auto leftDistance = p.localCoord(this).array();
		bool hitLeft = (leftDistance >= 0).all() && (leftDistance < Vector2i(theme()->mTabControlWidth, size().y()).array()).all();
		if (hitLeft)
			return ClickLocation::LeftControls;
		auto rightDistance = (p.localCoord(this) - (Vector2i(size().x() - theme()->mTabControlWidth, 0))).array();
		bool hitRight = (rightDistance >= 0).all() && (rightDistance < Vector2i(theme()->mTabControlWidth, size().y()).array()).all();
		if (hitRight)
			return ClickLocation::RightControls;
		return ClickLocation::TabButtons;
	}

	void UITabHeader::onArrowLeft() {
		if (mVisibleStart == 0)
			return;
		--mVisibleStart;
		calculateVisibleEnd();
	}

	void UITabHeader::onArrowRight() {
		if (mVisibleEnd == tabCount())
			return;
		++mVisibleStart;
		calculateVisibleEnd();
	}
}
