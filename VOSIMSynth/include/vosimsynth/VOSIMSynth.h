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

#ifndef __VOSIMSYNTH__
#define __VOSIMSYNTH__

#include <IPlug/IPlug_include_in_plug_hdr.h>

#include <VoiceManager.h>
#include <vosimsynth/MIDIReceiver.h>

namespace synui {
    class CircuitWidget;
}

class VOSIMSynth : public IPlug
{
public:
    explicit VOSIMSynth(IPlugInstanceInfo instanceInfo);

    void makeGraphics();
    void makeInstrument();
    void Reset() override;
    void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) override;
    void ProcessMidiMsg(IMidiMsg* pMsg) override;
    bool SerializeState(ByteChunk* pChunk) override;
    int UnserializeState(ByteChunk* pChunk, int startPos) override;
    void OnIdle() override;
    int getTickCount() const { return m_tickCount; }

    static void registerUnits();
    static void registerUnitWidgets(synui::CircuitWidget& a_cw);

private:
	syn::VoiceManager m_voiceManager;
    syn::MIDIReceiver m_MIDIReceiver;

    int m_tempo;
    unsigned m_tickCount;
};

#endif
