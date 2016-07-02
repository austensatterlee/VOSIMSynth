#include "UIControlPanel.h"
#include "UIUnitContainer.h"
#include "UIUnitControl.h"
#include "UIWindow.h"
#include <Theme.h>

synui::UIControlPanel::UIControlPanel(MainWindow* a_window):
	UIComponent(a_window),
	m_ctrlWindow(new UIWindow(a_window, "Control Panel")),
	m_currUnitContainer(nullptr) 
{
	addChild(m_ctrlWindow);
	m_ctrlWindow->lockPosition(true);
}

void synui::UIControlPanel::showUnitControl(UIUnitContainer* a_unitCointainer) {
	clearUnitControl();
	m_currUnitContainer = a_unitCointainer;
	m_currUnitContainer->makeSelected(true);
	shared_ptr<UIUnitControl> unitCtrl = m_currUnitContainer->getUnitControl();
	m_ctrlWindow->addChild(unitCtrl);
	unitCtrl->setSize(size() - Vector2i{0, theme()->mWindowHeaderHeight});
	unitCtrl->setRelPos({ 0,theme()->mWindowHeaderHeight });
}

void synui::UIControlPanel::clearUnitControl() {
	UIUnitControl* unitCtrl = getCurrentUnitControl();
	if (unitCtrl) {
		m_currUnitContainer->makeSelected(false);
		m_ctrlWindow->removeChild(static_cast<UIComponent*>(unitCtrl));
	}
	m_currUnitContainer = nullptr;
}

synui::UIUnitContainer* synui::UIControlPanel::getCurrentUnitContainer() const {
	return m_currUnitContainer;
}

synui::UIUnitControl* synui::UIControlPanel::getCurrentUnitControl() const {
	return m_currUnitContainer ? m_currUnitContainer->getUnitControl().get() : nullptr;
}

void synui::UIControlPanel::_onResize() {
	m_ctrlWindow->setSize(size());
	UIUnitControl* unitCtrl = getCurrentUnitControl();
	if (unitCtrl)
		unitCtrl->setSize(size() - Vector2i{ 0, theme()->mWindowHeaderHeight });
}
