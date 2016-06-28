#include "UIControlPanel.h"
#include "UIUnitContainer.h"
#include "UIUnitControl.h"
#include "UIWindow.h"
#include <Theme.h>

syn::UIControlPanel::UIControlPanel(MainWindow* a_window):
	UIComponent(a_window),
	m_ctrlWindow(new UIWindow(a_window, "Control Panel")),
	m_currUnitContainer(nullptr) 
{
	addChild(m_ctrlWindow);
	m_ctrlWindow->lockPosition(true);
}

void syn::UIControlPanel::showUnitControl(UIUnitContainer* a_unitCointainer) {
	clearUnitControl();
	m_currUnitContainer = a_unitCointainer;
	m_currUnitContainer->makeSelected(true);
	shared_ptr<UIUnitControl> unitCtrl = m_currUnitContainer->getUnitControl();
	m_ctrlWindow->addChild(unitCtrl);
	unitCtrl->setSize(size() - Vector2i{0, theme()->mWindowHeaderHeight});
	unitCtrl->setRelPos({ 0,theme()->mWindowHeaderHeight });
}

void syn::UIControlPanel::clearUnitControl() {
	UIUnitControl* unitCtrl = getCurrentUnitControl();
	if (unitCtrl) {
		m_currUnitContainer->makeSelected(false);
		m_ctrlWindow->removeChild(static_cast<UIComponent*>(unitCtrl));
	}
	m_currUnitContainer = nullptr;
}

syn::UIUnitContainer* syn::UIControlPanel::getCurrentUnitContainer() const {
	return m_currUnitContainer;
}

syn::UIUnitControl* syn::UIControlPanel::getCurrentUnitControl() const {
	return m_currUnitContainer ? m_currUnitContainer->getUnitControl().get() : nullptr;
}

void syn::UIControlPanel::_onResize() {
	m_ctrlWindow->setSize(size());
	UIUnitControl* unitCtrl = getCurrentUnitControl();
	if (unitCtrl)
		unitCtrl->setSize(size() - Vector2i{ 0, theme()->mWindowHeaderHeight });
}
