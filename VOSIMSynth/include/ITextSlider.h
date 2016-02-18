#pragma once
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

		void OnMouseWheel(int x, int y, IMouseMod* pMod, int d) override;

		ITextSlider(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_paramId, IRECT pR);

		virtual ~ITextSlider()
		{}

		void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;

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
