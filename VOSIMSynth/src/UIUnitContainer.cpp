#include "UIUnitContainer.h"
#include "UICircuitPanel.h"
#include "UIUnitPort.h"
#include "UICell.h"
#include "UIButton.h"
#include "Theme.h"
#include <entypo.h>
#include <VoiceManager.h>
#include <UILabel.h>
#include <UIUnitControl.h>

syn::UIUnitContainer::UIUnitContainer(MainWindow* a_window, UICircuitPanel* a_circuitPanel, VoiceManager* a_vm, UIUnitControl* a_unitCtrl) :
	UIComponent(a_window),
	m_vm(a_vm),
	m_circuitPanel(a_circuitPanel),
	m_unitId(a_unitCtrl->unitId()), 
	m_isSelected(false),
	m_unitCtrl(shared_ptr<UIUnitControl>(a_unitCtrl))
{
}

syn::UIUnitPort* syn::UIUnitContainer::getSelectedInPort(const syn::UICoord& a_pt) const {
	for (UIUnitPort* port : m_inPorts) {
		if (port->contains(a_pt))
			return port;
	}
	return nullptr;
}

syn::UIUnitPort* syn::UIUnitContainer::getSelectedOutPort(const UICoord& a_pt) const {
	for (UIUnitPort* port : m_outPorts) {
		if (port->contains(a_pt))
			return port;
	}
	return nullptr;
}

void syn::UIUnitContainer::close() const {
	m_circuitPanel->requestDeleteUnit(m_unitId);
}

bool syn::UIUnitContainer::onMouseMove(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	UIComponent::onMouseMove(a_relCursor, a_diffCursor);
	return true;
}

void syn::UIUnitContainer::makeSelected(bool a_isSelected) {
	m_isSelected = a_isSelected;
}

bool syn::UIUnitContainer::isSelected() const {
	return m_isSelected;
}

void syn::UIUnitContainer::draw(NVGcontext* a_nvg) {
	int ds = theme()->mWindowDropShadowSize, cr = theme()->mWindowCornerRadius;

	/* Draw window */
	nvgBeginPath(a_nvg);
	nvgRoundedRect(a_nvg, 0, 0, m_size[0], m_size[1], cr);

	nvgFillColor(a_nvg, m_focused ? theme()->mWindowFillFocused : theme()->mWindowFillUnfocused);

	nvgFill(a_nvg);

	/* Draw a drop shadow */
	NVGpaint shadowPaint;
	if (!isSelected()) {
		shadowPaint = nvgBoxGradient(a_nvg, 0, 0, m_size[0], m_size[1], cr * 2, ds * 2,
			Color(0.0f,1.0f), Color(1.0f,0.0f));
	}else {
		shadowPaint = nvgBoxGradient(a_nvg, 0, 0, m_size[0], m_size[1], cr * 2, ds * 2,
			Color(1.0f,1.0f),Color(0.0f,0.0f));
	}
	nvgBeginPath(a_nvg);
	nvgRect(a_nvg, 0 - ds, 0 - ds, m_size[0] + 2 * ds, m_size[1] + 2 * ds);
	nvgRoundedRect(a_nvg, 0, 0, m_size[0], m_size[1], cr);
	nvgPathWinding(a_nvg, NVG_HOLE);
	nvgFillPaint(a_nvg, shadowPaint);
	nvgFill(a_nvg);
}

