#include "ITextSlider.h"
#include <string>
#include <DSPMath.h>
#include <UI.h>

syn::ITextSlider::ITextSlider(IPlugBase* a_plug, VoiceManager* a_vm, int a_unitId, int a_paramId, IRECT pR) :
	IControl(a_plug, pR),
	m_unitId(a_unitId),
	m_paramId(a_paramId),
	m_vm(a_vm),
	m_minSize(0),
	m_name("") // gets updated in draw() method
{
	m_nameBitmap.resetConds(&mRECT,&m_name);
	m_valueBitmap.resetConds(&mRECT,&m_valuestr);
	m_bgBitmap.resetConds(&mRECT);
	m_fgBitmap.resetConds(&mRECT,&mValue,&m_valuestr);
}

syn::ITextSlider::ITextSlider(const ITextSlider& a_other) :
	ITextSlider(a_other.mPlug, a_other.m_vm, a_other.m_unitId, a_other.m_paramId, a_other.getRect())
{
}

syn::ITextSlider::~ITextSlider() {
}

void syn::ITextSlider::TextFromTextEntry(const char* txt) {
	try {
		size_t num_digits;
		double value = std::stod(txt, &num_digits);

		string str(txt);
		size_t decimal_pos;
		// adjust parameter precision
		decimal_pos = str.find_first_of(".");
		if (decimal_pos != string::npos) {
			int newParamPrecision = static_cast<int>(num_digits) - static_cast<int>(decimal_pos);
			newParamPrecision += 1;
			m_vm->queueAction(ModifyParamPrecision, {m_unitId, m_paramId, newParamPrecision});
		}

		ActionArgs params = {m_unitId, m_paramId};
		params.value = value;
		m_vm->queueAction(ModifyParam, params);
	} catch (std::invalid_argument&) {
		// do nothing
	}
}

void syn::ITextSlider::OnMouseUp(int x, int y, IMouseMod* pMod) {}

void syn::ITextSlider::OnMouseDown(int x, int y, IMouseMod* pMod) {
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	if (pMod->C) {
		IText entryTextFmt;
		string entryText = param.getString();
		mPlug->GetGUI()->CreateTextEntry(this, &entryTextFmt, &mRECT, entryText.c_str());
	}
}

void syn::ITextSlider::OnMouseWheel(int x, int y, IMouseMod* pMod, int d) {
	d = CLAMP(d, -1, 1);

	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);

	double scale;
	if (param.getType() != Double) {
		scale = 1.0;
	} else {
		scale = pow(10, -param.getPrecision() + 1);
		if (pMod->C) {
			scale *= 10;
		}
		if (pMod->S) {
			scale *= 0.1;
		}
	}

	double currValue = param.get<double>() + scale * d;

	ActionArgs params = {m_unitId, m_paramId};
	params.value = currValue;
	m_vm->queueAction(ModifyParam, params);
}

void syn::ITextSlider::OnMouseDblClick(int x, int y, IMouseMod* pMod) {
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);
	double defaultValue = param.getDefaultValue();
	ActionArgs params = {m_unitId, m_paramId};
	params.value = defaultValue;
	m_vm->queueAction(ModifyParam, params);
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
		mValue = mValue - adjust_speed * error;
		ActionArgs params = {m_unitId, m_paramId};
		params.value = mValue;
		m_vm->queueAction(ModifyParamNorm, params);
	}
}

void syn::ITextSlider::_drawName(LICE_IBitmap* a_dst, IGraphics* a_graphics) {
	ColorPoint textbgcolor(NDPoint<3>{0.,0.,0.});
	IText textfmtnear{13,&COLOR_WHITE,0,IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType};
	snprintf(m_strbuf, 256, "%s", m_name.c_str());
	// Measure text and resize bitmap
	IRECT blitrect{mRECT.L, mRECT.T, 0, 0};
	a_graphics->DrawIText(&textfmtnear, m_strbuf, &blitrect, true);
	blitrect.R += blitrect.L;
	a_dst->resize(blitrect.W(), blitrect.H());
	// Make background transparent and draw text on top
	LICE_SysBitmap tmp;
	tmp.resize(blitrect.W(), blitrect.H());
	LICE_Blit(&tmp, a_graphics->GetDrawBitmap(), 0, 0, blitrect.L, blitrect.T, blitrect.W(), blitrect.H(),1.0,LICE_BLIT_MODE_COPY);
	a_graphics->FillIRect(&textbgcolor.getIColor(), &blitrect);
	a_graphics->DrawIText(&textfmtnear, m_strbuf, &blitrect);
	// Copy text to cached bitmap
	LICE_Blit(a_dst, a_graphics->GetDrawBitmap(), 0, 0, blitrect.L, blitrect.T, blitrect.W(), blitrect.H(), 1.0, LICE_BLIT_MODE_COPY);
	LICE_SetAlphaFromColorMask(a_dst, 0x00000000);
	// Restore canvas
	LICE_Blit(a_graphics->GetDrawBitmap(), &tmp, blitrect.L, blitrect.T, 0, 0, blitrect.W(), blitrect.H(), 1.0, LICE_BLIT_MODE_COPY);
}

