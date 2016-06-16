#include "UIUnitPort.h"
#include "UIUnitControlContainer.h"
#include "UICircuitPanel.h"
#include "Unit.h"
#include "VoiceManager.h"
#include "Theme.h"


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
	setMinSize(Vector2i{textWidth + 5, theme()->mPortFontSize + 2});
}

bool syn::UIUnitPort::onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	return true;
}

syn::UIComponent* syn::UIUnitPort::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	m_isDragging = true;
	return this;
}

bool syn::UIUnitPort::onMouseUp(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	m_isDragging = false;
	UIUnitControlContainer* selectedUnit = m_window->getCircuitPanel()->getUnit(UICoord(m_window->cursorPos()));
	if (selectedUnit && selectedUnit != m_parent) {
		if (m_isInput) {
			UIUnitPort* port = selectedUnit->getSelectedOutPort(a_relCursor);
			if (port)
				m_window->getCircuitPanel()->requestAddConnection(port->getUnitId(), port->getPortId(), m_unitId, m_portNum);
		} else {
			UIUnitPort* port = selectedUnit->getSelectedInPort(a_relCursor);
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
	nvgFillColor(a_nvg, Color(Vector3f{1.0f,1.0f,1.0f}));
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
