#include "UICircuitPanel.h"
#include <UIWindow.h>
#include "UIUnitSelector.h"

syn::UICircuitPanel::UICircuitPanel(VOSIMWindow* a_window, VoiceManager* a_vm, UnitFactory* a_unitFactory):
	UIComponent{a_window},
	m_vm(a_vm),
	m_unitFactory(a_unitFactory) 
{
	UIUnitSelector* unitSelector = new UIUnitSelector{ this, m_unitFactory };
	UIWindow* unitSelectorWindow = new UIWindow{ this,"Unit Selector" };
	unitSelectorWindow->setRelPos({ 200,200 });
	unitSelector->setRelPos({ 0, m_root->m_theme->mWindowHeaderHeight });
	addChild(unitSelectorWindow);
	unitSelectorWindow->addChild(unitSelector);
}

void syn::UICircuitPanel::draw(NVGcontext* a_nvg) {
	
}
