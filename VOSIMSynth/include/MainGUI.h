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

#include <vosimlib/common_serial.h>
#include <nanogui/formhelper.h>
#include <map>
#include <memory>

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

    class SerializableFormHelper : public nanogui::FormHelper
    {
    public:
        SerializableFormHelper(nanogui::Screen* screen)
            : FormHelper(screen) {}

        /**
         * \brief Adds a variable that will be automatically serialized by this classes ::load and ::operator json methods.
         * 
         * \param name A name that will be used to serialize this variable. Will (ideally) never change.
         * \param label The label for this variable that will be displayed in the gui.
         */
        template <typename Type> 
        nanogui::detail::FormWidget<Type>* addSerializableVariable(const std::string &name, const std::string &label, const std::function<void(const Type &)> &setter, const std::function<Type()> &getter, bool editable = true)
        {
            auto ret = FormHelper::addVariable(label, setter, getter, editable);
            auto getterSerializer = [getter]()->json{
                json j = getter();
                return j;
            };
            auto setterSerializer = [setter](const json& j)
            {
                setter(j.get<Type>());
            };
            m_getterSerializers[name] = getterSerializer;
            m_setterSerializers[name] = setterSerializer;
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
                const json& curr = j.value(s.first, json());
                if(!curr.empty())
                    s.second(curr);
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
        ~MainGUI();
        
        void setGLFWWindow(GLFWwindow* a_window);

        nanogui::Screen* getScreen() { return m_screen.get(); }

        void show();
        void hide();
        void draw();

        operator json() const;
        MainGUI* load(const json& j);

        void reset(); 
        void resize(int a_w, int a_h);

        CircuitWidget* circuitWidget() const { return m_circuitWidget; }
    protected:
        void createUnitSelector_(nanogui::Widget* a_widget);
        void createSettingsEditor_(nanogui::Widget* a_widget, SerializableFormHelper* a_fh);
        void createLogViewer_(nanogui::Widget * a_widget);
        void createOscilloscopeViewer_(nanogui::Widget* a_widget);

    private:
        MainWindow* m_window;
        nanogui::ref<nanogui::Screen> m_screen;
        syn::VoiceManager* m_vm;
        syn::UnitFactory* m_uf;

        // Widgets
        EnhancedWindow* m_buttonPanel;
        EnhancedWindow* m_settingsEditor; std::shared_ptr<SerializableFormHelper> m_settingsFormHelper;
        EnhancedWindow* m_logViewer;
        EnhancedWindow* m_oscViewer;

        EnhancedWindow* m_sidePanelL;
        nanogui::TabWidget* m_tabWidget;
        nanogui::Widget* m_unitSelector;
        UnitEditorHost* m_unitEditorHost;

        EnhancedWindow* m_sidePanelR;
        CircuitWidget* m_circuitWidget;
    };
}
