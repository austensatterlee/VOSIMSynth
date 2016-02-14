#include "ITextSlider.h"

syn::ITextSlider::ITextSlider(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_paramId, IRECT pR):
	IControl(a_plug, pR),
	m_unitId(a_unitId),
	m_paramId(a_paramId),
	m_vm(a_vm),
	m_minSize(0) 
{
	const UnitParameter& param = m_vm->getUnit(a_unitId).getParameter(a_paramId);
	m_name = param.getName();
	mDisablePrompt = false;
	mValue = param.getNorm();
}

void syn::ITextSlider::OnMouseDblClick(int x, int y, IMouseMod* pMod) {
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	double defaultValue = param.getDefaultValue();
    MuxArgs params = {m_unitId, m_paramId};
    params.value = defaultValue;
	m_vm->doAction(ModifyParam, params);
    mValue = param.getNorm();
}

void syn::ITextSlider::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) {
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	mValue = param.getNorm();
	// do fine adjustments when shift is held
	double scale = pMod->S ? 0.1 : 1;
	// coarse adjustments when ctrl is held
	if(pMod->C)
	{
		scale = 10.0; 
	}
	scale /= mRECT.W();
			
	double modAmt = dX * scale / 4;
	mValue += modAmt;
    MuxArgs params = {m_unitId, m_paramId};
    params.value = mValue;
    m_vm->doAction(ModifyParamNorm, params);
    mValue = param.getNorm();
}

bool syn::ITextSlider::Draw(IGraphics* pGraphics) {
	// Local color palette
	IColor bg_color{ 255,100,50,125 };
	IColor fg_color{ 255,200,150,225 };
	// Local text palette
	IText textfmtfar{ 12,&COLOR_WHITE,"Helvetica",IText::kStyleNormal,IText::kAlignFar,0,IText::kQualityClearType };
	IText textfmtnear{ 12,&COLOR_WHITE,"Helvetica",IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType };
	// Draw background
	pGraphics->FillIRect(&bg_color, &mRECT);
	pGraphics->DrawRect(&COLOR_BLACK, &mRECT);

    const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	// Draw foreground (filled portion)
	IRECT filled_irect{ mRECT.L+1,mRECT.T+1,mRECT.L + int(mRECT.W()*param.getNorm()),mRECT.B-1 };
	pGraphics->FillIRect(&fg_color, &filled_irect);

	char strbuf[256];
	// Draw parameter name
	string namestr = param.getName();
	snprintf(strbuf, 256, "%s", namestr.c_str());
	IRECT name_display_size{ 0, 0, 0, 0 };
	pGraphics->DrawIText(&textfmtnear, strbuf, &name_display_size, true);
	pGraphics->DrawIText(&textfmtnear, strbuf, &mRECT);

	// Draw parameter value
	string displayText = param.getString();
    snprintf(strbuf, 256, "%s", displayText.c_str());
			
	IRECT value_display_size{ 0,0,0,0 };
	pGraphics->DrawIText(&textfmtnear, strbuf, &value_display_size, true);
	pGraphics->DrawIText(&textfmtfar, strbuf, &mRECT);

	// Update minimum size if text doesn't fit
	m_minSize = MAX(m_minSize, value_display_size.W() + name_display_size.W() + 10);
	return true;
}
