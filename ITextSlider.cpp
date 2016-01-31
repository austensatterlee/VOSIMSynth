#include "ITextSlider.h"

void syn::ITextSlider::OnMouseDblClick(int x, int y, IMouseMod* pMod) {
	UnitParameter& param = m_vm->getProtoInstrument()->getUnit(m_unitid).getParam(m_paramid);
	double defaultValue = param.getDefault();
	m_vm->modifyParameter(m_unitid, m_paramid, defaultValue, SET);
	mValue = param.getNormalized();
}

void syn::ITextSlider::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) {
	UnitParameter& param = m_vm->getProtoInstrument()->getUnit(m_unitid).getParam(m_paramid);
	// do fine adjustments when shift is held
	double scale = pMod->S ? 0.1 : 1;
	// coarse adjustments when ctrl is held
	if(pMod->C)
	{
		scale = 10.0; 
	}
	scale /= mRECT.W();
			
	double modamt = dX * scale / 4;
	mValue += modamt;
	m_vm->modifyParameter(m_unitid, m_paramid, mValue, SET_NORM);
	mValue = param.getNormalized();
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

	UnitParameter& param = m_vm->getProtoInstrument()->getUnit(m_unitid).getParam(m_paramid);
	// Draw foreground (filled portion)
	IRECT filled_irect{ mRECT.L,mRECT.T,mRECT.L + int(mRECT.W()*param.getNormalized()),mRECT.B };
	pGraphics->FillIRect(&fg_color, &filled_irect);

	char strbuf[256];
	// Draw parameter name
	string namestr = param.getName();
	sprintf(strbuf, "%s", namestr.c_str());
	IRECT name_display_size{ 0, 0, 0, 0 };
	pGraphics->DrawIText(&textfmtnear, strbuf, &name_display_size, true);
	pGraphics->DrawIText(&textfmtnear, strbuf, &mRECT);

	// Draw parameter value
	param.getDisplayText(strbuf);
			
	IRECT value_display_size{ 0,0,0,0 };
	pGraphics->DrawIText(&textfmtnear, strbuf, &value_display_size, true);
	pGraphics->DrawIText(&textfmtfar, strbuf, &mRECT);

	// Update minimum size if text doesn't fit
	m_minsize = MAX(m_minsize, value_display_size.W() + name_display_size.W() + 10);
	return true;
}
