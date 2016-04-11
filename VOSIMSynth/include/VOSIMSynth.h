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

#include "IPlug_include_in_plug_hdr.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"
#include "UnitFactory.h"
#include "CircuitPanel.h"

namespace syn{
	class KeyboardControl;
}

using namespace syn;
using namespace std;

class VOSIMSynth : public IPlug {
public:
	explicit VOSIMSynth(IPlugInstanceInfo instanceInfo);

    void makeGraphics();

    void makeInstrument();

    virtual ~VOSIMSynth() {
	    if(m_voiceManager)
			DELETE_NULL(m_voiceManager);
		// this will get deleted by IGraphics
//		if (m_circuitPanel)
//			DELETE_NULL(m_circuitPanel);
		if (m_unitFactory)
			DELETE_NULL(m_unitFactory);
		if (m_MIDIReceiver)
			DELETE_NULL(m_MIDIReceiver);
    };

    void Reset() override;

    void OnParamChange(int paramIdx) override;

    void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) override;

    void ProcessMidiMsg(IMidiMsg* pMsg) override;

    bool SerializeState(ByteChunk* pChunk) override;

    int UnserializeState(ByteChunk* pChunk, int startPos) override;

    void PresetsChangedByHost() override;

	void OnIdle() override;

	void OnActivate(bool active) override;

	bool isTransportRunning();

	int getTickCount() const { return m_tickCount; }

	void OnGUIOpen() override;

	void OnGUIClose() override;

private:
    MIDIReceiver* m_MIDIReceiver;
	VoiceManager* m_voiceManager;
	CircuitPanel* m_circuitPanel;
	UnitFactory* m_unitFactory;

	int m_tempo;
	unsigned m_tickCount;
	ITimeInfo m_timeInfo;
};

#endif