syn::UIDefaultUnitContainer::UIDefaultUnitContainer(MainWindow* a_window, UICircuitPanel* a_circuitPanel, VoiceManager* a_vm, UIUnitControl* a_unitCtrl) :
	UIComponent(a_window),
	UIUnitContainer(a_window, a_circuitPanel, a_vm, a_unitCtrl),
	m_titleRow(new UIRow(a_window)),
	m_col(new UICol(a_window)),
	m_portRow(new UIRow(a_window)),
	m_portCols{new UICol(a_window), new UICol(a_window)}
{
	addChild(m_col);
	m_col->addChild(m_titleRow);
	m_col->addChild(m_portRow);
	m_portRow->setChildResizePolicy(UICell::CNONE);
	m_portRow->setChildrenSpacing(2);
	m_portCols[0]->setChildResizePolicy(UICell::CMATCHMIN);
	m_portCols[1]->setChildResizePolicy(UICell::CMATCHMIN);

	const Unit& unit = m_vm->getUnit(m_unitId);

	int nIn = unit.getNumInputs();
	int nOut = unit.getNumOutputs();

	for (int i = 0; i < nIn; i++) {
		m_inPorts.push_back(new UIUnitPort(a_window, a_circuitPanel, a_vm, m_unitId, i, true));
		m_portCols[0]->addChild(m_inPorts[i]);
	}

	for (int i = 0; i < nOut; i++) {
		m_outPorts.push_back(new UIUnitPort(a_window, a_circuitPanel, a_vm, m_unitId, i, false));
		m_portCols[1]->addChild(m_outPorts[i]);
	}

	m_portRow->addChild(m_portCols[0]);
	m_portRow->addChild(m_portCols[1]);

	m_closeButton = new UIButton(a_window, "", ENTYPO_TRASH);
	m_closeButton->setCallback([&]() {
		this->close();
	});
	m_closeButton->setRelPos({m_closeButton->getRelPos().x(), 2});
	m_closeButton->setSize({theme()->mWindowHeaderHeight, theme()->mWindowHeaderHeight - 6});
	m_closeButton->setFontSize(10);
	UILabel *title = new UILabel(a_window);
	title->setText(unit.getName());
	m_titleRow->addChild(title);
	m_titleRow->addChild(m_closeButton);
	setMinSize(m_col->minSize());
}

syn::UIComponent* syn::UIDefaultUnitContainer::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* child = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (child)
		return child;
	return this;
}

bool syn::UIDefaultUnitContainer::onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	move(a_diffCursor);
	return true;
}

syn::UIInputUnitContainer::UIInputUnitContainer(MainWindow* a_window, UICircuitPanel* a_circuitPanel, VoiceManager* a_vm, UIUnitControl* a_unitCtrl) :
	UIComponent(a_window),
	UIUnitContainer(a_window, a_circuitPanel, a_vm, a_unitCtrl),
	m_titleRow(new UIRow(a_window)),
	m_col(new UICol(a_window)),
	m_portRow(new UIRow(a_window)),
	m_portCol(new UICol(a_window))
{
	addChild(m_col);
	m_col->addChild(m_titleRow);
	m_col->addChild(m_portRow);
	m_portRow->setChildResizePolicy(UICell::CNONE);
	m_portRow->setChildrenSpacing(2);
	m_portCol->setChildResizePolicy(UICell::CMATCHMIN);

	const Unit& unit = m_vm->getUnit(m_unitId);

	int nOut = unit.getNumOutputs();

	for (int i = 0; i < nOut; i++) {
		m_outPorts.push_back(new UIUnitPort(a_window, a_circuitPanel, a_vm, m_unitId, i, false));
		m_portCol->addChild(m_outPorts[i]);
	}

	m_portRow->addChild(m_portCol);

	UILabel* title = new UILabel(a_window);
	title->setText(unit.getName());
	m_titleRow->addChild(title);
	setMinSize(m_col->minSize());
}

syn::UIOutputUnitContainer::UIOutputUnitContainer(MainWindow* a_window, UICircuitPanel* a_circuitPanel, VoiceManager* a_vm, UIUnitControl* a_unitCtrl) :
	UIComponent(a_window),
	UIUnitContainer(a_window, a_circuitPanel, a_vm, a_unitCtrl),
	m_titleRow(new UIRow(a_window)),
	m_col(new UICol(a_window)),
	m_portRow(new UIRow(a_window)),
	m_portCol(new UICol(a_window))
{
	addChild(m_col);
	m_col->addChild(m_titleRow);
	m_col->addChild(m_portRow);
	m_portRow->setChildResizePolicy(UICell::CNONE);
	m_portRow->setChildrenSpacing(2);
	m_portCol->setChildResizePolicy(UICell::CMATCHMIN);

	const Unit& unit = m_vm->getUnit(m_unitId);

	int nIn = unit.getNumInputs();

	for (int i = 0; i < nIn; i++) {
		m_inPorts.push_back(new UIUnitPort(a_window, a_circuitPanel, a_vm, m_unitId, i, true));
		m_portCol->addChild(m_inPorts[i]);
	}

	m_portRow->addChild(m_portCol);

	UILabel *title = new UILabel(a_window);
	title->setText(unit.getName());
	m_titleRow->addChild(title);
	setMinSize(m_col->minSize());
}
