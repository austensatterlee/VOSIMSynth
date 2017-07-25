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
#include "vosimsynth/VOSIMSynth.h"
#include <IPlug/IPlug_include_in_plug_src.h>
#include <vosimlib/units/OscillatorUnit.h>
#include <vosimlib/units/VosimOscillator.h>
#include <vosimlib/units/ADSREnvelope.h>
#include <vosimlib/units/Follower.h>
#include <vosimlib/units/MathUnits.h>
#include <vosimlib/units/MemoryUnit.h>
#include <vosimlib/units/MidiUnits.h>
#include <vosimlib/units/StateVariableFilter.h>
#include <vosimlib/UnitFactory.h>
#include <vosimlib/tables.h>
#include "vosimsynth/MainWindow.h"
#include "vosimsynth/MainGUI.h"
#include "vosimsynth/widgets/CircuitWidget.h"
#include "vosimsynth/widgets/SummerUnitWidget.h"
#include "vosimsynth/widgets/GainUnitWidget.h"
#include "vosimsynth/widgets/OscilloscopeWidget.h"
#include "vosimsynth/Logging.h"

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
    : IPLUG_CTOR(0, 1, instanceInfo),
      m_voiceManager(),
      m_MIDIReceiver(m_voiceManager),
      m_tempo(0),
      m_tickCount(0)
{
    TIME_TRACE;
    makeInstrument();
    makeGraphics();
    syn::lut_bl_tri_table();
    syn::lut_bl_saw_table();
    syn::lut_bl_square_table();
}

void VOSIMSynth::makeGraphics() {
    TIME_TRACE;
    syn::VoiceManager* vm = &m_voiceManager;
    synui::MainWindow* mainWindow = new synui::MainWindow(GUI_WIDTH, GUI_HEIGHT, [vm](synui::MainWindow* a_win) { return new synui::MainGUI(a_win, vm); });
    mainWindow->setHInstance(gHInstance);
    mainWindow->onResize.connect_member(this, &VOSIMSynth::ResizeGraphics);
    AttachAppWindow(mainWindow);
    registerUnitWidgets(*mainWindow->getGUI()->circuitWidget());
}

void VOSIMSynth::makeInstrument() {
    TIME_TRACE;
    registerUnits();
    m_voiceManager.setMaxVoices(8);
}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
    // Mutex is already locked for us.

    for (int s = 0; s < nFrames; s++) {
        m_MIDIReceiver.advance();
    }

    // If tempo has changed, notify instrument
    if (m_tempo != GetTempo()) {
        m_tempo = GetTempo();
        m_voiceManager.setTempo(GetTempo());
    }

    // Process samples
    m_voiceManager.tick(inputs[0], inputs[1], outputs[0], outputs[1]);

    m_MIDIReceiver.Flush(nFrames);
    m_tickCount++;
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg) {
    m_MIDIReceiver.onMessageReceived(pMsg);
}

bool VOSIMSynth::SerializeState(ByteChunk* pChunk) {
    TIME_TRACE
    const syn::Circuit& circuit = m_voiceManager.getPrototypeCircuit();
    std::stringstream ss;
    json j;
    // Store synthesizer data
    json& synth = j["synth"] = json();
    synth["circuit"] = circuit.operator json();

    // Store gui data
    j["gui"] = GetAppWindow()->operator json();

    ss << j;
    pChunk->PutStr(ss.str().c_str());
    return true;
}

int VOSIMSynth::UnserializeState(ByteChunk* pChunk, int startPos) {
    TIME_TRACE
    syn::UnitFactory::instance().resetBuildCounts();
    string input;
    startPos = pChunk->Get(&input, startPos);
    std::stringstream ss{ input };

    try {
        json j; ss >> j;

        // Main JSON components
        const json& synth = j["synth"];
        const json& gui = j["gui"];

        syn::Unit* circuit = syn::Unit::fromJSON(synth["circuit"]);
        if (!circuit) {
            throw std::runtime_error("Error loading circuit.");
        }

        // Reset gui
        GetAppWindow()->reset();
        // Load new circuit into voice manager
        m_voiceManager.setPrototypeCircuit(*static_cast<const syn::Circuit*>(circuit));
        // Inform new circuit of buffer size, sampling rate, etc...
        Reset();
        // Load gui
        GetAppWindow()->load(gui);
        m_voiceManager.onIdle();
        startPos += ss.gcount();
        return startPos;
    } catch (const std::exception& e) {
        std::ostringstream alertmsg;
        alertmsg << "Unable to load preset!" << std::endl;
        alertmsg << e.what();
        GetAppWindow()->getGUI()->alert("Error", alertmsg.str(), nanogui::MessageDialog::Type::Warning);
        return -1;
    }
}

