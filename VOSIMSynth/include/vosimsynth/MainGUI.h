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
#include "ChildWindow.h"
#include "Signal.h"
#include "SerializableFormHelper.h"
#include <nanogui/messagedialog.h>
#include <memory>

#define MAX_GUI_MSG_QUEUE_SIZE 64

namespace nanogui {
    class Window;
    class Widget;
    class Screen;
    class TabWidget;
}

namespace syn {
    class VoiceManager;
    class UnitFactory;
}

struct GLFWwindow;

namespace synui {
    class EnhancedWindow;
    class UnitEditorHost;
    class CircuitWidget;
    using DlgType = nanogui::MessageDialog::Type;    

    /**
     * Handles the logic of creating the GUI and gluing the components toegether.
     */
    class MainGui : public ChildWindow {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    public:
        MainGui(syn::VoiceManager* a_vm, int a_width, int a_height, int a_minWidth = 0, int a_minHeight = 0);
        ~MainGui();

        /// Create a temporary message box
        void alert(const std::string& a_title, const std::string& a_msg, DlgType a_type = DlgType::Information);

        operator nlohmann::json() const;
        MainGui* load(const nlohmann::json& j);

        /// Reset the GUI to its initial state
        void reset();

        CircuitWidget* circuitWidget() const { return m_circuitWidget; }

        Signal<int, int> onResize;
    protected:
        void createUnitSelector_(nanogui::Widget* a_widget);
        void createSettingsEditor_(nanogui::Widget* a_widget);
        void createThemeEditor_(nanogui::Widget* a_widget);
        void createOscilloscopeViewer_(nanogui::Widget* a_widget);

    private:
        void _onCreateWindow();
        void _runLoop() override;
        /// Initialize the GUI components
        void _rebuild();
        /// Recompute the GUI layout
        void _onResize();
        void _onOpen() override;
        void _onClose() override;

    private:
        int m_minWidth, m_minHeight;
        nanogui::ref<nanogui::Screen> m_screen;
        syn::VoiceManager* m_vm;

        // Forms
        std::shared_ptr<SerializableFormHelper> m_settingsFormHelper;
        std::shared_ptr<SerializableFormHelper> m_themeFormHelper;

        // Widgets
        EnhancedWindow* m_buttonPanel;

        EnhancedWindow* m_sidePanelL;
        nanogui::TabWidget* m_tabWidget;
        nanogui::Widget* m_unitSelector;
        UnitEditorHost* m_unitEditorHost;

        EnhancedWindow* m_sidePanelR;
        CircuitWidget* m_circuitWidget;
    };
}
