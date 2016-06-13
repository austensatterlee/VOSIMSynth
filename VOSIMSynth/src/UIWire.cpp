#include "UIWire.h"
#include <UIUnitControlContainer.h>
#include <UICircuitPanel.h>
#include <UIUnitPort.h>

bool syn::UIWire::contains(const Vector2i& a_absPt) const {
	// Otherwise compute distance to line and verify it is below a threshold
	double distance = pointLineDistance(a_absPt, fromPt(), toPt());
	return distance <= 2;
}

void syn::UIWire::onMouseEnter(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering) {
	UIComponent::onMouseEnter(a_relCursor, a_diffCursor, a_isEntering);
	m_hoverDistance = pointLineDistance(m_window->cursorPos(), fromPt(), toPt());
	if (a_isEntering)
		m_window->getCircuitPanel()->setSelectedWire(this);
	else
		m_window->getCircuitPanel()->clearSelectedWire(this);
}

bool syn::UIWire::onMouseMove(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	UIComponent::onMouseMove(a_relCursor, a_diffCursor);
	m_hoverDistance = pointLineDistance(m_window->cursorPos(), fromPt(), toPt());
	return true;
}

bool syn::UIWire::onMouseUp(const Vector2i& a_relCursor, const Vector2i& a_diffCursor) {
	UIUnitControlContainer* selectedUnit = m_window->getCircuitPanel()->getUnit(m_window->cursorPos());
	m_isDragging = false;

	if (selectedUnit) {
		if (!m_isDraggingInput) {
			UIUnitPort* port = selectedUnit->getSelectedOutPort(m_window->cursorPos());
			if (port) {
				m_window->getCircuitPanel()->requestMoveConnection(this, port->getUnitId(), port->getPortId(), toUnit(), toPort());
				return true;
			}
		} else {
			UIUnitPort* port = selectedUnit->getSelectedInPort(m_window->cursorPos());
			if (port) {
				m_window->getCircuitPanel()->requestMoveConnection(this, fromUnit(), fromPort(), port->getUnitId(), port->getPortId());
				return true;
			}
		}
	}
	m_window->getCircuitPanel()->requestDeleteConnection(fromUnit(), fromPort(), toUnit(), toPort());	
	return true;
}

syn::UIComponent* syn::UIWire::onMouseDown(const Vector2i& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	Vector2i frompt = fromPt();
	Vector2i topt = toPt();

	int distanceToInput = (a_relCursor - topt).squaredNorm();
	int distanceToOutput = (a_relCursor - frompt).squaredNorm();
	m_isDragging = true;
	m_isDraggingInput = distanceToInput < distanceToOutput;
	return this;
}

Eigen::Vector2i syn::UIWire::destVector() const {
	return{ toUnit(),toPort() };
}

Eigen::Vector2i syn::UIWire::originVector() const {
	return{ fromUnit(),fromPort() };
}

Eigen::Vector2i syn::UIWire::fromPt() const {
	Vector2i fromPos;
	if (m_isDragging && !m_isDraggingInput) {
		fromPos = m_window->cursorPos();
	} else {
		fromPos = m_window->getCircuitPanel()->findUnit(m_fromUnit)->getOutPorts()[m_fromPort]->getAbsCenter();
	}
	return fromPos;
}

Eigen::Vector2i syn::UIWire::toPt() const {
	Vector2i toPos;
	if (m_isDragging && m_isDraggingInput) {
		toPos = m_window->cursorPos();
	} else {
		toPos = m_window->getCircuitPanel()->findUnit(m_toUnit)->getInPorts()[m_toPort]->getAbsCenter();
	}
	return toPos;
}

void syn::UIWire::draw(NVGcontext* a_nvg) {
	Vector2i fromPos = fromPt();
	Vector2i toPos = toPt();

	nvgBeginPath(a_nvg);
	if (m_window->getCircuitPanel()->getSelectedWire() == this) {
		nvgStrokeColor(a_nvg, Color(Vector3f{1.0f,0.0f,0.0f}));
	} else {
		nvgStrokeColor(a_nvg, Color(Vector3f{0.0f,0.0f,0.0f}));
	}
	nvgMoveTo(a_nvg, fromPos[0], fromPos[1]);
	nvgLineTo(a_nvg, toPos[0], toPos[1]);

	nvgStroke(a_nvg);
}