void VOSIMSynth::OnIdle() {
    m_voiceManager.onIdle();
}

void VOSIMSynth::registerUnits()
{
    TIME_TRACE
    syn::UnitFactory& uf = syn::UnitFactory::instance();
    uf.addUnitPrototype<syn::BasicOscillatorUnit>("Oscillators", "basic");
    uf.addUnitPrototype<syn::VosimOscillator>("Oscillators", "vosim");
    uf.addUnitPrototype<syn::FormantOscillator>("Oscillators", "fmt");

    uf.addUnitPrototype<syn::ADSREnvelope>("Modulators", "ADSR");
    uf.addUnitPrototype<syn::LFOOscillatorUnit>("Modulators", "LFO");

    uf.addUnitPrototype<syn::StateVariableFilter>("Filters", "SVF");
    uf.addUnitPrototype<syn::TrapStateVariableFilter>("Filters", "TSVF");
    uf.addUnitPrototype<syn::LadderFilterA>("Filters", "LdrA");
    uf.addUnitPrototype<syn::LadderFilterB>("Filters", "LdrB");

    uf.addUnitPrototype<syn::OnePoleLPUnit>("Filters", "lag");
    uf.addUnitPrototype<syn::FollowerUnit>("Filters", "follow");
    uf.addUnitPrototype<syn::DCRemoverUnit>("Filters", "DC");
    
    uf.addUnitPrototype<syn::SummerUnit>("Math", "sum");
    uf.addUnitPrototype<syn::GainUnit>("Math", "gain");
    uf.addUnitPrototype<syn::LerpUnit>("Math", "affine");
    uf.addUnitPrototype<syn::RectifierUnit>("Math", "rect");
    uf.addUnitPrototype<syn::QuantizerUnit>("Math", "quant");
    uf.addUnitPrototype<syn::PanningUnit>("Math", "pan");
    uf.addUnitPrototype<syn::SwitchUnit>("Math", "switch");
    uf.addUnitPrototype<syn::ConstantUnit>("Math", "const");
    uf.addUnitPrototype<syn::TanhUnit>("Math", "tanh");

    uf.addUnitPrototype<syn::MemoryUnit>("Delays", "z^-1");
    uf.addUnitPrototype<syn::VariableMemoryUnit>("Delays", "z^-t");
    
    uf.addUnitPrototype<syn::PitchToFreqUnit>("Converters", "p2f");
    uf.addUnitPrototype<syn::FreqToPitchUnit>("Converters", "f2p");

    uf.addUnitPrototype<syn::GateUnit>("MIDI", "gate");
    uf.addUnitPrototype<syn::MidiNoteUnit>("MIDI", "pitch");
    uf.addUnitPrototype<syn::VelocityUnit>("MIDI", "vel");
    uf.addUnitPrototype<syn::MidiCCUnit>("MIDI", "CC");
    uf.addUnitPrototype<syn::VoiceIndexUnit>("MIDI", "voice");
    
    uf.addUnitPrototype<synui::OscilloscopeUnit>("Visualizers", "scope");

    uf.addUnitPrototype<syn::Circuit>("", "circuit");
    uf.addUnitPrototype<syn::InputUnit>("", "in");
    uf.addUnitPrototype<syn::OutputUnit>("", "out");
}

void VOSIMSynth::registerUnitWidgets(synui::CircuitWidget& a_cw)
{
    TIME_TRACE
    a_cw.registerUnitWidget<syn::SummerUnit>([](synui::CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new synui::SummerUnitWidget(parent, a_vm, unitId); });
    a_cw.registerUnitWidget<syn::GainUnit>([](synui::CircuitWidget* parent, syn::VoiceManager* a_vm, int unitId) { return new synui::GainUnitWidget(parent, a_vm, unitId); });
}


void VOSIMSynth::Reset() {
    TIME_TRACE
    m_MIDIReceiver.Resize(GetBlockSize());
    m_voiceManager.setBufferSize(GetBlockSize());
    m_voiceManager.setFs(GetSampleRate());
}