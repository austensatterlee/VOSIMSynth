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

syn::GainUnit::GainUnit(const string& a_name) :
	Unit(a_name),
	m_pGain(addParameter_(UnitParameter("gain", -1E4, 1E4, 1.0, UnitParameter::None, 2).setControlType(UnitParameter::Unbounded)))
{
	m_iInput = addInput_("in");
	m_iGain = addInput_("g[x]", 1.0);
	addOutput_("out");
}

syn::GainUnit::GainUnit(const GainUnit& a_rhs) :
	GainUnit(a_rhs.getName()) { }

void syn::GainUnit::process_() {
	double input = getInputValue(m_iInput);
	double gain = getParameter(m_pGain).getDouble();
	gain *= getInputValue(m_iGain);
	setOutputChannel_(0, input * gain);
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

syn::ConstantUnit::ConstantUnit(const string& a_name) :
	Unit(a_name) 
{
	addParameter_(UnitParameter{ "out",-1E6,1E6,0.0,UnitParameter::None,2 });
	getParameter("out").setControlType(UnitParameter::EControlType::Unbounded);
	addOutput_("out");
}

syn::ConstantUnit::ConstantUnit(const ConstantUnit& a_rhs) :
	ConstantUnit(a_rhs.getName()) { }

void syn::ConstantUnit::process_() {
	double output = getParameter(0).getDouble();
	setOutputChannel_(0, output);
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

syn::LerpUnit::LerpUnit(const string& a_name) :
	Unit(a_name) {
	m_pMinInput = addParameter_(UnitParameter("min in", -1E6, 1E6, 0.0).setControlType(UnitParameter::Unbounded));
	m_pMaxInput = addParameter_(UnitParameter("max in", -1E6, 1E6, 0.0).setControlType(UnitParameter::Unbounded));
	m_pMinOutput = addParameter_(UnitParameter("min out", -1E6, 1E6, 0.0).setControlType(UnitParameter::Unbounded));
	m_pMaxOutput = addParameter_(UnitParameter("max out", -1E6, 1E6, 1.0).setControlType(UnitParameter::Unbounded));
	addInput_("in");
	addOutput_("out");
}

syn::LerpUnit::LerpUnit(const LerpUnit& a_rhs) :
	LerpUnit(a_rhs.getName()) { }

void syn::LerpUnit::process_() {
	double input = getInputValue(0);
	double aIn = getParameter(m_pMinInput).getDouble();
	double bIn = getParameter(m_pMaxInput).getDouble();
	double aOut = getParameter(m_pMinOutput).getDouble();
	double bOut = getParameter(m_pMaxOutput).getDouble();
	double inputNorm = INVLERP(aIn, bIn, input);
	double output = LERP(aOut, bOut, inputNorm);	
	setOutputChannel_(0, output);
}