#include "MathUnits.h"
#include "DSPMath.h"

syn::MovingAverage::MovingAverage() :
	m_windowSize(1),
	m_lastOutput(0.0) {
	m_delay.resizeBuffer(m_windowSize);
}

void syn::MovingAverage::setWindowSize(int a_newWindowSize) {
	m_windowSize = a_newWindowSize;
	m_delay.resizeBuffer(m_windowSize);
	m_delay.clearBuffer();
	m_lastOutput = 0.0;
}

double syn::MovingAverage::getWindowSize() const {
	return m_windowSize;
}

double syn::MovingAverage::process(double a_input) {
	double output = (1.0 / m_windowSize) * (a_input - m_delay.process(a_input)) + m_lastOutput;
	m_lastOutput = output;
	return output;
}

double syn::MovingAverage::getPastInputSample(int a_offset) {
	return m_delay.getPastSample(a_offset);
}

syn::BLIntegrator::BLIntegrator() : m_state(0), m_normfc(0) {}

void syn::BLIntegrator::setFc(double a_newfc) {
	m_normfc = a_newfc / 2;
}

double syn::BLIntegrator::process(double a_input) {
	a_input = m_normfc * a_input;
	m_state = 2 * a_input + m_state;
	return m_state;
}

syn::DCRemoverUnit::DCRemoverUnit(const string& a_name) :
	Unit(a_name),
	m_pAlpha(addParameter_(UnitParameter("hp", 0.0, 1.0, 0.995))),
	m_lastInput(0.0),
	m_lastOutput(0.0) {
	addInput_("in");
	addOutput_("out");
}

syn::DCRemoverUnit::DCRemoverUnit(const DCRemoverUnit& a_rhs) :
	DCRemoverUnit(a_rhs.getName()) {}

void syn::DCRemoverUnit::process_() {
	double input = getInputValue(0);
	double alpha = getParameter(m_pAlpha).getDouble();
	double gain = 0.5 * (1 + alpha);
	// dc removal
	input = input * gain;
	double output = input - m_lastInput + alpha * m_lastOutput;
	m_lastInput = input;
	m_lastOutput = output;
	setOutputChannel_(0, output);
}

string syn::DCRemoverUnit::_getClassName() const {
	return "DCRemoverUnit";
}

syn::Unit* syn::DCRemoverUnit::_clone() const {
	return new DCRemoverUnit(*this);
}

syn::LagUnit::LagUnit(const string& a_name) :
	Unit(a_name),
	m_pFc(addParameter_(UnitParameter("fc", 0.01, 20000.0, 1.0, UnitParameter::Freq))),
	m_state(0.0) {
	addInput_("in");
	m_iFcAdd = addInput_("fc");
	m_iFcMul = addInput_("fc[x]", 1.0);
	addOutput_("out");
}

syn::LagUnit::LagUnit(const LagUnit& a_rhs) : LagUnit(a_rhs.getName()) {}

void syn::LagUnit::process_() {
	double input = getInputValue(0);
	double fc = getParameter(m_pFc).getDouble() * getInputValue(m_iFcMul) + getInputValue(m_iFcAdd); // freq cutoff
	fc = CLAMP(fc, getParameter(m_pFc).getMin(), getParameter(m_pFc).getMax());
	fc = fc / getFs();
	double wc = 2 * tan(DSP_PI * fc / 2.0);
	double gain = wc / (1 + wc);
	double trap_in = gain * (input - m_state);
	double output = trap_in + m_state;
	m_state = trap_in + output;
	setOutputChannel_(0, output);
}

string syn::LagUnit::_getClassName() const {
	return "LagUnit";
}

syn::Unit* syn::LagUnit::_clone() const {
	return new LagUnit(*this);
}

syn::RectifierUnit::RectifierUnit(const string& a_name) :
	Unit(a_name),
	m_pRectType(addParameter_(UnitParameter{ "type",{"full","half"} }))
{
	addInput_("in");
	addOutput_("out");
}

syn::RectifierUnit::RectifierUnit(const RectifierUnit& a_rhs) :
	RectifierUnit(a_rhs.getName()) { }

void syn::RectifierUnit::process_() {
	double input = getInputValue(0);
	double output;
	switch (getParameter(m_pRectType).getInt()) {
	case 0: // full
		output = abs(input);
		break;
	case 1: // half
		output = input > 0 ? input : 0;
		break;
	}
	setOutputChannel_(0, output);
}

string syn::RectifierUnit::_getClassName() const {
	return "RectifierUnit";
}

syn::Unit* syn::RectifierUnit::_clone() const {
	return new RectifierUnit(*this);
}

syn::MACUnit::MACUnit(const string& a_name) :
	Unit(a_name) {
	addInput_("in");
	addInput_("a[x]", 1.0);
	addInput_("b[+]");
	addOutput_("a*in+b");
}

syn::MACUnit::MACUnit(const MACUnit& a_rhs) :
	MACUnit(a_rhs.getName()) { }

void syn::MACUnit::process_() {
	double output = getInputValue(0);
	output *= getInputValue(1);
	output += getInputValue(2);
	setOutputChannel_(0, output);
}

string syn::MACUnit::_getClassName() const {
	return "MACUnit";
}

syn::Unit* syn::MACUnit::_clone() const {
	return new MACUnit(*this);
}

