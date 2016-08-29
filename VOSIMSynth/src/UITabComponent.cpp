#include "UITabComponent.h"
#include <UITabHeader.h>
#include <UIStackedComponent.h>
#include <Theme.h>

namespace synui
{
	UITabWidget::UITabWidget(MainWindow* a_window)
		: UIComponent(a_window), mHeader(new UITabHeader(a_window)), mContent(new UIStackedComponent(a_window)) {
		mHeader->setCallback([this](int i) {
			mContent->setSelectedIndex(i);
			if (mCallback)
				mCallback(i);
		});
		addChild(mHeader,"header");
		addChild(mContent,"content");
	}

	void UITabWidget::setActiveTab(int tabIndex) {
		mHeader->setActiveTab(tabIndex);
		mContent->setSelectedIndex(tabIndex);
	}

	int UITabWidget::activeTab() const {
		assert(mHeader->activeTab() == mContent->selectedIndex());
		return mContent->selectedIndex();
	}

	int UITabWidget::tabCount() const {
		assert(mContent->numChildren() == mHeader->tabCount());
		return mHeader->tabCount();
	}

	UIComponent* UITabWidget::createTab(int index, const std::string &label) {
		UIComponent* tab = new UIComponent(m_window);
		addTab(index, label, tab);
		return tab;
	}

	UIComponent* UITabWidget::createTab(const std::string &label) {
		return createTab(tabCount(), label);
	}

	void UITabWidget::addTab(const std::string &name, UIComponent *tab) {
		addTab(tabCount(), name, tab);
	}

	void UITabWidget::addTab(int index, const std::string &label, UIComponent *tab) {
		assert(index <= tabCount());
		// It is important to add the content first since the callback
		// of the header will automatically fire when a new tab is added.
		mContent->addChild(index, tab);
		mHeader->addTab(index, label);
		assert(mHeader->tabCount() == mContent->numChildren());
	}

	int UITabWidget::tabLabelIndex(const std::string &label) {
		return mHeader->tabIndex(label);
	}

	int UITabWidget::tabIndex(UIComponent* tab) {
		return mContent->childIndex(tab);
	}

	void UITabWidget::ensureTabVisible(int index) {
		if (!mHeader->isTabVisible(index))
			mHeader->ensureTabVisible(index);
	}

	const UIComponent* UITabWidget::tab(const std::string &tabName) const {
		int index = mHeader->tabIndex(tabName);
		if (index == mContent->numChildren())
			return nullptr;
		return mContent->children()[index].get();
	}

	UIComponent *UITabWidget::tab(const std::string &tabName) {
		int index = mHeader->tabIndex(tabName);
		if (index == mContent->numChildren())
			return nullptr;
		return mContent->children()[index].get();
	}

	bool UITabWidget::removeTab(const std::string &tabName) {
		int index = mHeader->removeTab(tabName);
		if (index == -1)
			return false;
		mContent->removeChild(index);
		return true;
	}

	void UITabWidget::removeTab(int index) {
		assert(mContent->numChildren() < index);
		mHeader->removeTab(index);
		mContent->removeChild(index);
		if (activeTab() == index)
			setActiveTab(index == (index - 1) ? index - 1 : 0);
	}

	const std::string &UITabWidget::tabLabelAt(int index) const {
		return mHeader->tabLabelAt(index);
	}

	const UITabHeader& UITabWidget::header() const { return *mHeader; }
	const UIStackedComponent& UITabWidget::content() const { return *mContent; }

	void UITabWidget::performLayout(NVGcontext* ctx) {
		int headerHeight = mHeader->minSize().y();
		int margin = theme()->mTabInnerMargin;
		mHeader->setRelPos({ 0, 0 });
		mHeader->setSize({ size().x(), headerHeight });
		mHeader->performLayout(ctx);
		mContent->setRelPos({ margin, headerHeight + margin });
		mContent->setSize({ size().x() - 2 * margin, size().y() - 2 * margin - headerHeight });
		mContent->performLayout(ctx);
	}

	Vector2i UITabWidget::calcMinSize(NVGcontext* ctx) const {
		auto contentSize = mContent->minSize();
		auto headerSize = mHeader->minSize();
		int margin = theme()->mTabInnerMargin;
		auto borderSize = Vector2i(2 * margin, 2 * margin);
		Vector2i tabPreferredSize = contentSize + borderSize + Vector2i(0, headerSize.y());
		return tabPreferredSize;
	}

	void UITabWidget::draw(NVGcontext* ctx) {
		int tabHeight = mHeader->calcMinSize(ctx).y();
		auto activeArea = mHeader->activeButtonArea();

		for (int i = 0; i < 3; ++i) {
			nvgSave(ctx);

			if (i == 0)
				nvgIntersectScissor(ctx, 0, 0, activeArea.first.x() + 1, size().y());
			else if (i == 1)
				nvgIntersectScissor(ctx, 0 + activeArea.second.x(), 0, size().x() - activeArea.second.x(), size().y());
			else
				nvgIntersectScissor(ctx, 0, 0 + tabHeight + 2, size().x(), size().y());

			nvgBeginPath(ctx);
			nvgStrokeWidth(ctx, 1.0f);
			nvgRoundedRect(ctx, 0 + 0.5f, 0 + tabHeight + 1.5f, size().x() - 1,
				size().y() - tabHeight - 2, theme()->mButtonCornerRadius);
			nvgStrokeColor(ctx, theme()->mBorderLight);
			nvgStroke(ctx);

			nvgBeginPath(ctx);
			nvgRoundedRect(ctx, 0 + 0.5f, 0 + tabHeight + 0.5f, size().x() - 1,
				size().y() - tabHeight - 2, theme()->mButtonCornerRadius);
			nvgStrokeColor(ctx, theme()->mBorderDark);
			nvgStroke(ctx);

			nvgRestore(ctx);
		}
		setMinSize(calcMinSize(ctx));
	}

	void UITabWidget::notifyChildResized(UIComponent* a_child) {
		performLayout(m_window->getContext());
	}

	void UITabWidget::_onResize() {		
		performLayout(m_window->getContext());
	}
}
