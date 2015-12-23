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
			m_min = param.getMin();
			m_max = param.getMax();
			m_value = (param - m_min) / (m_max - m_min);
			m_name = param.getName();
			mDisablePrompt = false;
		}

		virtual ~ITextSlider()
		{}

		virtual void OnMouseDblClick(int x, int y, IMouseMod* pMod) override
		{
			double defaultValue = m_vm->getProtoInstrument()->getParameter(m_unitid, m_paramid).getDefault();
			m_value = (defaultValue - m_min) / (m_max - m_min);
			m_vm->modifyParameter(m_unitid, m_paramid, defaultValue, SET);
		}

		void setRECT(const IRECT& a_newrect)
		{
			mRECT = a_newrect;
		}

		int getMinSize() const
		{
			return m_minsize;
		}

		virtual void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) override
		{
			// do fine adjustments when shift is held
			double scale = pMod->S ? 0.01 : 0.1;
			// do integer adjustments when ctrl is held
			if(pMod->C)
			{
				scale = 1.0; 
			}
			
			m_value += dX*scale / double(m_max - m_min);

			if (m_value > 1) m_value = 1;
			else if (m_value < 0) m_value = 0;

			m_vm->modifyParameter(m_unitid, m_paramid, m_value*(m_max - m_min) + m_min, SET);
		}

		virtual bool Draw(IGraphics* pGraphics) override
		{
			// Local color palette
			IColor bg_color{ 255,100,50,125 };
			IColor fg_color{ 255,200,150,225 };
			// Local text palette
			IText textfmtfar{ 12,&COLOR_WHITE,"Helvetica",IText::kStyleNormal,IText::kAlignFar,0,IText::kQualityClearType };
			IText textfmtnear{ 12,&COLOR_WHITE,"Helvetica",IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType };
			// Draw background
			pGraphics->FillIRect(&bg_color, &mRECT);
			// Draw foreground (filled portion)
			IRECT filled_irect{ mRECT.L,mRECT.T,mRECT.L + int(mRECT.W()*m_value),mRECT.B };
			pGraphics->FillIRect(&fg_color, &filled_irect);

			UnitParameter& param = m_vm->getProtoInstrument()->getUnit(m_unitid).getParam(m_paramid);
			char strbuf[256];
			// Draw parameter name
			string namestr = param.getName();
			sprintf(strbuf, "%s", namestr.c_str());
			IRECT name_display_size{ 0, 0, 0, 0 };
			pGraphics->DrawIText(&textfmtnear, strbuf, &name_display_size, true);
			pGraphics->DrawIText(&textfmtnear, strbuf, &mRECT);

			// Draw parameter value
			string paramstr = param.getString();
			sprintf(strbuf, "%s", paramstr.c_str());
			IRECT value_display_size{ 0,0,0,0 };
			pGraphics->DrawIText(&textfmtnear, strbuf, &value_display_size, true);
			pGraphics->DrawIText(&textfmtfar, strbuf, &mRECT);

			// Update minimum size if text doesn't fit
			m_minsize = max(m_minsize, value_display_size.W() + name_display_size.W() + 10);
			return true;
		}

	protected:
		string m_name;
		int m_unitid;
		int m_paramid;
		double m_value;
		double m_min, m_max;
		VoiceManager* m_vm;
		int m_minsize;
	};
}
