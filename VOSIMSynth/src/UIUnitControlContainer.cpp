#include "UIUnitControlContainer.h"
#include "UICircuitPanel.h"

syn::UIUnitControlContainer::UIUnitControlContainer(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, UIUnitControl* a_unitControl):
	UIWindow{a_window, a_vm->getUnit(a_unitId).getName()},
	m_vm(a_vm),
	m_unitId(a_unitId),
	m_unitControl(a_unitControl),
	m_inWidth(0),
	m_outWidth(0)
{
	a_unitControl->setRelPos({ 50, theme()->mWindowHeaderHeight });
	a_unitControl->setAutosize(false);
	this->addChild(a_unitControl);

	const Unit& unit = m_vm->getUnit(m_unitId);
	int nIn = unit.getNumInputs();
	int nOut = unit.getNumOutputs();
	for (int i = 0; i < nIn; i++) {
		m_inPorts.push_back(new UIUnitPort(a_window, a_vm, a_unitId, i, true));
		m_inPorts[i]->setRelPos({ 1,theme()->mWindowHeaderHeight + i * 15 });
		m_inPorts[i]->setAutosize(false);
		this->addChild(m_inPorts[i]);
	}

	for (int i = 0; i < nOut; i++) {
		m_outPorts.push_back(new UIUnitPort(a_window, a_vm, a_unitId, i, false));
		m_outPorts[i]->setRelPos({ size().x()-50,theme()->mWindowHeaderHeight + i * 15 });
		m_outPorts[i]->setAutosize(false);
		this->addChild(m_outPorts[i]);
	}
}

Eigen::Vector2i syn::UIUnitControlContainer::calcAutoSize() const {
	int inWidth = 0;
	int inHeight = 0;
	for(UIUnitPort* port : m_inPorts) {
		Vector2i portSize = port->size();
		inWidth = MAX(inWidth, portSize[0]);
		inHeight += portSize[1];
	}
	int outWidth = 0;
	int outHeight = 0;
	for (UIUnitPort* port : m_outPorts) {
		Vector2i portSize = port->size();
		outWidth = MAX(outWidth, portSize[0]);
		outHeight += portSize[1];
	}
	Vector2i unitCtrlSize = m_unitControl->calcAutoSize();
	return{ outWidth + inWidth + unitCtrlSize[0],theme()->mWindowHeaderHeight+MAX(outHeight,MAX(unitCtrlSize[1],inHeight)) };
}

void syn::UIUnitControlContainer::onResize() {
	const Unit& unit = m_vm->getUnit(m_unitId);
	m_inWidth = 0;
	m_outWidth = 0;
	int portHeight = 0;
	for (UIUnitPort* port : m_inPorts) {
		Vector2i portSize = port->calcAutoSize();
		m_inWidth = MAX(m_inWidth, portSize[0]);
		portHeight = MAX(portHeight, portSize[1]);
	}
	for (UIUnitPort* port : m_outPorts) {
		Vector2i portSize = port->calcAutoSize();
		m_outWidth = MAX(m_outWidth, portSize[0]);
		portHeight = MAX(portHeight, portSize[1]);
	}
	Vector2i unitCtrlSize = m_unitControl->calcAutoSize();
	m_unitControl->setRelPos({ m_inWidth, theme()->mWindowHeaderHeight });
	m_unitControl->setSize({ unitCtrlSize[0],size()[1] });

	int nIn = unit.getNumInputs();
	int nOut = unit.getNumOutputs();
	for (int i = 0; i < nIn; i++) {
		m_inPorts[i]->setRelPos({ 1,theme()->mWindowHeaderHeight + i * portHeight });
		m_inPorts[i]->setSize({ m_inWidth,portHeight });
	}

	for (int i = 0; i < nOut; i++) {
		m_outPorts[i]->setRelPos({ size()[0] - m_outWidth, theme()->mWindowHeaderHeight+i * portHeight });
		m_outPorts[i]->setSize({ m_outWidth,portHeight });
	}
}

syn::UIUnitPort* syn::UIUnitControlContainer::getSelectedInPort(const Vector2i& a_pt) const {
	Vector2i relPos = a_pt - getAbsPos();
	for (UIUnitPort* port : m_inPorts) {
		if (port->contains(relPos))
			return port;
	}
	return nullptr;
}

syn::UIUnitPort* syn::UIUnitControlContainer::getSelectedOutPort(const Vector2i& a_pt) const {
	Vector2i relPos = a_pt - getAbsPos();
	for (UIUnitPort* port : m_outPorts) {
		if (port->contains(relPos))
			return port;
	}
	return nullptr;
}

bool syn::UIUnitControlContainer::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	if (UIComponent::onMouseDown(a_relCursor, a_diffCursor))
		return true;
	return true;
}

bool syn::UIUnitPort::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	return true;
}

bool syn::UIUnitPort::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	m_isDragging = true;
	return true;
}

bool syn::UIUnitPort::onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	m_isDragging = false;
	UIUnitControlContainer* selectedUnit = m_window->getCircuitPanel()->getUnit(m_window->cursorPos());
	if(selectedUnit && selectedUnit!=m_parent) {
		if (m_isInput) {
			UIUnitPort* port = selectedUnit->getSelectedOutPort(m_window->cursorPos());
			if (port)
				m_window->getCircuitPanel()->requestAddConnection(port->getUnitId(), port->getPortId(), m_unitId, m_portNum);
		}else {
			UIUnitPort* port = selectedUnit->getSelectedInPort(m_window->cursorPos());
			if(port)
				m_window->getCircuitPanel()->requestAddConnection(m_unitId, m_portNum, port->getUnitId(), port->getPortId());
		}
	}
	return true;
}

Eigen::Vector2i syn::UIUnitPort::calcAutoSize() const {
	return{ m_autoWidth, m_textHeight };
}

void syn::UIUnitPort::draw(NVGcontext* a_nvg) {
	const Unit& unit = m_vm->getUnit(m_unitId);
	string portName;
	if (m_isInput) {
		portName = unit.getInputName(m_portNum);
	}else {
		portName = unit.getOutputName(m_portNum);
	}
	nvgSave(a_nvg);
	nvgFontSize(a_nvg, (float)m_textHeight);
	nvgFillColor(a_nvg, Color(Vector3f{ 1.0,1.0,1.0 }));
	nvgTextAlign(a_nvg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
	float bounds[4];
	nvgTextBounds(a_nvg, 0, 0, portName.c_str(), nullptr, bounds);
	m_autoWidth = 10 + bounds[2] - bounds[0];
	nvgText(a_nvg, size()[0]/2, 0, portName.c_str(), nullptr);
	
	
	if (m_isDragging) {
		nvgStrokeColor(a_nvg, Color(Vector3f{ 0,0,0 }));
		nvgBeginPath(a_nvg);
		nvgMoveTo(a_nvg, size()[0] / 2.0f, size()[1] / 2.0f);
		nvgLineTo(a_nvg, m_window->cursorPos()[0] - getAbsPos()[0], m_window->cursorPos()[1] - getAbsPos()[1]);
		nvgStroke(a_nvg);
	}
	nvgRestore(a_nvg);
}
