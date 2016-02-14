#ifndef __MIDIRECEIVER__
#define __MIDIRECEIVER__

#include <cstdint>
#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"
#include "VoiceManager.h"

/**
 * \brief Midi queue handler
 */
class MIDIReceiver {
public:
    MIDIReceiver(shared_ptr<syn::VoiceManager> a_vm) :
            m_vm(a_vm),
            m_offset(0)
    {
        for (int i = 0 ; i < s_keyCount ; i++) {
            m_keyStatus[i] = false;
        }
    };

    // Returns true if the key with a given index is currently pressed
    inline bool getKeyStatus(int keyIndex) const
    { return m_keyStatus[keyIndex]; }

    void advance();

    void onMessageReceived(IMidiMsg* midiMessage);

    inline void Flush(int nFrames)
    {
        m_midiQueue.Flush(nFrames);
        m_offset = 0;
    }

    inline void Resize(int blockSize)
    { m_midiQueue.Resize(blockSize); }

private:
    IMidiQueue m_midiQueue;
    static const int s_keyCount = 128;
    bool m_keyStatus[s_keyCount]; // array of on/off for each key (index is note number)
    int m_offset;
    shared_ptr<syn::VoiceManager> m_vm;
};

#endif