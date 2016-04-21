#include "UIUnitControlContainer.h"

syn::UIUnitControlContainer::UIUnitControlContainer(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, UIUnitControl* a_unitControl):
	UIComponent{a_window},
	m_vm(a_vm),
	m_unitId(a_unitId),
	m_unitControl(a_unitControl) 
{
	a_unitControl->setRelPos({ 50,5 });
	this->addChild(a_unitControl);

	const Unit& unit = m_vm->getUnit(m_unitId);
	int nIn = unit.getNumInputs();
	int nOut = unit.getNumOutputs();
	for (int i = 0; i < nIn; i++) {
		m_inPorts.push_back(new UIUnitPort(a_window, a_vm, a_unitId, i, true));
		m_inPorts[i]->setRelPos({ 1,i * 15 });
		this->addChild(m_inPorts[i]);
	}

	for (int i = 0; i < nOut; i++) {
		m_outPorts.push_back(new UIUnitPort(a_window, a_vm, a_unitId, i, false));
		m_outPorts[i]->setRelPos({ m_size.x()-50,i * 15 });
		this->addChild(m_outPorts[i]);
	}
}

void syn::UIUnitControlContainer::draw(NVGcontext* a_nvg) {
	const Unit& unit = m_vm->getUnit(m_unitId);
}

bool syn::UIUnitPort::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	return true;
}

bool syn::UIUnitPort::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) { return true; }

bool syn::UIUnitPort::onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	return true;
}

Eigen::Vector2i syn::UIUnitPort::calcAutoSize() const {
	return{ m_autoWidth, 12 };
}

void syn::UIUnitPort::draw(NVGcontext* a_nvg) {
	const Unit& unit = m_vm->getUnit(m_unitId);
	string portName;
	if (m_isInput) {
		portName = unit.getInputName(m_portNum);
	}else {
		portName = unit.getOutputName(m_portNum);
	}
	m_autoWidth = nvgText(a_nvg, 0, 0, portName.c_str(), nullptr);
}
