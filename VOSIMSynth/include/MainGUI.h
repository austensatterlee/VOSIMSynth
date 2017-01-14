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
            if(m_logs.size()<m_maxLogs-1){
                m_logs.push_back({ a_channel, a_msg });
                m_nextIndex++;
            }else{
                m_logs[m_nextIndex] = {a_channel, a_msg};
                m_nextIndex = m_nextIndex % m_maxLogs;
            }

            for (auto& listener : m_listeners)
                listener();
        }

        const std::vector<std::pair<std::string, std::string> >& getLogs() { return m_logs; }

        void addListener(Listener a_listener)
        {
            m_listeners.push_back(a_listener);
        }

        void reset()
        {
            m_listeners.clear();
            m_logs.clear();
            m_nextIndex = 0;
        }

    private:
        Logger() : m_maxLogs(1000), m_nextIndex(0) {}
        std::vector<std::pair<std::string, std::string> > m_logs;
        std::vector<Listener> m_listeners;
        int m_maxLogs;
        int m_nextIndex;
    };

    class SerializableFormHelper : public nanogui::FormHelper
    {
    public:
        SerializableFormHelper(nanogui::Screen* screen)
            : FormHelper(screen) {}

        template <typename Type> 
        nanogui::detail::FormWidget<Type>* addSerializableVariable(const std::string &label, const std::function<void(const Type &)> &setter, const std::function<Type()> &getter, bool editable = true)
        {
            auto ret = nanogui::FormHelper::addVariable(label, setter, getter, editable);
            auto getterSerializer = [getter]()->json{
                json j = getter();
                return j;
            };
            auto setterSerializer = [setter](const json& j)
            {
                setter(j.get<Type>());
            };
            m_getterSerializers[label] = getterSerializer;
            m_setterSerializers[label] = setterSerializer;
            return ret;
        }

        operator json() const
        {
            json j;
            for(auto& g : m_getterSerializers)
            {
                j[g.first] = g.second();
            }
            return j;
        }

        SerializableFormHelper* load(const json& j)
        {
            for(auto& s : m_setterSerializers)
            {
                s.second(j[s.first]);
            }
            return this;
        }

    protected:
        std::map<std::string, std::function<json()> > m_getterSerializers;
        std::map<std::string, std::function<void(const json&)> > m_setterSerializers;
    };

    class MainGUI
    {
    public:
        MainGUI(MainWindow* a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_uf);
        virtual ~MainGUI();

        void show();
        void hide();
        void draw();

        nanogui::Screen*  getScreen() { return m_screen.get(); }

        operator json() const;
        MainGUI* load(const json& j);

        void reset();
        void resize(int a_w, int a_h);
    protected:
        void createUnitSelector_(nanogui::Widget* a_widget);
        void createSettingsEditor_(nanogui::Widget* a_widget, SerializableFormHelper* a_fh);
        void createLogViewer_(nanogui::Widget * a_widget);
        void initialize_(GLFWwindow* a_window);

    private:
        MainWindow* m_window;
        nanogui::ref<nanogui::Screen> m_screen;
        syn::VoiceManager* m_vm;
        syn::UnitFactory* m_uf;

        // Widgets
        synui::EnhancedWindow* m_buttonPanel;
        synui::EnhancedWindow* m_settingsEditor; std::shared_ptr<synui::SerializableFormHelper> m_settingsFormHelper;
        synui::EnhancedWindow* m_logViewer;

        synui::EnhancedWindow* m_sidePanelL;
        nanogui::TabWidget* m_tabWidget;
        nanogui::Widget* m_unitSelector;
        UnitEditorHost* m_unitEditorHost;

        synui::EnhancedWindow* m_sidePanelR;
        CircuitWidget* m_circuit;
    };
}
