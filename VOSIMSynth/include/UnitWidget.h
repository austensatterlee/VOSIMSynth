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
 *  \file UnitWidget.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 12/2016
 */

#pragma once
#include <nanogui/nanogui.h>

namespace syn
{
    class VoiceManager;
}

namespace synui
{  

	class CircuitWidget;
	class UnitWidget : public nanogui::Widget
	{
	public:
		UnitWidget(CircuitWidget *a_parent, syn::VoiceManager *a_vm, int a_unitId);

		void draw(NVGcontext *ctx) override;
		bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;
		bool mouseDragEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers) override;

		/**
		 * \brief Compute the coordinates of the input port with the given id.
		 * \param a_portId The id of the requested input port, corresponding with the id of an input port on the associated syn::Unit.
		 * \return Coordinates of the requested input port.
		 */
		Eigen::Vector2i getInputPortPosition(int a_portId);

		/**
		 * \brief Compute the coordinates of the output port with the given id.
		 * \param a_portId The id of the requested output port, corresponding with the id of an output port on the associated syn::Unit.
		 * \return Coordinates of the requested output port.
		 */
		Eigen::Vector2i getOutputPortPosition(int a_portId);

	    /**
         * \brief Set the function that will be called when the unit's editor interface has been requested by the user.
         * The callback receives the unit's editor interface as an argument. It may then attach the interface somewhere that will be displayed on-screen.
         */
        void setCallback(std::function<void(Widget*)> callback){ m_callback = callback; }

	    Eigen::Vector2i preferredSize(NVGcontext *ctx) const override;

	    void setParamValue(int a_paramId, double a_val) const;
	    void setParamNorm(int a_paramId, double a_normval) const;
	    void nudgeParam(int a_paramId, double a_logScale, double a_linScale) const;
	    void setParamFromString(int a_paramId, const std::string &a_str) const;

	protected:
        virtual nanogui::Widget *createEditor_(nanogui::Widget *a_parent);  

	protected:	
        std::function<void(Widget*)> m_callback; // editor callback
        Widget *m_editorWidget;
		CircuitWidget *m_parentCircuit;
	    syn::VoiceManager *m_vm;
		Widget *m_titleLabel;
		std::map<int, Widget*> m_inputLabels;
		std::map<int, Widget*> m_outputLabels;
        std::map<int, Widget*> m_emptyInputLabels;
        std::map<int, Widget*> m_emptyOutputLabels;

		Eigen::Vector2i m_oldPos;
		enum
		{
		    Uninitialized,
            Idle,
            Dragging
		} m_state;

		int m_unitId;
	};

    class InputUnitWidget : public UnitWidget
    {
    public:
        InputUnitWidget(CircuitWidget *a_parent, syn::VoiceManager *a_vm, int a_unitId)
            : UnitWidget(a_parent, a_vm, a_unitId)
        {
            for(auto lbl : m_inputLabels)
            {
                auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());
                l->removeAnchor(lbl.second);
                removeChild(lbl.second);
            }
            m_inputLabels.clear();
        }
    };
    
    class OutputUnitWidget : public UnitWidget
    {
    public:
        OutputUnitWidget(CircuitWidget *a_parent, syn::VoiceManager *a_vm, int a_unitId)
            : UnitWidget(a_parent, a_vm, a_unitId)
        {
            for(auto lbl : m_outputLabels)
            {
                auto l = static_cast<nanogui::AdvancedGridLayout*>(layout());
                l->removeAnchor(lbl.second);
                removeChild(lbl.second);
            }
            m_outputLabels.clear();
        }
    };
}
