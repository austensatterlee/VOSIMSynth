#include "vosimsynth/widgets/ContextMenu.h"
namespace synui {
    void ContextMenu::addMenuItem(const std::string& a_name, const std::function<void()>& a_value) {
        m_items[a_name] = a_value;
        m_menu.push_back(a_name);
        auto lbl = new nanogui::Label(m_itemContainer, a_name);
        m_labels[a_name] = lbl;
        lbl->setShowShadow(true);
        lbl->setFontSize(theme()->get("/ContextMenu/text-size", 20));
        lbl->setHeight(m_labels[a_name]->fontSize() * 2);        
    }

    ContextMenu* ContextMenu::addSubMenu(const std::string& a_name) {
        if (find(m_menu.begin(), m_menu.end(), a_name) != m_menu.end())
            return nullptr;        
        m_menu.push_back(a_name);
        m_submenus[a_name] = new ContextMenu(this, true);
        auto w = m_labels[a_name] = new nanogui::Widget(m_itemContainer);
        w->setLayout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 2));
        auto lbl1 = new nanogui::Label(w, a_name);
        auto lbl2 = new nanogui::Label(w, nanogui::utf8(ENTYPO_ICON_CHEVRON_THIN_RIGHT).data(), "icons");
        lbl1->setShowShadow(true);
        lbl1->setFontSize(theme()->get("/ContextMenu/text-size", 20));
        lbl1->setHeight(m_labels[a_name]->fontSize() * 2);
        lbl2->setTextAlign(nanogui::Label::Alignment::Right);
        lbl2->setShowShadow(true);
        lbl2->setFontSize(theme()->get("/ContextMenu/text-size", 20));
        lbl2->setHeight(m_labels[a_name]->fontSize() * 2);
        return m_submenus[a_name];
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
        for (const auto& w : m_labels) {
            // Deactivate old highlighted submenu, activate new submenu
            if (w.second->mouseFocus()) {
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
                }
            }
        }
        Widget::mouseMotionEvent(p, rel, button, modifiers);
        return true;
    }

    bool ContextMenu::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) {
        if (!down) {
            Vector2i mousePos = p - mPos;
            for (const auto& w : m_labels) {
                if (w.second->contains(mousePos) && !isSubMenu_(w.first)) {
                    if (m_items[w.first]) {
                        m_items[w.first]();
                    }
                    deactivate();
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

        int w = m_itemContainer->position().x() + m_itemContainer->width();
        int h = m_itemContainer->position().y() + m_itemContainer->height();

        /* Draw shadow */
        int dr = 5;
        drawRectShadow(ctx, 0, 0, w, h, 0, dr, 1, mTheme->get<Color>("/shadow"), mTheme->get<Color>("/transparent"));

        /* Draw background */
        Color bgColor = theme()->get<Color>("/ContextMenu/bgColor", { 0.3f, 0.9f });
        nvgBeginPath(ctx);
        nvgRect(ctx, 0, 0, w, h);
        nvgFillColor(ctx, bgColor);
        nvgFill(ctx);

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

        Color highlightColor = theme()->get<Color>("/ContextMenu/hoverColor", { 0.15f, 1.0f });
        for (const auto& pair : m_labels) {
            auto lbl = pair.second;

            /* Highlight selected item and/or currently active submenu */
            if (lbl->mouseFocus() || (isSubMenu_(pair.first) && m_submenus[pair.first]==m_highlightedSubmenu)) {
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

    void ContextMenu::_dispose() {
        if (parent())
            parent()->removeChild(this);
    }
}