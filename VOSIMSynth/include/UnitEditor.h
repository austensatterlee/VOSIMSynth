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
#include "Unit.h"
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
                UnitEditor::_build();
        }

        virtual ~UnitEditor() {}

        int getUnitId() const { return m_unitId; }

        void setUnitId(int a_unitId)
        {
            if (m_unitId != a_unitId)
            {
                m_unitId = a_unitId;
                _build();
            }
        }

        void setParamValue(int a_paramId, double a_val);
        void setParamNorm(int a_paramId, double a_normval);
        void nudgeParam(int a_paramId, double a_logScale, double a_linScale);
        void setParamFromString(int a_paramId, const string& a_str);

        void draw(NVGcontext* ctx) override;

    protected:
        std::unordered_map<int, Widget*> m_controls;
        std::unordered_map<int, nanogui::Label*> m_controlLabels;
        std::unordered_map<int, std::function<void()> > m_refreshFuncs;
        syn::VoiceManager* m_vm;
        int m_unitId;

        bool m_isDirty;
    private:
        virtual void _build();
    };

    class UnitEditorHost : public nanogui::StackedWidget
    {
    public:
        UnitEditorHost(Widget* parent, syn::VoiceManager* a_vm);
        void activateEditor(syn::UnitTypeId a_classId, int a_unitId);
        int getActiveUnitId() const { return m_activeUnitId; }
        void addEditor(syn::UnitTypeId a_classId, int a_unitId);
        void removeEditor(int a_unitId);
        template<typename UnitType>
        void registerUnitEditor(UnitEditorConstructor a_func) { m_registeredUnitEditors[UnitType("").getClassIdentifier()] = a_func; }

        void reset()
        {
            setSelectedIndex(-1);
            m_activeUnitId = -1;
            m_editorMap.clear();
            while (childCount())
                removeChild(0);
        };

    protected:
        syn::VoiceManager* m_vm;
        
        std::unordered_map<int, UnitEditor*> m_editorMap; /// maps unit IDs to unit editor instances.
        std::unordered_map<syn::UnitTypeId, UnitEditorConstructor> m_registeredUnitEditors; /// maps types of units (class IDs) to unit editor constructors.

        int m_activeUnitId; /// ID of the unit whose editor is currently visible.
    };
}
