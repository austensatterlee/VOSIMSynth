#pragma once

#define PLUG_MFR "Austen"
#if defined(_RELWITHDEBINFO)
#define PLUG_NAME "VOSIMProject-rdb"
#elif defined(_DEBUG)
#define PLUG_NAME "VOSIMProject-debug"
#else
#define PLUG_NAME "VOSIMProject"
#endif

#define PLUG_CLASS_NAME VOSIMSynth

#define BUNDLE_MFR "Austen Satterlee"
#define BUNDLE_NAME PLUG_NAME

#define PLUG_ENTRY VOSIMSynth_Entry
#define PLUG_VIEW_ENTRY VOSIMSynth_ViewEntry

#define PLUG_ENTRY_STR "VOSIMSynth_Entry"
#define PLUG_VIEW_ENTRY_STR "VOSIMSynth_ViewEntry"

#define VIEW_CLASS VOSIMSynth_View
#define VIEW_CLASS_STR "VOSIMSynth_View"

// Format        0xMAJR.MN.BG - in HEX! so version 10.1.5 would be 0x000A0105
#define PLUG_VER 0x00010000
#define VST3_VER_STR "1.0.0"

// http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm
// 4 chars, single quotes. At least one capital letter
#define PLUG_UNIQUE_ID 'Ipef'
// make sure this is not the same as BUNDLE_MFR
#define PLUG_MFR_ID 'Aust'

#define PLUG_CHANNEL_IO "1-1 2-2"

#define PLUG_LATENCY 0
#define PLUG_IS_INST 1
#define EFFECT_TYPE_VST3 'Instrument|Synth'

// if this is 0 RTAS can't get tempo info
#define PLUG_DOES_MIDI 1

#define PLUG_DOES_STATE_CHUNKS 1

// GUI default dimensions
#define GUI_WIDTH 1000
#define GUI_HEIGHT 700

// on MSVC, you must define SA_API in the resource editor preprocessor macros as well as the c++ ones
#if defined(SA_API) && !defined(OS_IOS)
#include "app_resource.h"
#endif
