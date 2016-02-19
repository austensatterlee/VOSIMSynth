#include "ITextSlider.h"

syn::ITextSlider::ITextSlider(IPlugBase* a_plug, shared_ptr<VoiceManager> a_vm, int a_unitId, int a_paramId, IRECT pR) :
	IControl(a_plug, pR),
	m_unitId(a_unitId),
	m_paramId(a_paramId),
	m_vm(a_vm),
	m_minSize(0)
{
	const UnitParameter& param = m_vm->getUnit(a_unitId).getParameter(a_paramId);
	m_name = param.getName();
	mValue = param.getNorm();
}

void syn::ITextSlider::TextFromTextEntry(const char* txt) {
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	try {
		size_t num_digits;
		double value = std::stod(txt, &num_digits);

		string str(txt);
		size_t decimal_pos;
		// adjust parameter precision
		decimal_pos = str.find_first_of(".");
		if(decimal_pos!=string::npos) {
			m_vm->doAction(ModifyParamPrecision, { m_unitId, m_paramId, static_cast<int>(num_digits) - static_cast<int>(decimal_pos) });
		}

		MuxArgs params = { m_unitId, m_paramId };
		params.value = value;
		m_vm->doAction(ModifyParam, params);
		mValue = param.getNorm();
	}catch(std::invalid_argument& e) {
		// do nothing
	}	
}

void syn::ITextSlider::OnMouseUp(int x, int y, IMouseMod* pMod) {
}

void syn::ITextSlider::OnMouseDown(int x, int y, IMouseMod* pMod) {
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	if (pMod->C) {
		IText entryTextFmt;
		string entryText = param.getString();
		mPlug->GetGUI()->CreateTextEntry(this, &entryTextFmt, &mRECT, entryText.c_str());
	}
}

void syn::ITextSlider::OnMouseWheel(int x, int y, IMouseMod* pMod, int d) {
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	mValue = param.getNorm();
	double scale;
	if (param.getType() != Double) {
		scale = 1.0 / (param.getMax() - param.getMin());
	}else {
		scale = pow(10, -param.getPrecision());
		if (pMod->C) {
			scale *= 10;
		}
		if (pMod->S) {
			scale *= 0.1;
		}
	}

	mValue += scale*d;

	MuxArgs params = { m_unitId, m_paramId };
	params.value = mValue;
	m_vm->doAction(ModifyParamNorm, params);
	mValue = param.getNorm();
}

void syn::ITextSlider::OnMouseDblClick(int x, int y, IMouseMod* pMod) {
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	double defaultValue = param.getDefaultValue();
	MuxArgs params = { m_unitId, m_paramId };
	params.value = defaultValue;
	m_vm->doAction(ModifyParam, params);
	mValue = param.getNorm();	
}

void syn::ITextSlider::OnMouseOver(int x, int y, IMouseMod* pMod) {}

void syn::ITextSlider::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod) {
	if (pMod->L && !pMod->C) {
		const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
		mValue = param.getNorm();
		double targetValue = (x - mRECT.L) * (1.0 / mRECT.W());
		double error = mValue - targetValue;
		double adjust_speed = 0.5;
		if (pMod->S)
			adjust_speed *= 0.1;
		mValue = mValue - adjust_speed*error;
		MuxArgs params = { m_unitId, m_paramId };
		params.value = mValue;
		m_vm->doAction(ModifyParamNorm, params);
		mValue = param.getNorm();
	}
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
	m_minSize = value_display_size.W() + name_display_size.W() + 10;
	return true;
}
