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
 *  \file MainGUI.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once

#include <common_serial.h>
#include "nanogui/formhelper.h"
#include <nanovg.h>

namespace nanogui
{
    class Window;
    class Widget;
    class Screen;
    class TabWidget;
}

namespace syn
{
    class VoiceManager;
    class UnitFactory;
}

struct GLFWwindow;

namespace synui
{
    class EnhancedWindow;
    class UnitEditorHost;
    class MainWindow;
    class CircuitWidget;

    class Logger
    {
    public:
        typedef std::function<void()> Listener;

        static Logger& instance()
        {
            static Logger singleton;
            return singleton;
        }

        void log(const std::string& a_channel, const std::string& a_msg)
        {
            m_logs.push_back({ a_channel, a_msg });
            if (m_channelMap.find(a_channel) == m_channelMap.end())
                m_channelMap[a_channel] = {};
            m_channelMap[a_channel].push_back(m_logs.size() - 1);

            for (auto& listener : m_listeners)
                listener();
        }

        const std::vector<std::pair<std::string, std::string> >& getLogs() { return m_logs; }

        void addListener(Listener a_listener)
        {
            m_listeners.push_back(a_listener);
        }

    private:
        Logger() {}
        std::vector<std::pair<std::string, std::string> > m_logs;
        std::unordered_map<std::string, std::vector<int> > m_channelMap;
        std::vector<Listener> m_listeners;
    };

    class MainGUI
    {
    public:
        MainGUI(MainWindow* a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf);
        virtual ~MainGUI();

        void show();
        void hide();
        void draw();

        const nanogui::Screen*  getScreen() const { return m_screen.get(); }

        operator json() const;
        MainGUI* load(const json& j);

        void reset();

    protected:
        void _createUnitSelector(nanogui::Widget* a_widget);
        void _createSettingsEditor(nanogui::Widget* a_widget, nanogui::FormHelper* a_fh);
        void _createLogViewer(nanogui::Widget * a_widget);

    private:
        MainWindow* m_window;
        nanogui::ref<nanogui::Screen> m_screen;
        syn::VoiceManager* m_vm;
        syn::UnitFactory* m_uf;

        // Widgets
        synui::EnhancedWindow* m_buttonPanel;
        synui::EnhancedWindow* m_settingsEditor; std::shared_ptr<nanogui::FormHelper> m_settingsFormHelper;
        synui::EnhancedWindow* m_logViewer;

        synui::EnhancedWindow* m_sidePanelL;
        nanogui::TabWidget* m_tabWidget;
        nanogui::Widget* m_unitSelector;
        UnitEditorHost* m_unitEditorHost;

        synui::EnhancedWindow* m_sidePanelR;
        CircuitWidget* m_circuit;
    };
}
