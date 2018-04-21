#include "vosimsynth/widgets/ContextMenu.h"
#include "vosimsynth/VOSIMTheme.h"
#include <nanogui/layout.h>
#include <nanogui/screen.h>
#include <nanogui/label.h>
#include <nanogui/entypo.h>

using nanogui::Color;
using nanogui::Label;
using Anchor = nanogui::AdvancedGridLayout::Anchor;

namespace synui {
    ContextMenu::ContextMenu(Widget* a_parent, bool a_disposable)
        : Widget(a_parent),
          m_highlightedSubmenu(nullptr),
          m_disposable(a_disposable),
          m_activated(false)
    {
        m_itemContainer = new Widget(this);
        m_itemContainer->setPosition({0,0});
        m_itemLayout = new nanogui::AdvancedGridLayout({0,0,0}, {}, 2);
        m_itemContainer->setLayout(m_itemLayout);
        setVisible(false);
    }

    void ContextMenu::activate(const Vector2i& a_pos) {
        m_activated = true;
        setVisible(true);
        setPosition(a_pos);
        requestFocus();
        screen()->performLayout();
    }

    void ContextMenu::deactivate() {
        m_activated = false;
        setVisible(false);
        screen()->performLayout();
        if (m_disposable)
            _dispose();
    }

    void ContextMenu::addMenuItem(const std::string& a_name, const std::function<void()>& a_value, int a_icon) {
        m_items[a_name] = a_value;
        auto lbl = new Label(m_itemContainer, a_name);
        m_labels[a_name] = lbl;
        lbl->setShowShadow(true);
        lbl->setFontSize(theme()->get("/ContextMenu/text-size", 20));
        lbl->setHeight(lbl->fontSize() * 2);
        m_itemLayout->appendRow(0);
        m_itemLayout->setAnchor(lbl, Anchor{1,m_itemLayout->rowCount() - 1, 1, 1});
        if (a_icon > 0) {
            auto iconLbl = new Label(m_itemContainer, nanogui::utf8(a_icon).data(), "icons");
            iconLbl->setFontSize(theme()->get("/ContextMenu/text-size", 20));
            iconLbl->setHeight(iconLbl->fontSize() * 2);
            m_itemLayout->setAnchor(iconLbl, Anchor{ 0,m_itemLayout->rowCount() - 1,1,1 });
        }
    }

    ContextMenu* ContextMenu::addSubMenu(const std::string& a_name, int a_icon) {
        m_submenus[a_name] = new ContextMenu(this, false);
        auto lbl1 = new Label(m_itemContainer, a_name);
        auto lbl2 = new Label(m_itemContainer, nanogui::utf8(ENTYPO_ICON_CHEVRON_THIN_RIGHT).data(), "icons");
        m_labels[a_name] = lbl1;
        lbl1->setShowShadow(true);
        lbl1->setFontSize(theme()->get("/ContextMenu/text-size", 20));
        lbl1->setHeight(lbl1->fontSize() * 2);
        lbl2->setShowShadow(true);
        lbl2->setFontSize(theme()->get("/ContextMenu/text-size", 20));
        lbl2->setHeight(lbl2->fontSize() * 2);
        m_itemLayout->appendRow(0);
        m_itemLayout->setAnchor(lbl1, Anchor{1, m_itemLayout->rowCount() - 1});
        m_itemLayout->setAnchor(lbl2, Anchor{2, m_itemLayout->rowCount() - 1});
        if (a_icon > 0) {
            auto iconLbl = new Label(m_itemContainer, nanogui::utf8(a_icon).data(), "icons");
            iconLbl->setFontSize(theme()->get("/ContextMenu/text-size", 20));
            iconLbl->setHeight(iconLbl->fontSize() * 2);
            m_itemLayout->setAnchor(iconLbl, Anchor{ 0, m_itemLayout->rowCount() - 1 });
        }
        return m_submenus[a_name];
    }

    Vector2i ContextMenu::preferredSize(NVGcontext* ctx) const {
        Vector2i base = m_itemContainer->position() + m_itemContainer->preferredSize(ctx);
        if (m_highlightedSubmenu) {
            base = base.cwiseMax(m_highlightedSubmenu->position() + m_highlightedSubmenu->size());
        }
        return base;
    }

    bool ContextMenu::mouseEnterEvent(const Vector2i& p, bool enter) {
        Widget::mouseEnterEvent(p, enter);
        if (!enter && m_highlightedSubmenu) {
            m_highlightedSubmenu->setVisible(false);
            m_highlightedSubmenu = nullptr;
        }
        return true;
    }

