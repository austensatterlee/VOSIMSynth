#pragma once
#include "IGraphics.h"
#include "UnitParameter.h"
#include "VoiceManager.h"
#include "DSPMath.h"

namespace syn
{
	class ITextSlider : public IControl
	{
	public:
		ITextSlider(IPlugBase* pPlug, VoiceManager* vm, IRECT pR, int unitid, int paramid) :
			IControl(pPlug, pR),
			m_unitid(unitid),
			m_paramid(paramid),
			m_vm(vm),
			m_minsize(0)
		{
			UnitParameter& param = m_vm->getProtoInstrument()->getUnit(unitid).getParam(paramid);
			m_name = param.getName();
			mDisablePrompt = false;
			mValue = param.getNormalized();
		}

		virtual ~ITextSlider()
		{}

		void OnMouseDblClick(int x, int y, IMouseMod* pMod) override;

		void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override;

		bool Draw(IGraphics* pGraphics) override;


		void setRECT(const IRECT& a_newrect)
		{
			mRECT = a_newrect;
		}

		int getMinSize() const
		{
			return m_minsize;
		}

	protected:
		string m_name;
		int m_unitid;
		int m_paramid;
		VoiceManager* m_vm;
		int m_minsize;
	};
}
