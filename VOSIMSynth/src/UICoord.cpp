#include "UICoord.h"
#include "UIComponent.h"

syn::UICoord::UICoord() : UICoord({ 0,0 }) {}

syn::UICoord::UICoord(const Vector2i& a_globalPt) : m_globalPt(a_globalPt), m_referenceComponent(nullptr) {}

syn::UICoord::UICoord(const UICoord& a_pt, const UIComponent* a_referenceComponent) : m_globalPt(a_pt.globalCoord()), m_referenceComponent(a_referenceComponent) {}

Eigen::Vector2i syn::UICoord::globalCoord() const {
	return m_globalPt;
}

Eigen::Vector2i syn::UICoord::localCoord(const UIComponent* a_refComponent) const {
	return a_refComponent ? m_globalPt - a_refComponent->getAbsPos() : m_globalPt;
}

syn::UICoord syn::UICoord::toComponent(const UIComponent* a_refComponent) const {
	return UICoord(*this, a_refComponent);
}

const syn::UIComponent* syn::UICoord::refComponent() const { return m_referenceComponent; }

syn::UICoord::UICoord(const Vector2i& a_globalPt, const UIComponent* a_localComp) : m_globalPt(a_globalPt), m_referenceComponent(a_localComp) {}

syn::UICoord syn::operator+(const UICoord& a_coord, const Vector2i& a_offset) {
	return UICoord{ a_coord.globalCoord() + a_offset, a_coord.refComponent() };
}

syn::UICoord syn::operator-(const UICoord& a_coord, const Vector2i& a_offset) {
	return a_coord + (a_offset*-1);
}
