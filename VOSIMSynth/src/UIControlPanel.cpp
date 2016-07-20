#include "UIControlPanel.h"
#include "UIUnitContainer.h"
#include "UIUnitControl.h"
#include "UIWindow.h"
#include "UIScrollPanel.h"
#include <Theme.h>

synui::UIControlPanel::UIControlPanel(MainWindow* a_window):
	UIComponent(a_window),
	m_ctrlWindow(new UIWindow(a_window, "Control Panel")),
	m_scrollPanel(new UIScrollPanel(a_window)),
	m_currUnitContainer(nullptr)
{
	addChild(m_ctrlWindow);
	m_ctrlWindow->addChild(m_scrollPanel);
	m_scrollPanel->setRelPos({ 0,theme()->mWindowHeaderHeight });
	m_ctrlWindow->lockPosition(true);
}

void synui::UIControlPanel::showUnitControl(UIUnitContainer* a_unitCointainer) {
	clearUnitControl();
	m_currUnitContainer = a_unitCointainer;
	m_currUnitContainer->makeSelected(true);

	shared_ptr<UIUnitControl> unitCtrl = m_currUnitContainer->getUnitControl();
	m_scrollPanel->addChild(unitCtrl);
	unitCtrl->setRelPos({ 0, 0});
	unitCtrl->setSize(size() - Vector2i{ 0, theme()->mWindowHeaderHeight });
}

void synui::UIControlPanel::clearUnitControl() {
	UIUnitControl* unitCtrl = getCurrentUnitControl();
	if (unitCtrl) {
		m_currUnitContainer->makeSelected(false);
		m_scrollPanel->removeChild(static_cast<UIComponent*>(unitCtrl));
		m_scrollPanel->setScrollPos({ 0,0 });
	}
	m_currUnitContainer = nullptr;
}

synui::UIUnitContainer* synui::UIControlPanel::getCurrentUnitContainer() const {
	return m_currUnitContainer;
}

synui::UIUnitControl* synui::UIControlPanel::getCurrentUnitControl() const {
	return m_currUnitContainer ? m_currUnitContainer->getUnitControl().get() : nullptr;
}

bool synui::UIControlPanel::onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	grow({ 0, -a_diffCursor.y() });
	return true;
}

synui::UIComponent* synui::UIControlPanel::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	if (a_relCursor.localCoord(this).y() < theme()->mWindowHeaderHeight) {
		return this;
	}
	UIComponent* child = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	return child;
}

void synui::UIControlPanel::_onResize() {
	m_ctrlWindow->setSize(size());
	m_scrollPanel->setSize(size() - Vector2i{ 0, theme()->mWindowHeaderHeight });
	UIUnitControl* unitCtrl = getCurrentUnitControl();
	if (unitCtrl)
		unitCtrl->setSize(size() - Vector2i{ 0, theme()->mWindowHeaderHeight });
}
