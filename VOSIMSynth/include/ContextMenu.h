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

namespace synui {
    template <typename T>
    class ContextMenu : public nanogui::Widget {
        using Color = nanogui::Color;
    public:
        typedef std::function<void(const string&, const T&)> ItemSelectCallback;

        ContextMenu(Widget* a_parent, bool a_disposable, ItemSelectCallback a_callback = nullptr)
            : Widget(a_parent),
              m_callback(a_callback),
              m_disposable(a_disposable),
              m_activated(false)
        {
            m_gridLayout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Fill, 2, 1);
            setLayout(m_gridLayout);
        }

        void activate() {
            m_activated = true;
            setVisible(true);
            requestFocus();
        }

        void deactivate() {
            m_activated = false;
            setVisible(false);
            if (m_disposable)
                _dispose();
        }

        /**
         * \brief Add an item to the menu
         * If the item already exists, it will be removed and re-inserted.
         * \param a_name The display text of the item
         * \param a_value The associated value of the item
         */
        void addMenuItem(const string& a_name, const T& a_value);
        void removeMenuItem(const string& a_name);
        void removeMenuItem(int a_index);
        void setCallback(ItemSelectCallback a_callback) { m_callback = a_callback; }
        ItemSelectCallback getCallback() const { return m_callback; }

        bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) override;
        void draw(NVGcontext* ctx) override;
        bool focusEvent(bool focused) override;

    protected:
        std::pair<string, T>& getMenuItem_(const string& a_name);

    private:
        void _dispose();

    private:
        ItemSelectCallback m_callback;
        vector<std::pair<string, T>> m_menuItems;
        std::unordered_map<string, nanogui::Label*> m_itemMap;
        nanogui::GridLayout* m_gridLayout;
        bool m_disposable;
        bool m_activated;
    };

    template <typename T>
    void ContextMenu<T>::addMenuItem(const string& a_name, const T& a_value) {
        if (m_itemMap.find(a_name) != m_itemMap.end())
            removeMenuItem(a_name);
        auto p = std::make_pair(a_name, a_value);
        m_menuItems.push_back(p);
        m_itemMap[a_name] = new nanogui::Label(this, a_name);
        m_itemMap[a_name]->setShowShadow(true);
        m_itemMap[a_name]->setHeight(m_itemMap[a_name]->fontSize() * 1.5);
    }

    template <typename T>
    void ContextMenu<T>::removeMenuItem(const string& a_name) {
        int index = -1;
        for (auto& p : m_menuItems) {
            index++;
            if (p.first == a_name)
                break;
        }
        if (index == -1)
            return;
        removeMenuItem(index);
    }

    template <typename T>
    void ContextMenu<T>::removeMenuItem(int a_index) {
        auto& item = m_menuItems[a_index];
        removeChild(m_itemMap[item.first]);
        m_itemMap.erase(item.first);
        m_menuItems.erase(m_menuItems.begin() + a_index);
    }

    template <typename T>
    bool ContextMenu<T>::mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers) {
        if (!down) {
            Vector2i mousePos = p - mPos;
            for (const auto& w : m_itemMap) {
                if (w.second->contains(mousePos)) {
                    if (m_callback) {
                        const auto& item = getMenuItem_(w.first);
                        m_callback(item.first, item.second);
                    }
                    deactivate();
                    return true;
                }
            }
        }
        return true;
    }

    template <typename T>
    void ContextMenu<T>::draw(NVGcontext* ctx) {
        nvgSave(ctx);
        nvgResetScissor(ctx);
        nvgTranslate(ctx, mPos.x(), mPos.y());

        /* Draw shadow */
        int dr = 3;
        drawRectShadow(ctx, 0, 0, mSize.x(), mSize.y(), 0, dr, 1, mTheme->get<Color>("/shadow"), mTheme->get<Color>("/transparent"));

        /* Draw background */
        Color bgColor = theme()->get<Color>("/button/unfocused/grad-top", {0.3f, 0.9f});
        nvgBeginPath(ctx);
        nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
        nvgFillColor(ctx, bgColor);
        nvgFill(ctx);

        Color highlightColor = theme()->get<Color>("/button/focused/grad-top", {0.15f, 1.0f});
        for (const auto& w : m_itemMap) {
            /* Draw outline */
            nvgBeginPath(ctx);
            nvgStrokeWidth(ctx, 1.0f);
            nvgRect(ctx, 0.5f, w.second->position().y() + 1.5f, mSize.x() - 1, w.second->height() - 2);
            nvgStrokeColor(ctx, mTheme->get<Color>("/border/light"));
            nvgStroke(ctx);

            nvgBeginPath(ctx);
            nvgRect(ctx, 0.5f, w.second->position().y() + 0.5f, mSize.x() - 1, w.second->height() - 2);
            nvgStrokeColor(ctx, mTheme->get<Color>("/border/dark"));
            nvgStroke(ctx);

            /* Highlight selected item */
            if (w.second->mouseFocus()) {
                nvgBeginPath(ctx);
                nvgRect(ctx, 1, w.second->position().y() + 1, width() - 2, w.second->height() - 2);
                nvgFillColor(ctx, highlightColor);
                nvgFill(ctx);
            }
        }

        nvgRestore(ctx);

        Widget::draw(ctx);
    }

    template <typename T>
    bool ContextMenu<T>::focusEvent(bool a_focused) {
        Widget::focusEvent(a_focused);
        if (!a_focused && m_activated && m_disposable)
            deactivate();
        return true;
    }

    template <typename T>
    std::pair<string, T>& ContextMenu<T>::getMenuItem_(const string& a_name) {
        int index = -1;
        for (auto& p : m_menuItems) {
            index++;
            if (p.first == a_name)
                return p;
        }
        throw std::runtime_error("Item does not exist");
    }

    template <typename T>
    void ContextMenu<T>::_dispose() {
        if (parent())
            parent()->removeChild(this);
    }
}