syn::GainUnit::GainUnit(const string& a_name) :
	Unit(a_name),
	m_pGain(addParameter_(UnitParameter("gain", 0.0, 1.0, 1.0))),
	m_pScale(addParameter_(UnitParameter("scale", scale_selections, scale_values))) {
	m_iInput = addInput_("in[+]");
	m_iInvInput = addInput_("in[-]");
	m_iGain = addInput_("gain[x]", 1.0);
	addOutput_("out");
}

syn::GainUnit::GainUnit(const GainUnit& a_rhs) :
	GainUnit(a_rhs.getName()) { }

void syn::GainUnit::process_() {
	double input = getInputValue(m_iInput) - getInputValue(m_iInvInput);
	double gain = getParameter(m_pGain).getDouble() * getParameter(m_pScale).getEnum();
	gain *= getInputValue(m_iGain);
	setOutputChannel_(0, input * gain);
}

string syn::GainUnit::_getClassName() const {
	return "GainUnit";
}

syn::Unit* syn::GainUnit::_clone() const {
	return new GainUnit(*this);
}

syn::SummerUnit::SummerUnit(const string& a_name) :
	Unit(a_name)
{
	addInput_("1");
	addInput_("2");
	addOutput_("out");
}

syn::SummerUnit::SummerUnit(const SummerUnit& a_rhs) :
	SummerUnit(a_rhs.getName()) { }

void syn::SummerUnit::process_() {
	double output = getInputValue(0) + getInputValue(1);
	setOutputChannel_(0, output);
}

string syn::SummerUnit::_getClassName() const {
	return "SummerUnit";
}

syn::Unit* syn::SummerUnit::_clone() const {
	return new SummerUnit(*this);
}

syn::ConstantUnit::ConstantUnit(const string& a_name) :
	Unit(a_name) 
{
	addParameter_(UnitParameter{ "out",-1.0,1.0,0.0 });
	addParameter_(UnitParameter{ "scale",scale_selections,scale_values });
	getParameter("scale").setControlType(UnitParameter::EControlType::Unbounded);
	addOutput_("out");
}

syn::ConstantUnit::ConstantUnit(const ConstantUnit& a_rhs) :
	ConstantUnit(a_rhs.getName()) { }

void syn::ConstantUnit::process_() {
	double output = getParameter(0).getDouble();
	double scale = getParameter(1).getEnum();
	output = output * scale;
	setOutputChannel_(0, output);
}

string syn::ConstantUnit::_getClassName() const {
	return "ConstantUnit";
}

syn::Unit* syn::ConstantUnit::_clone() const {
	return new ConstantUnit(*this);
}

syn::PanningUnit::PanningUnit(const string& a_name) :
	Unit(a_name) {
	addInput_("in1");
	addInput_("in2");
	addInput_("bal1");
	addInput_("bal2");
	addOutput_("out1");
	addOutput_("out2");
	m_pBalance1 = addParameter_({ "bal1",-1.0,1.0,0.0 });
	m_pBalance2 = addParameter_({ "bal2",-1.0,1.0,0.0 });
}

syn::PanningUnit::PanningUnit(const PanningUnit& a_rhs) :
	PanningUnit(a_rhs.getName()) { }

void syn::PanningUnit::process_() {
	double in1 = getInputValue(0);
	double in2 = getInputValue(1);
	double bal1 = getParameter(m_pBalance1).getDouble() + getInputValue(2);
	double bal2 = getParameter(m_pBalance2).getDouble() + getInputValue(3);
	bal1 = 0.5*(1+CLAMP(bal1, -1.0, 1.0));
	bal2 = 0.5*(1+CLAMP(bal2, -1.0, 1.0));
	setOutputChannel_(0, (1 - bal1) * in1 + (1 - bal2) * in2);
	setOutputChannel_(1, bal1 * in1 + bal2 * in2);
}

string syn::PanningUnit::_getClassName() const {
	return "PanningUnit";
}

syn::Unit* syn::PanningUnit::_clone() const {
	return new PanningUnit(*this);
}

syn::LerpUnit::LerpUnit(const string& a_name) :
	Unit(a_name) {
	m_pInputRange = addParameter_(UnitParameter("input", { "bipolar","unipolar" }));
	m_pMinOutput = addParameter_(UnitParameter("min out", -1.0, 1.0, 0.0));
	m_pMinOutputScale = addParameter_(UnitParameter("min scale", scale_selections, scale_values));
	m_pMaxOutput = addParameter_(UnitParameter("max out", -1.0, 1.0, 1.0));
	m_pMaxOutputScale = addParameter_(UnitParameter("max scale", scale_selections, scale_values));
	addInput_("in");
	addOutput_("out");
}

syn::LerpUnit::LerpUnit(const LerpUnit& a_rhs) :
	LerpUnit(a_rhs.getName()) { }

void syn::LerpUnit::process_() {
	double input = getInputValue(0);
	double a_scale = getParameter(m_pMinOutputScale).getEnum();
	double a = getParameter(m_pMinOutput).getDouble() * a_scale;
	double b_scale = getParameter(m_pMaxOutputScale).getEnum();
	double b = getParameter(m_pMaxOutput).getDouble() * b_scale;
	double output;
	if (getParameter(m_pInputRange).getInt() == 1) {
		output = LERP(a, b, input);
	}
	else {
		output = LERP(a, b, 0.5*(input + 1));
	}
	setOutputChannel_(0, output);
}

string syn::LerpUnit::_getClassName() const {
	return "LerpUnit";
}

syn::Unit* syn::LerpUnit::_clone() const {
	return new LerpUnit(*this);
}