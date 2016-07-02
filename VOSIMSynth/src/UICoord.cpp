#include "UICoord.h"
#include "UIComponent.h"

synui::UICoord::UICoord() : UICoord({ 0,0 }) {}

synui::UICoord::UICoord(const Vector2i& a_globalPt) : 
	m_globalPt(a_globalPt)
{}

Eigen::Vector2i synui::UICoord::globalCoord() const {
	return m_globalPt;
}

Eigen::Vector2i synui::UICoord::localCoord(const UIComponent* a_refComponent) const {
	return a_refComponent ? m_globalPt - a_refComponent->getAbsPos() : m_globalPt;
}
