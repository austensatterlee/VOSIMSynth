#include "UIUnitControlContainer.h"
#include "UICircuitPanel.h"

syn::UIUnitControlContainer::UIUnitControlContainer(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, UIUnitControl* a_unitControl):
	UIWindow{a_window, a_vm->getUnit(a_unitId).getName()},
	m_vm(a_vm),
	m_unitId(a_unitId),
	m_unitControl(a_unitControl) {
	m_row = new UIRow(m_window);
	m_row->setRelPos({0,theme()->mWindowHeaderHeight});
	addChildToBody(m_row);
	m_cols[0] = new UICol(m_window);
	m_row->addChild(m_cols[0]);
	m_cols[1] = new UICol(m_window);
	m_row->addChild(m_cols[1]);
	m_cols[2] = new UICol(m_window);
	m_row->addChild(m_cols[2]);

	m_cols[1]->addChild(a_unitControl);

	m_cols[0]->setChildResizePolicy(UICol::CMAX);
	m_cols[0]->setSelfResizePolicy(UICol::SFIT);
	m_cols[1]->setChildResizePolicy(UICol::CMAX);
	m_cols[1]->setSelfResizePolicy(UICol::SFIT);
	m_cols[1]->setPadding({5,1,5,1});
	m_cols[2]->setChildResizePolicy(UICol::CMAX);
	m_cols[2]->setSelfResizePolicy(UICol::SFIT);


	const Unit& unit = m_vm->getUnit(m_unitId);
	int nIn = unit.getNumInputs();
	int nOut = unit.getNumOutputs();

	for (int i = 0; i < nIn; i++) {
		m_inPorts.push_back(new UIUnitPort(a_window, a_vm, a_unitId, i, true));
		m_cols[0]->addChild(m_inPorts[i]);
	}

	for (int i = 0; i < nOut; i++) {
		m_outPorts.push_back(new UIUnitPort(a_window, a_vm, a_unitId, i, false));
		m_cols[2]->addChild(m_outPorts[i]);
	}

	m_cols[0]->pack();
	m_cols[1]->pack();
	m_cols[2]->pack();
	m_row->setGreedyChild(m_cols[1], NVG_ALIGN_CENTER);
	m_row->pack();

	m_closeButton = new UIButton(a_window, "", 0xE729);
	m_closeButton->setCallback([&]() {
		this->close();
	});
	m_closeButton->setSize({theme()->mWindowHeaderHeight - 4 ,theme()->mWindowHeaderHeight - 4});
	m_closeButton->setFontSize(8);
	addChildToHeader(m_closeButton);
}

syn::UIUnitPort* syn::UIUnitControlContainer::getSelectedInPort(const Vector2i& a_pt) const {
	for (UIUnitPort* port : m_inPorts) {
		Vector2i relPos = a_pt - port->parent()->getAbsPos();
		if (port->contains(relPos))
			return port;
	}
	return nullptr;
}

syn::UIUnitPort* syn::UIUnitControlContainer::getSelectedOutPort(const Vector2i& a_pt) const {
	for (UIUnitPort* port : m_outPorts) {
		Vector2i relPos = a_pt - port->parent()->getAbsPos();
		if (port->contains(relPos))
			return port;
	}
	return nullptr;
}

syn::UIComponent* syn::UIUnitControlContainer::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	UIComponent* child = UIComponent::onMouseDown(a_relCursor, a_diffCursor, a_isDblClick);
	if (child)
		return child;
	return this;
}

void syn::UIUnitControlContainer::close() const {
	m_window->getCircuitPanel()->requestDeleteUnit(m_unitId);
}

syn::UIUnitPort::UIUnitPort(VOSIMWindow* a_window, VoiceManager* a_vm, int a_unitId, int a_portNum, bool a_isInput) :
	UIComponent{a_window},
	m_vm(a_vm),
	m_unitId(a_unitId),
	m_portNum(a_portNum),
	m_isInput(a_isInput) {
	const Unit& unit = m_vm->getUnit(m_unitId);
	string portName;
	if (m_isInput) {
		portName = unit.getInputName(m_portNum);
	} else {
		portName = unit.getOutputName(m_portNum);
	}
	int textWidth;
	float bounds[4];
	NVGcontext* nvg = m_window->getContext();
	nvgFontSize(nvg, (float)theme()->mPortFontSize);
	nvgTextBounds(nvg, 0, 0, portName.c_str(), nullptr, bounds);
	textWidth = bounds[2] - bounds[0];
	setMinSize_(Vector2i{textWidth + 5, theme()->mPortFontSize + 5});
}

bool syn::UIUnitPort::onMouseDrag(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	return true;
}

syn::UIComponent* syn::UIUnitPort::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	m_isDragging = true;
	return this;
}

bool syn::UIUnitPort::onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	m_isDragging = false;
	UIUnitControlContainer* selectedUnit = m_window->getCircuitPanel()->getUnit(m_window->cursorPos());
	if (selectedUnit && selectedUnit != m_parent) {
		if (m_isInput) {
			UIUnitPort* port = selectedUnit->getSelectedOutPort(m_window->cursorPos());
			if (port)
				m_window->getCircuitPanel()->requestAddConnection(port->getUnitId(), port->getPortId(), m_unitId, m_portNum);
		} else {
			UIUnitPort* port = selectedUnit->getSelectedInPort(m_window->cursorPos());
			if (port)
				m_window->getCircuitPanel()->requestAddConnection(m_unitId, m_portNum, port->getUnitId(), port->getPortId());
		}
	}
	return true;
}

void syn::UIUnitPort::draw(NVGcontext* a_nvg) {
	const Unit& unit = m_vm->getUnit(m_unitId);
	string portName;

	Color bgColor;

	if (m_isInput) {
		portName = unit.getInputName(m_portNum);
		bgColor = hovered() || m_isDragging ? theme()->mInputPortHighlightedBG : theme()->mInputPortBG;
	} else {
		portName = unit.getOutputName(m_portNum);
		bgColor = hovered() || m_isDragging ? theme()->mOutputPortHighlightedBG : theme()->mOutputPortBG;
	}

	nvgBeginPath(a_nvg);
	nvgFillColor(a_nvg, bgColor);
	nvgRoundedRect(a_nvg, 0, 0, size()[0], size()[1], 2);
	nvgFill(a_nvg);

	nvgFontSize(a_nvg, (float)theme()->mPortFontSize);
	nvgFillColor(a_nvg, Color(Vector3f{1.0,1.0,1.0}));
	nvgTextAlign(a_nvg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgText(a_nvg, size()[0] / 2, size()[1] / 2, portName.c_str(), nullptr);

	if (m_isDragging) {
		nvgStrokeColor(a_nvg, Color(Vector3f{0,0,0}));
		nvgBeginPath(a_nvg);
		nvgMoveTo(a_nvg, size()[0] / 2.0f, size()[1] / 2.0f);
		nvgLineTo(a_nvg, m_window->cursorPos()[0] - getAbsPos()[0], m_window->cursorPos()[1] - getAbsPos()[1]);
		nvgStroke(a_nvg);
	}
}
