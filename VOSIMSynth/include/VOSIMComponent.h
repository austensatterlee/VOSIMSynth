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
 *  \file VOSIMComponent.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 06/2016
 */

#ifndef __VOSIMCOMPONENT__
#define __VOSIMCOMPONENT__
#include "UIComponent.h"
#include "UICircuitPanel.h"
#include "UIUnitSelector.h"
#include "UIControlPanel.h"

namespace synui {
	class VOSIMComponent : public UIComponent
	{
	public:
		VOSIMComponent(MainWindow* a_window, syn::VoiceManager* a_vm, syn::UnitFactory* a_unitFactory);

		syn::VoiceManager* voiceManager() const { return m_vm; }
		syn::UnitFactory* unitFactory() const { return m_unitFactory; }
		UICircuitPanel* circuitPanel() const { return m_circuitPanel; }
		UIUnitSelector* unitSelector() const { return m_unitSelector; }
		UIControlPanel* controlPanel() const { return m_controlPanel; }

		void save(ByteChunk* a_data) const;
		int load(ByteChunk* a_data, int startPos) const;

		void notifyChildResized(UIComponent* a_child) override;
	protected:
		void draw(NVGcontext* a_nvg) override;
	private:
		void _onResize() override;
	private:
		UITabWidget* m_rightPanel;
		UICircuitPanel* m_circuitPanel;

		UITabWidget* m_leftPanel;
		UIControlPanel* m_controlPanel;
		UIUnitSelector* m_unitSelector;

		UIControlPanel* m_floatingPanel;

		syn::VoiceManager* m_vm;
		syn::UnitFactory* m_unitFactory;
	};
}

#endif
