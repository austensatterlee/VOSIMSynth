#include "UICoord.h"
#include "UIComponent.h"

synui::UICoord::UICoord() : UICoord({ 0,0 }) {}

synui::UICoord::UICoord(const Vector2i& a_globalPt) : m_globalPt(a_globalPt), m_referenceComponent(nullptr) {}

synui::UICoord::UICoord(const UICoord& a_pt, const UIComponent* a_referenceComponent) : m_globalPt(a_pt.globalCoord()), m_referenceComponent(a_referenceComponent) {}

Eigen::Vector2i synui::UICoord::globalCoord() const {
	return m_globalPt;
}

Eigen::Vector2i synui::UICoord::localCoord(const UIComponent* a_refComponent) const {
	return a_refComponent ? m_globalPt - a_refComponent->getAbsPos() : m_globalPt;
}

synui::UICoord synui::UICoord::toComponent(const UIComponent* a_refComponent) const {
	return UICoord(*this, a_refComponent);
}

const synui::UIComponent* synui::UICoord::refComponent() const { return m_referenceComponent; }

synui::UICoord::UICoord(const Vector2i& a_globalPt, const UIComponent* a_localComp) : m_globalPt(a_globalPt), m_referenceComponent(a_localComp) {}

synui::UICoord synui::operator+(const UICoord& a_coord, const Vector2i& a_offset) {
	return UICoord{ a_coord.globalCoord() + a_offset, a_coord.refComponent() };
}

synui::UICoord synui::operator-(const UICoord& a_coord, const Vector2i& a_offset) {
	return a_coord + (a_offset*-1);
}
