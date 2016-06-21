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

#include "Oscillator.h"
#include "DSPMath.h"
#include "tables.h"

namespace syn
{
	double sampleWaveShape(WAVE_SHAPE shape, double phase, double period, bool useNaive) {
		double output;
		switch (static_cast<int>(shape)) {
		case SAW_WAVE:
			if (useNaive) {
				if (phase < 0.5)
					output = 2 * phase;
				else
					output = 2 * phase - 2;
			}
			else {
				output = lut_bl_saw.getresampled(phase, period);
			}
			break;
		case SINE_WAVE:
			output = lut_sin.getlinear(phase);
			break;
		case TRI_WAVE:
			phase = WRAP(phase, 1.0);
			output = phase <= 0.5 ? 4 * phase - 1 : -4 * (phase - 0.5) + 1;
			break;
		case SQUARE_WAVE:
			if (useNaive)
				output = phase <= 0.5 ? 1 : -1;
			else
				output = -lut_bl_saw.getresampled(phase + 0.5, period) + lut_bl_saw.getresampled(phase, period);
			break;
		default:
			output = 0;
			break;
		}
		return output;
	}

	Oscillator::Oscillator(const string& a_name) :
		Unit(a_name),
		m_basePhase(0),
		m_phase(0),
		m_last_phase(0),
		m_phase_step(0),
		m_period(1),
		m_freq(0.0),
		m_gain(0.0),
		m_bias(0.0),
		m_oOut(addOutput_("out")),
		m_oPhase(addOutput_("ph")),
		m_pGain(addParameter_({ "gain", 0.0, 1.0, 1.0 })),
		m_pPhaseOffset(addParameter_({ "phase", 0.0, 1.0, 0.0 })),
		m_pUnipolar(addParameter_(UnitParameter("unipolar", false))),
		m_iGainMul(addInput_("g[x]", 1.0)),
		m_iPhaseAdd(addInput_("ph")) { }

	void Oscillator::sync_() {}

	/******************************
	* Oscillator methods
	*
	******************************/

	void Oscillator::updatePhaseStep_() {
		if (m_freq) {
			m_period = getFs() / m_freq;
			m_phase_step = 1. / m_period;
		}
		else {
			m_period = 0.0;
			m_phase_step = 0.0;
		}
	}

	void Oscillator::tickPhase_(double a_phaseOffset) {
		m_basePhase += m_phase_step;
		if (m_basePhase >= 1) {
			m_basePhase -= 1;
		}
		m_phase = m_basePhase + a_phaseOffset;
		m_phase = WRAP(m_phase, 1.0);
		if (1 - (m_last_phase - m_phase) < 0.5) {
			sync_();
		}
		m_last_phase = m_phase;
	}

	void Oscillator::process_() {
		double phase_offset = getParameter(m_pPhaseOffset).getDouble() + getInputValue(m_iPhaseAdd);
		m_gain = getParameter(m_pGain).getDouble() * getInputValue(m_iGainMul);
		m_bias = 0;
		if (getParameter(m_pUnipolar).getBool()) {
			// make signal unipolar
			m_gain *= 0.5;
			m_bias = m_gain;
		}
		updatePhaseStep_();
		tickPhase_(phase_offset);

		setOutputChannel_(m_oPhase, m_phase);
	}

	void TunedOscillator::onNoteOn_() {
		m_basePhase = 0.0;
		sync_();
	}

	void TunedOscillator::updatePhaseStep_() {
		m_freq = pitchToFreq(m_pitch);
		Oscillator::updatePhaseStep_();
	}

	TunedOscillator::TunedOscillator(const string& a_name) :
		Oscillator(a_name),
		m_pitch(0),
		m_pTune(addParameter_({ "semi", -12.0, 12.0, 0.0 })),
		m_pOctave(addParameter_({ "oct", -3, 3, 0 })) {
		m_iNote = addInput_("pitch");
	}

