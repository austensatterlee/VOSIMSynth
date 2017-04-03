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

/**
 *  \file ContextMenu.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 01/2017
 */

#pragma once
#include "nanogui/nanogui.h"

namespace synui
{
    template <typename T>
    class ContextMenu : public nanogui::Widget
    {
    public:
        typedef std::function<void(const std::string&, const T&)> ItemSelectCallback;

        ContextMenu(Widget* a_parent, bool a_disposable, ItemSelectCallback a_callback = nullptr)
            : Widget(a_parent),
              m_callback(a_callback),
              m_disposable(a_disposable),
              m_activated(false)
        {
            m_gridLayout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Fill, 5, 5);
            setLayout(m_gridLayout);
        }

        void activate()
        {
            m_activated = true;
            setVisible(true);
            requestFocus();
        }

        void deactivate()
        {
            m_activated = false;
            setVisible(false);
            if(m_disposable)
                _dispose();
        }

        /**
         * \brief Add an item to the menu
         * If the item already exists, it will be removed and re-inserted.
         * \param a_name The display text of the item
         * \param a_value The associated value of the item
         */
        void addMenuItem(const std::string& a_name, const T& a_value);
        void removeMenuItem(const std::string& a_name);
        void removeMenuItem(int a_index);
        void setCallback(ItemSelectCallback a_callback) { m_callback = a_callback; }
        ItemSelectCallback getCallback() const { return m_callback; }

        bool mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers) override;
        void draw(NVGcontext* ctx) override;
    protected:
        std::pair<std::string, T>& getMenuItem_(const std::string& a_name);
    private:
        void _dispose();
    private:
        ItemSelectCallback m_callback;
        std::vector<std::pair<std::string, T>> m_menuItems;
        std::unordered_map<std::string, nanogui::Label*> m_itemMap;
        nanogui::GridLayout* m_gridLayout;
        bool m_disposable;
        bool m_activated;
    };

    template <typename T>
    void ContextMenu<T>::addMenuItem(const std::string& a_name, const T& a_value)
    {
        if (m_itemMap.find(a_name) != m_itemMap.end())
            removeMenuItem(a_name);
        auto p = std::make_pair(a_name, a_value);
        m_menuItems.push_back(p);
        m_itemMap[a_name] = new nanogui::Label(this, a_name);
    }

    template <typename T>
    void ContextMenu<T>::removeMenuItem(const std::string& a_name)
    {
        int index = -1;
        for (auto& p : m_menuItems)
        {
            index++;
            if (p.first == a_name)
                break;
        }
        if (index == -1)
            return;
        removeMenuItem(index);
    }

    template <typename T>
    void ContextMenu<T>::removeMenuItem(int a_index)
    {
        auto& item = m_menuItems[a_index];
        removeChild(m_itemMap[item.first]);
        m_itemMap.erase(item.first);
        m_menuItems.erase(m_menuItems.begin() + a_index);
    }

    template <typename T>
    bool ContextMenu<T>::mouseButtonEvent(const Eigen::Vector2i& p, int button, bool down, int modifiers)
    {
        if(!down){
            Eigen::Vector2i mousePos = p - mPos;
            for (const auto& w : m_itemMap)
            {
                if (w.second->contains(mousePos))
                {
                    if (m_callback)
                    {
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
    void ContextMenu<T>::draw(NVGcontext* ctx)
    {
        if(m_activated && !focused()){
            deactivate();
            return;
        }
        nvgSave(ctx);
        nvgResetScissor(ctx);
        nvgTranslate(ctx, mPos.x(), mPos.y());

        /* Draw background */
        nanogui::Color bgColor{0.0f, 1.0f};
        nvgBeginPath(ctx);
        nvgRect(ctx, 0, 0, mSize.x(), mSize.y());
        nvgFillColor(ctx, bgColor);
        nvgFill(ctx);

        /* Draw separators */

        /* Highlight selected item */
        nanogui::Color highlightColor(0.25, 1.0f);
        Eigen::Vector2i mousePos = screen()->mousePos() - absolutePosition();
        for (const auto& w : m_itemMap)
        {
            if (w.second->contains(mousePos))
            {
                nvgBeginPath(ctx);
                nvgRect(ctx, w.second->position().x(), w.second->position().y(), w.second->width(), w.second->height());
                nvgFillColor(ctx, highlightColor);
                nvgFill(ctx);
                break;
            }
        }

        nvgRestore(ctx);

        Widget::draw(ctx);
    }

    template <typename T>
    std::pair<std::string, T>& ContextMenu<T>::getMenuItem_(const std::string& a_name)
    {
        int index = -1;
        for (auto& p : m_menuItems)
        {
            index++;
            if (p.first == a_name)
                return p;
        }
        throw std::runtime_error("Item does not exist");
    }

    template <typename T>
    void ContextMenu<T>::_dispose()
    {
        parent()->removeChild(this);
    }
}
