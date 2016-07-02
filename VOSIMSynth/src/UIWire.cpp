#include "UIWire.h"
#include "UICoord.h"
#include <UIUnitContainer.h>
#include <UICircuitPanel.h>
#include <UIUnitPort.h>
#include <Theme.h>


synui::UIWire::UIWire(MainWindow* a_window, UICircuitPanel* a_circuitPanel, int a_fromUnit, int a_fromPort, int a_toUnit, int a_toPort) :
	UIComponent{a_window},
	m_fromUnit(a_fromUnit),
	m_fromPort(a_fromPort),
	m_toUnit(a_toUnit),
	m_toPort(a_toPort),
	m_circuitPanel(a_circuitPanel),
	m_hoverDistance(INFINITY)
{}

bool synui::UIWire::contains(const UICoord& a_pt) const {
	// Otherwise compute distance to line and verify it is below a threshold
	double distance = pointLineDistance(a_pt.localCoord(this), fromPt().localCoord(this), toPt().localCoord(this));
	return distance <= 2;
}

void synui::UIWire::onMouseEnter(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isEntering) {
	UIComponent::onMouseEnter(a_relCursor, a_diffCursor, a_isEntering);
	m_hoverDistance = pointLineDistance(a_relCursor.localCoord(this), fromPt().localCoord(this), toPt().localCoord(this));
	if (a_isEntering)
		m_circuitPanel->setSelectedWire(this);
	else
		m_circuitPanel->clearSelectedWire(this);
}

bool synui::UIWire::onMouseMove(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	UIComponent::onMouseMove(a_relCursor, a_diffCursor);
	m_hoverDistance = pointLineDistance(a_relCursor.localCoord(this), fromPt().localCoord(this), toPt().localCoord(this));
	return false;
}

bool synui::UIWire::onMouseUp(const UICoord& a_relCursor, const Vector2i& a_diffCursor) {
	UIUnitContainer* selectedUnit = m_circuitPanel->findUnitContainer(a_relCursor);
	m_isDragging = false;

	if (selectedUnit) {
		if (!m_isDraggingInput) {
			UIUnitPort* port = selectedUnit->getSelectedOutPort(a_relCursor);
			if (port) {
				m_circuitPanel->requestMoveConnection(this, port->getUnitId(), port->getPortId(), toUnit(), toPort());
				return true;
			}
		} else {
			UIUnitPort* port = selectedUnit->getSelectedInPort(a_relCursor);
			if (port) {
				m_circuitPanel->requestMoveConnection(this, fromUnit(), fromPort(), port->getUnitId(), port->getPortId());
				return true;
			}
		}
	}
	m_circuitPanel->requestDeleteConnection(fromUnit(), fromPort(), toUnit(), toPort());
	return true;
}

synui::UIComponent* synui::UIWire::onMouseDown(const UICoord& a_relCursor, const Vector2i& a_diffCursor, bool a_isDblClick) {
	Vector2i frompt = fromPt().localCoord(this);
	Vector2i topt = toPt().localCoord(this);

	int distanceToInput = (a_relCursor.globalCoord() - topt).squaredNorm();
	int distanceToOutput = (a_relCursor.globalCoord() - frompt).squaredNorm();
	m_isDragging = true;
	m_isDraggingInput = distanceToInput < distanceToOutput;
	return this;
}

Eigen::Vector2i synui::UIWire::destVector() const {
	return{ toUnit(),toPort() };
}

Eigen::Vector2i synui::UIWire::originVector() const {
	return{ fromUnit(),fromPort() };
}

synui::UICoord synui::UIWire::fromPt() const {
	Vector2i fromPos;
	if (m_isDragging && !m_isDraggingInput) {
		fromPos = m_window->cursorPos();
	} else {
		UIUnitPort* port = m_circuitPanel->findUnitContainer(m_fromUnit)->getOutPorts()[m_fromPort];
		fromPos = port->getAbsPos() + Vector2i{port->size().x(), port->size().y() / 2};
	}
	return UICoord{ fromPos };
}

synui::UICoord synui::UIWire::toPt() const {
	Vector2i toPos;
	if (m_isDragging && m_isDraggingInput) {
		toPos = m_window->cursorPos();
	} else {
		UIUnitPort* port = m_circuitPanel->findUnitContainer(m_toUnit)->getInPorts()[m_toPort];
		toPos = port->getAbsPos() + Vector2i{ 0,port->size().y()/2 };
	}
	return UICoord{ toPos };
}

void synui::UIWire::draw(NVGcontext* a_nvg) {
	Vector2i fromPos = fromPt().localCoord(this);
	Vector2i toPos = toPt().localCoord(this);
	Vector2f rawWire = (toPos - fromPos).cast<float>();
	float angle = atan2(rawWire.y(),rawWire.x());
	nvgTranslate(a_nvg, fromPos.x(), fromPos.y());
	nvgRotate(a_nvg, angle);

	nvgLineCap(a_nvg, NVG_ROUND);
	// Draw wire
	Color wireColor;
	if (m_circuitPanel->getSelectedWire() == this)
		wireColor = theme()->mSelectedWireColor;
	else
		wireColor = theme()->mWireColor;
	NVGpaint wirePaint = nvgLinearGradient(a_nvg, 0.0f, 0.0f, 0.0f, 0.75f, wireColor, theme()->mWireInnerShadowColor);
	nvgStrokeWidth(a_nvg, 2.0f);
	nvgStrokePaint(a_nvg, wirePaint);

	nvgBeginPath(a_nvg);
	nvgMoveTo(a_nvg, 0.0f, 0.0f);
	nvgLineTo(a_nvg, rawWire.norm(), 0.0f);
	nvgStroke(a_nvg); 
}