    bool ContextMenu::mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button, int modifiers) {
        const Vector2i mousePos = p - mPos;
        for (const auto& w : m_labels) {
            // Deactivate old highlighted submenu, activate new submenu
            if (isRowSelected_(w.first, mousePos)) {
                // Deactivate current submenu unless we are still hovering over it.
                if (m_highlightedSubmenu) {
                    if (!(isSubMenu_(w.first) && m_submenus[w.first] == m_highlightedSubmenu)) {
                        m_highlightedSubmenu->setVisible(false);
                        m_highlightedSubmenu = nullptr;
                    }
                }
                if (isSubMenu_(w.first)) {
                    m_highlightedSubmenu = m_submenus[w.first];
                    m_highlightedSubmenu->activate(m_itemContainer->position() + Vector2i{ m_itemContainer->width()-1, m_labels[w.first]->position().y()+1 });
                    break;
                }
            }
        }
        Widget::mouseMotionEvent(p, rel, button, modifiers);
        return true;
    }

    bool ContextMenu::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) {
        const Vector2i mousePos = p - mPos;
        if (!down) {
            for (const auto& w : m_labels) {
                if (isRowSelected_(w.first, mousePos) && !isSubMenu_(w.first)) {
                    const std::function<void()> cb = m_items[w.first];
                    deactivate();
                    if (cb) {
                        cb();
                    }
                    return true;
                }
            }
            if (m_highlightedSubmenu) {
                m_highlightedSubmenu->mouseButtonEvent(p - position(), button, down, modifiers);
                // Deactivate self if item in submenu was selected
                if(!m_highlightedSubmenu->m_activated) {
                    deactivate();
                    return true;
                }
            }
        }
        return true;
    }

    void ContextMenu::draw(NVGcontext* ctx) {
        nvgSave(ctx);
        nvgResetScissor(ctx);
        nvgTranslate(ctx, mPos.x(), mPos.y());

        const int w = m_itemContainer->position().x() + m_itemContainer->width();
        const int h = m_itemContainer->position().y() + m_itemContainer->height();

        /* Draw shadow */
        const int dr = 5;
        drawRectShadow(ctx, 0, 0, w, h, 0, dr, 1, mTheme->get<Color>("/shadow"), mTheme->get<Color>("/transparent"));

        /* Draw background */
        const Color bgColor = theme()->get<Color>("/ContextMenu/bg-color", { 0.3f, 0.9f });
        nvgBeginPath(ctx);
        nvgRect(ctx, 0, 0, w, h);
        nvgFillColor(ctx, bgColor);
        nvgFill(ctx);

        /* Draw margin background */
        if (!m_labels.empty()) {
            const auto lbl = m_labels.begin()->second;
            const Color marginColor = theme()->get<Color>("/ContextMenu/margin-color", { 0.2f, 0.9f });
            nvgBeginPath(ctx);
            nvgRect(ctx, 0, 0, lbl->position().x()-1, h);
            nvgFillColor(ctx, marginColor);
            nvgFill(ctx);
        }

        /* Draw outline */
        nvgBeginPath(ctx);
        nvgStrokeWidth(ctx, 1.0f);
        nvgRect(ctx, 0.5f, 1.5f, w - 1, h - 2);
        nvgStrokeColor(ctx, mTheme->get<Color>("/border/light"));
        nvgStroke(ctx);

        nvgBeginPath(ctx);
        nvgRect(ctx, 0.5f, 0.5f, w - 1, h - 0.5f);
        nvgStrokeColor(ctx, mTheme->get<Color>("/border/dark"));
        nvgStroke(ctx);

        const Color highlightColor = theme()->get<Color>("/ContextMenu/hover-color", { 0.15f, 1.0f });
        const Vector2i mousePos = screen()->mousePos() - absolutePosition();
        for (const auto& pair : m_labels) {
            const auto lbl = pair.second;

            /* Highlight selected item and/or currently active submenu */
            if (isRowSelected_(pair.first, mousePos) || (isSubMenu_(pair.first) && m_submenus[pair.first]==m_highlightedSubmenu)) {
                nvgBeginPath(ctx);
                nvgRect(ctx, 1, lbl->position().y() + 1, w - 2, lbl->height() - 2);
                nvgFillColor(ctx, highlightColor);
                nvgFill(ctx);
            }
        }
        nvgRestore(ctx);

        Widget::draw(ctx);
    }

    bool ContextMenu::focusEvent(bool a_focused) {
        Widget::focusEvent(a_focused);
        // Deactivate when focus is lost
        if (!a_focused && m_activated) {
            deactivate();
        }
        return true;
    }

    bool ContextMenu::isSubMenu_(const std::string& a_name) {
        return m_submenus.find(a_name) != m_submenus.end();
    }

    bool ContextMenu::isRowSelected_(const std::string& a_name, const Vector2i& p) const {
        if (m_labels.find(a_name) == m_labels.end())
            return false;
        const auto w = m_labels.at(a_name);
        const int lblLeft = m_itemContainer->position().x();
        const int lblRight = m_itemContainer->position().x() + m_itemContainer->width();
        const int lblTop = m_itemContainer->position().y() + w->position().y();
        const int lblBot = m_itemContainer->position().y() + w->position().y() + w->height();
        return p.y() >= lblTop && p.y() < lblBot && p.x() >= lblLeft && p.x() < lblRight;
    }

    void ContextMenu::_dispose() {
        if (parent())
            parent()->removeChild(this);
    }
}