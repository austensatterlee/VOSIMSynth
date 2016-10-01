#include "UIUnitPort.h"
#include "UIUnitContainer.h"
#include "UICircuitPanel.h"
#include "Unit.h"
#include "VoiceManager.h"
#include "Theme.h"


synui::UIUnitPort::UIUnitPort(MainWindow* a_window, UICircuitPanel* a_circuitPanel, syn::VoiceManager* a_vm, int a_unitId, int a_portNum, bool a_isInput) :
	UIComponent{a_window},
	m_vm(a_vm),
	m_circuitPanel(a_circuitPanel),
	m_unitId(a_unitId),
	m_portNum(a_portNum),
	m_isInput(a_isInput) {
	const syn::Unit& unit = m_vm->getUnit(m_unitId);
	string portName;
	if (m_isInput) {
		portName = unit.inputName(m_portNum);
	} else {
		portName = unit.outputName(m_portNum);
	}
	int textWidth;
	float bounds[4];
	NVGcontext* nvg = m_window->getContext();
	nvgFontSize(nvg, (float)theme()->mPortFontSize);
	nvgTextBounds(nvg, 0, 0, portName.c_str(), nullptr, bounds);
	textWidth = syn::MAX(bounds[2] - bounds[0],10.0f);
	setMinSize(Vector2i{textWidth, theme()->mPortFontSize});
}

bool synui::UIUnitPort::onMouseDrag(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	return true;
}

synui::UIComponent* synui::UIUnitPort::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	m_isDragging = true;
	return this;
}

bool synui::UIUnitPort::onMouseUp(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	m_isDragging = false;
	UIUnitContainer* selectedUnit = m_circuitPanel->findUnitContainer(UICoord(m_window->cursorPos()));
	if (selectedUnit && selectedUnit != m_parent) {
		if (m_isInput) {
			UIUnitPort* port = selectedUnit->getSelectedOutPort(a_relCursor);
			if (port)
				m_circuitPanel->requestAddConnection(port->getUnitId(), port->getPortId(), m_unitId, m_portNum);
		} else {
			UIUnitPort* port = selectedUnit->getSelectedInPort(a_relCursor);
			if (port)
				m_circuitPanel->requestAddConnection(m_unitId, m_portNum, port->getUnitId(), port->getPortId());
		}
	}
	return true;
}

void synui::UIUnitPort::draw(NVGcontext* a_nvg) {
	const syn::Unit& unit = m_vm->getUnit(m_unitId);
	string portName;

	Color bgColor;

	if (m_isInput) {
		portName = unit.inputName(m_portNum);
		bgColor = hovered() || m_isDragging ? theme()->mInputPortHighlightedBG : theme()->mInputPortBG;
	} else {
		portName = unit.outputName(m_portNum);
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