	TunedOscillator::TunedOscillator(const TunedOscillator& a_rhs) :
		TunedOscillator(a_rhs.getName()) {}

	void TunedOscillator::process_() {
		double tune = getParameter(m_pTune).getDouble() + 128 * getInputValue(m_iNote);
		double oct = getParameter(m_pOctave).getInt();
		m_pitch = tune + oct * 12;
		Oscillator::process_();
	}

	BasicOscillator::BasicOscillator(const string& a_name) :
		TunedOscillator(a_name),
		m_pWaveform(addParameter_(UnitParameter("waveform", WAVE_SHAPE_NAMES))) { }

	BasicOscillator::BasicOscillator(const BasicOscillator& a_rhs) : BasicOscillator(a_rhs.getName()) {}

	void BasicOscillator::process_() {
		TunedOscillator::process_();
		double output;
		int shape = getParameter(m_pWaveform).getInt();
		output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period, false);
		setOutputChannel_(m_oOut, m_gain * output + m_bias);
	}

	string BasicOscillator::_getClassName() const {
		return "BasicOscillator";
	}

	Unit* BasicOscillator::_clone() const {
		return new BasicOscillator(*this);
	}

	LFOOscillator::LFOOscillator(const string& a_name) :
		Oscillator(a_name),
		m_oQuadOut(addOutput_("quad")),
		m_iFreqAdd(addInput_("freq")),
		m_iFreqMul(addInput_("freq[x]", 1.0)),
		m_iSync(addInput_("sync")),
		m_lastSync(0.0)
	{
		m_pBPMFreq = addParameter_({ "rate", g_bpmStrs, g_bpmVals, 0, UnitParameter::EUnitsType::BPM });
		getParameter(m_pBPMFreq).setVisible(false);
		m_pFreq = addParameter_({ "freq", 0.01, 30.0, 1.0, UnitParameter::EUnitsType::Freq });
		m_pWaveform = addParameter_(UnitParameter("waveform", WAVE_SHAPE_NAMES));
		m_pTempoSync = addParameter_(UnitParameter("tempo sync", false));
	}

	LFOOscillator::LFOOscillator(const LFOOscillator& a_rhs) :
		LFOOscillator(a_rhs.getName())
	{
	}

	string LFOOscillator::_getClassName() const {
		return "LFOOscillator";
	}

	Unit* LFOOscillator::_clone() const {
		return new LFOOscillator(*this);
	}

	void LFOOscillator::process_() {
		// determine frequency
		if (getParameter(m_pTempoSync).getBool()) {
			m_freq = bpmToFreq(getInputValue(m_iFreqMul) * (getParameter(m_pBPMFreq).getEnum() + getInputValue(m_iFreqAdd)), getTempo());
		}
		else {
			m_freq = getInputValue(m_iFreqMul) * (getParameter(m_pFreq).getDouble() + getInputValue(m_iFreqAdd));
		}
		// sync
		if (m_lastSync <= 0.0 && getInputValue(m_iSync) > 0.0) {
			m_basePhase = 0.0;
			sync_();
		}
		m_lastSync = getInputValue(m_iSync);
		Oscillator::process_();
		double output;
		int shape = getParameter(m_pWaveform).getInt();
		output = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase, m_period, true);
		setOutputChannel_(m_oOut, m_gain * output + m_bias);
		///  Output quad output
		/// \todo <experimentation>
		double quadoutput;
		quadoutput = sampleWaveShape(static_cast<WAVE_SHAPE>(shape), m_phase + 0.25, m_period, true);
		setOutputChannel_(m_oQuadOut, m_gain * quadoutput + m_bias);
	}

	void LFOOscillator::onParamChange_(int a_paramId) {
		if (a_paramId == m_pTempoSync) {
			bool useTempoSync = getParameter(m_pTempoSync).getBool();
			getParameter(m_pFreq).setVisible(!useTempoSync);
			getParameter(m_pBPMFreq).setVisible(useTempoSync);
		}
	}
}