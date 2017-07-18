/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include "nanogui/nanogui.h"
#include "UI.h"

namespace synui {
    class ContextMenu : public nanogui::Widget {
        using Color = nanogui::Color;
    public:

        ContextMenu(Widget* a_parent, bool a_disposable)
            : Widget(a_parent),
              m_highlightedSubmenu(nullptr),
              m_disposable(a_disposable),
              m_activated(false)
        {
            m_itemContainer = new Widget(this);
            m_itemContainer->setPosition({ 10,0 });
            auto itemLayout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Fill, 1, 0);
            m_itemContainer->setLayout(itemLayout);
            setVisible(false);
        }

        Vector2i preferredSize(NVGcontext* ctx) const override { 
            Vector2i base = m_itemContainer->position() + m_itemContainer->preferredSize(ctx);
            if (m_highlightedSubmenu) {
                base = base.cwiseMax(m_highlightedSubmenu->position() + m_highlightedSubmenu->size());
            }           
            return base;
        }

        void activate(const Vector2i& a_pos) {
            m_activated = true;
            setVisible(true);
            setPosition(a_pos);
            requestFocus();
            screen()->performLayout();
        }

        void deactivate() {
            m_activated = false;
            setVisible(false);
            screen()->performLayout();
            if (m_disposable)
                _dispose();
        }

        /**
         * Add an item to the menu.
         * 
         * \param a_name The display text of the item.
         */
        void addMenuItem(const std::string& a_name, const std::function<void()>& a_cb);

        /**
         * Add a submenu to the menu.
         *
         * \param a_name The display text of the item.
         * \returns nullptr if a submenu or item already exists under the given name.
         */
        ContextMenu* addSubMenu(const std::string& a_name);

        bool mouseEnterEvent(const Vector2i& p, bool enter) override;
        bool mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button, int modifiers) override;
        bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) override;
        void draw(NVGcontext* ctx) override;
        bool focusEvent(bool focused) override;

    protected:
        bool isSubMenu_(const std::string& a_name);

    private:
        void _dispose();

    private:
        Widget* m_itemContainer;
        std::vector<std::string> m_menu;
        std::unordered_map<std::string, std::function<void()>> m_items;
        std::unordered_map<std::string, ContextMenu*> m_submenus;
        std::unordered_map<std::string, nanogui::Widget*> m_labels;
        ContextMenu* m_highlightedSubmenu;
        bool m_disposable;
        bool m_activated;
    };    
}
