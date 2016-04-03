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
#include "../lice/lice.h"
#include "UI.h"
#include <memory>

using std::shared_ptr;

namespace syn
{
	class ITextSlider : public IControl
	{
	public:

		ITextSlider(IPlugBase* a_plug, VoiceManager* a_vm, int a_unitId, int a_paramId, IRECT pR);

		ITextSlider(const ITextSlider& a_other);

		virtual ~ITextSlider();

		void TextFromTextEntry(const char* txt) override;

		void OnMouseUp(int x, int y, IMouseMod* pMod) override;

		void OnMouseDown(int x, int y, IMouseMod* pMod) override;

		void OnMouseWheel(int x, int y, IMouseMod* pMod, int d) override;

		void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;

		void OnMouseOver(int x, int y, IMouseMod* pMod) override;

		void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;

		bool Draw(IGraphics* pGraphics) override;

		void changeRect(const IRECT& a_newrect)
		{
			mRECT = a_newrect;
			mTargetRECT = a_newrect;
		}

		int getMinSize() const
		{
			return m_minSize;
		}

		IRECT getRect() const {
			return mRECT;
		}

	private:
		void _drawName(LICE_IBitmap* a_dst, IGraphics* a_graphics);
		void _drawValue(LICE_IBitmap* a_dst, IGraphics* a_graphics);
		void _drawBg(LICE_IBitmap* a_dst) const;
		void _drawFg(LICE_IBitmap* a_dst) const;

	private:
		string m_name, m_valuestr;
		int m_unitId;
		int m_paramId;
		VoiceManager* m_vm;
		int m_minSize;
		char m_strbuf[256];

		CachedImage m_nameBitmap;
		CachedImage m_valueBitmap;
		CachedImage m_bgBitmap;
		CachedImage m_fgBitmap;
	};
}
#endif