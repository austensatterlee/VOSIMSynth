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
#include "vosimsynth/UI.h"
#include <nanogui/widget.h>
#include <unordered_map>

namespace synui {
    class ContextMenu : public nanogui::Widget {
    public:
        /**
         * \brief Construct a new ContextMenu.
         * \param a_parent Parent widget.
         * \param a_disposable When true, the context menu will be destroyed upon deactivation.
         */
        ContextMenu(Widget* a_parent, bool a_disposable);

        /**
         * \brief Activate the context menu at the given position.
         * 
         * Make the context menu visible. When an item is clicked, the context menu will be deactivated.
         * 
         * \param a_pos The desired position of the top left corner of the context menu, relative to the parent.
         */
        void activate(const Vector2i& a_pos);

        /**
         * \brief Deactivate the context menu.
         * 
         * This method is called automatically after an item is clicked. If the
         * context menu is disposable, it will be removed from its parent and
         * destroyed.
         */
        void deactivate();


        /**
         * \brief Add an item to the menu. The callback is called when the item is clicked.
         * \param a_name Name of the item. The name is displayed in the context menu.
         * \param a_cb Callback to be executed when the item is clicked.
         * \param a_icon Optional icon to display to the left of the label.
         */
        void addMenuItem(const std::string& a_name, const std::function<void()>& a_cb, int a_icon=0);

        /**
         * Add a submenu to the menu.
         *
         * \param a_name The display text of the item.
         * \returns nullptr if a submenu or item already exists under the given name.
         * \param a_icon Optional icon to display to the left of the label.
         */
        ContextMenu* addSubMenu(const std::string& a_name, int a_icon = 0);

        Vector2i preferredSize(NVGcontext* ctx) const override;
        bool mouseEnterEvent(const Vector2i& p, bool enter) override;
        bool mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button, int modifiers) override;
        bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) override;
        void draw(NVGcontext* ctx) override;
        bool focusEvent(bool focused) override;

    protected:
        /**
         * \brief Determine if an item opens a new context menu.
         * \param a_name Name of the item.
         */
        bool isSubMenu_(const std::string& a_name);
        /**
         * \brief Determine if a row contains the point `p`.
         * \param a_name Name of the item that is on the desired row.
         * \param p Point relative to self.
         */
        bool isRowSelected_(const std::string& a_name, const Vector2i& p) const;

    private:
        void _dispose();

    private:
        Widget* m_itemContainer;
        nanogui::AdvancedGridLayout* m_itemLayout;
        std::unordered_map<std::string, std::function<void()>> m_items;
        std::unordered_map<std::string, ContextMenu*> m_submenus;
        std::unordered_map<std::string, Widget*> m_labels;
        ContextMenu* m_highlightedSubmenu;
        bool m_disposable;
        bool m_activated;
    };    
}
