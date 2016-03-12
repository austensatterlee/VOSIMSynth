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

#ifndef __ITEXTSLIDER__
#define __ITEXTSLIDER__
#include "IGraphics.h"
#include "UnitParameter.h"
#include "VoiceManager.h"

namespace syn
{
	class ITextSlider : public IControl
	{
	public:
		void TextFromTextEntry(const char* txt) override;

		void OnMouseUp(int x, int y, IMouseMod* pMod) override;

		void OnMouseDown(int x, int y, IMouseMod* pMod) override;

		void OnMouseWheel(int x, int y, IMouseMod* pMod, int d) override;

		ITextSlider(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_paramId, IRECT pR);

		virtual ~ITextSlider()
		{}

		void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;

		void OnMouseOver(int x, int y, IMouseMod* pMod) override;

		void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;

		bool Draw(IGraphics* pGraphics) override;


		void setRECT(const IRECT& a_newrect)
		{
			mRECT = a_newrect;
			mTargetRECT = a_newrect;
		}

		int getMinSize() const
		{
			return m_minSize;
		}

	private:
		string m_name;
		int m_unitId;
		int m_paramId;
		shared_ptr<VoiceManager> m_vm;
		int m_minSize;
	};
}
#endif