void syn::ITextSlider::_drawValue(LICE_IBitmap* a_dst, IGraphics* a_graphics) {
	ColorPoint textbgcolor(NDPoint<3>{ 0.,0.,0. });
	IText textfmtnear{13,&COLOR_WHITE,0,IText::kStyleNormal,IText::kAlignNear,0,IText::kQualityClearType};
	snprintf(m_strbuf, 256, "%s", m_valuestr.c_str());
	// Measure text and resize bitmap
	IRECT blitrect{mRECT.L, mRECT.T, 0, 0};
	a_graphics->DrawIText(&textfmtnear, m_strbuf, &blitrect, true);
	blitrect.R += blitrect.L;
	a_dst->resize(blitrect.W(), blitrect.H());
	// Make background transparent and draw text on top
	LICE_SysBitmap tmp;
	tmp.resize(blitrect.W(), blitrect.H());
	LICE_Blit(&tmp, a_graphics->GetDrawBitmap(), 0, 0, blitrect.L, blitrect.T, blitrect.W(), blitrect.H(),1.0,LICE_BLIT_MODE_COPY);
	a_graphics->FillIRect(&textbgcolor.getIColor(), &blitrect);
	a_graphics->DrawIText(&textfmtnear, m_strbuf, &blitrect);
	// Copy text to cached bitmap
	LICE_Blit(a_dst, a_graphics->GetDrawBitmap(), 0, 0, blitrect.L, blitrect.T, blitrect.W(), blitrect.H(), 1.0, LICE_BLIT_MODE_COPY);
	LICE_SetAlphaFromColorMask(a_dst, 0x00000000);
	// Restore canvas
	LICE_Blit(a_graphics->GetDrawBitmap(), &tmp, blitrect.L, blitrect.T, 0, 0, blitrect.W(), blitrect.H(), 1.0, LICE_BLIT_MODE_COPY);
}

void syn::ITextSlider::_drawBg(LICE_IBitmap* a_dst) const {
	a_dst->resize(mRECT.W(), mRECT.H());
	LICE_FillRect(a_dst, 0, 0, mRECT.W(), mRECT.H(), ColorPoint(0x0).getLicePixel(), 1.0, LICE_BLIT_MODE_COPY | LICE_BLIT_USE_ALPHA);
	LICE_DrawRect(a_dst, 0, 0, mRECT.W(), mRECT.H(), ColorPoint(NDPoint<3>{ 0.0, 0.0, 0.0 }).getLicePixel());
}

void syn::ITextSlider::_drawFg(LICE_IBitmap* a_dst) const {
	ColorPoint fg_color(NDPoint<3>{0.4,0.1,0.7});	

	int fillwidth = mRECT.W()*mValue;
	int fillheight = mRECT.H();
	a_dst->resize(mRECT.W(), mRECT.H());
	LICE_FillRect(a_dst, 0, 0, mRECT.W(), mRECT.H(), 0x00, 1.0, LICE_BLIT_MODE_COPY);
	LICE_FillRect(a_dst, 0, 0, fillwidth, fillheight,\
		fg_color.getLicePixel(), 1.0f,
		LICE_BLIT_MODE_COPY | LICE_BLIT_USE_ALPHA
		);

	// Draw glow
	ColorPoint glowcolor = LICE_AlterColorHSV(fg_color.getLicePixel(), 0.0f, 0.0f, 0.5f);
	double gradwidth = MAX(1.0,MIN(mRECT.W() - fillwidth, mRECT.W() / 6.));
	LICE_GradRect(a_dst, fillwidth, 0, int(gradwidth), mRECT.H(), \
		glowcolor.r(), glowcolor.g(), glowcolor.b(), 0.5, \
		0.0, 0.0, 0.0, -0.5/float(gradwidth),
		0.f, 0.f, 0.f, 0.f, \
		LICE_BLIT_MODE_COPY | LICE_BLIT_USE_ALPHA
		);
}

bool syn::ITextSlider::Draw(IGraphics* pGraphics) {
	LICE_SysBitmap* target = pGraphics->GetDrawBitmap();
	// Update mValue
	const UnitParameter& param = m_vm->getUnit(m_unitId).getParameter(m_paramId);

	// Update cached bitmaps if necessary
	m_name = param.getName();
	m_valuestr = param.getString();
	mValue = param.getNorm();

	m_nameBitmap.draw([&](LICE_IBitmap* dst) {_drawName(dst, pGraphics); });
	m_valueBitmap.draw([&](LICE_IBitmap* dst) {_drawValue(dst, pGraphics); });
	m_bgBitmap.draw([this](LICE_IBitmap* dst) {_drawBg(dst); });
	m_fgBitmap.draw([this](LICE_IBitmap* dst) {_drawFg(dst); });

	// Draw background
	m_bgBitmap.blit(target, NDPoint<2,int>{ mRECT.L, mRECT.T });
	// Draw foreground (filled portion)
	m_fgBitmap.blit(target, NDPoint<2,int>{ mRECT.L, mRECT.T });

	// Draw parameter name
	m_nameBitmap.blit(target, NDPoint<2,int>{mRECT.L,mRECT.T});

	// Draw parameter value
	m_valueBitmap.blit(target, NDPoint<2,int>{mRECT.R-m_valueBitmap.width(),mRECT.T});

	// Update minimum size if text doesn't fit
	m_minSize = m_nameBitmap.width() + m_valueBitmap.width() + 1;
	return true;
}

