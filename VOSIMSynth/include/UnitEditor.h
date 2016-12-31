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
 *  \file UnitEditor.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once
#include <nanogui/nanogui.h>
#include <nanogui/stackedwidget.h>

namespace syn
{
    class VoiceManager;
}

namespace synui
{
    class UnitEditor;

    typedef std::function<UnitEditor*(nanogui::Widget*, syn::VoiceManager*, int)> UnitEditorConstructor;

    class UnitEditor : public nanogui::Widget
    {
    public:
        UnitEditor(Widget* parent, syn::VoiceManager* a_vm, int a_unitId)
            : Widget(parent),
              m_vm(a_vm),
              m_unitId(a_unitId),
              m_isDirty(true)
        {
            if (m_unitId >= 0)
                _build();
        }

        int getUnitId() const { return m_unitId; }

        void setUnitId(int a_unitId)
        {
            if (m_unitId != a_unitId)
            {
                m_unitId = a_unitId;
                _build();
            }
        }

        void setParamValue(int a_paramId, double a_val) const;
        void setParamNorm(int a_paramId, double a_normval) const;
        void nudgeParam(int a_paramId, double a_logScale, double a_linScale) const;
        void setParamFromString(int a_paramId, const std::string& a_str) const;

        void draw(NVGcontext* ctx) override;

    protected:
        std::unordered_map<int, nanogui::Widget*> m_controls;
        std::unordered_map<int, nanogui::Label*> m_controlLabels;
        std::unordered_map<int, std::function<void()>> m_refreshFuncs;
        syn::VoiceManager* m_vm;
        int m_unitId;

        bool m_isDirty;
    private:
        void _build();
    };

    class UnitEditorHost : public nanogui::StackedWidget
    {
    public:
        UnitEditorHost(Widget* parent, syn::VoiceManager* a_vm);
        void activateEditor(unsigned a_classId, int a_unitId);
        int getActiveUnitId() const { return m_activeUnitId; }
        void addEditor(unsigned a_classId, int a_unitId);
        void removeEditor(int a_unitId);
        template<typename UnitType>
        void registerUnitEditor(UnitEditorConstructor a_func) { m_registeredUnitEditors[UnitType("").getClassIdentifier()] = a_func; }

        void reset()
        {
            setSelectedIndex(-1);
            m_editorMap.clear();
            m_activeUnitId = -1;
            while (childCount())
                removeChild(0);
        };

    protected:
        syn::VoiceManager* m_vm;
        
        std::unordered_map<int, UnitEditor*> m_editorMap; /// maps unit IDs to unit editors
        std::unordered_map<unsigned, UnitEditorConstructor> m_registeredUnitEditors;

        int m_activeUnitId;
    };
}
