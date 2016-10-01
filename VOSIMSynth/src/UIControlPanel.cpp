#include "UIControlPanel.h"
#include "UIUnitContainer.h"
#include "UIUnitControl.h"
#include "UIScrollPanel.h"
#include <Theme.h>
#include <UITabComponent.h>

synui::UIControlPanel::UIControlPanel(MainWindow* a_window, UITabWidget* a_ctrlPanelTabHeader):
	UIComponent(a_window),
	m_scrollPanel(new UIScrollPanel(a_window)),
	m_currUnitContainer(nullptr),
	m_parentTabComponent(a_ctrlPanelTabHeader)
{
	addChild(m_scrollPanel);
}

void synui::UIControlPanel::showUnitControl(UIUnitContainer* a_unitContainer) {
	clearUnitControl();
	m_currUnitContainer = a_unitContainer;
	m_currUnitContainer->makeSelected(true);
	m_currUnitContainer->m_onClose = [this](const UIUnitContainer*) { this->clearUnitControl(); };

	shared_ptr<UIUnitControl> unitCtrl = m_currUnitContainer->getUnitControl();
	m_scrollPanel->addChild(unitCtrl);
	setMinSize(m_scrollPanel->minSize());
	m_scrollPanel->setSize(m_size - Vector2i::Ones());
	unitCtrl->setRelPos({ 0, 0});
	unitCtrl->setSize(m_size - Vector2i::Ones());
	if (m_parentTabComponent)
		m_parentTabComponent->setActiveTab(m_parentTabComponent->tabIndex(this));
}

void synui::UIControlPanel::clearUnitControl() {
	UIUnitControl* unitCtrl = getCurrentUnitControl();
	
	if (unitCtrl) {
		m_currUnitContainer->m_onClose = [](const UIUnitContainer*) {return; };
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

void synui::UIControlPanel::draw(NVGcontext* a_nvg) {
	
}

void synui::UIControlPanel::_onResize() {
	m_scrollPanel->setSize(m_size - Vector2i::Ones());
	UIUnitControl* unitCtrl = getCurrentUnitControl();
	if (unitCtrl)
		unitCtrl->setSize(m_size - Vector2i::Ones());
}
