#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "UI.h"

const int kNumPrograms = 1;

enum EParams
{
	kKnob1 = 0,
	kKnob2,
	kKnob3,
	kKnob4,
	kKnob5,
	kKnob6,
	kKnob7,
	kKnob8,
	kKnob9,
	kKnob10,
	kKnob11,
	kKnob12,
	kKnob13,
	kKnob14,
	kKnob15,
	kKnob16,
	kKnob17,
	kKnob18,
	kKnob19,
	kKnob20,
	kKnob21,
	kWedgeSwitch1,
	kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kKnob1X = 10,
  kKnob1Y = 10,
  kKnob1Frames = 60
};

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
	mVolume(1.0)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kKnob1)->InitDouble("Env1A", 0.1, 0.01, 10.0, 0.01, "Env1 A");
  GetParam(kKnob1)->SetShape(3.);
  GetParam(kKnob2)->InitDouble("Env1D", 0.5, 0.01, 10.0, 0.01, "Env1 D");
  GetParam(kKnob2)->SetShape(3.);
  GetParam(kKnob3)->InitDouble("Env1R", 0.1, 0.01, 10.0, 0.01, "Env1 R");
  GetParam(kKnob3)->SetShape(3.);
  GetParam(kKnob4)->InitDouble("Env1S", 0.8, 0, 1, 0.001, "Env1 S");
  GetParam(kKnob5)->InitDouble("O1VF", 0.5, 0, 1, 0.001, "Osc1 VF");
  GetParam(kKnob5)->SetShape(2.);
  GetParam(kKnob6)->InitDouble("O1N", 0.25, 0, 0.5, 0.001, "Osc1 Number");
  GetParam(kKnob7)->InitDouble("O1D", 0.001, 0.0, 1.0, 0.001, "Osc1 Decay");
  GetParam(kKnob7)->SetShape(5.);
  GetParam(kKnob8)->InitDouble("O1PMf", 0.001, 0.0, 1.0, 0.001, "Osc1 P. Mod F");
  GetParam(kKnob9)->InitDouble("O1PMg", 0.001, 0.0, 1.0, 0.001, "Osc1 P. Mod G");
  GetParam(kKnob10)->InitDouble("O1WMf", 0.001, 0.0, 1.0, 0.001, "Osc1 VF. Mod F");
  GetParam(kKnob11)->InitDouble("O1WMg", 0.001, 0.0, 1.0, 0.001, "Osc1 VF. Mod G");
  GetParam(kKnob12)->InitDouble("Env1AShp", 0.1, 0.0, 1.0, 0.001, "A Shape");
  GetParam(kKnob13)->InitDouble("Env1DShp", 0.1, 0.0, 1.0, 0.001, "D Shape");
  GetParam(kKnob14)->InitDouble("Env1RShp", 0.1, 0.0, 1.0, 0.001, "R Shape");
  GetParam(kKnob15)->InitDouble("Volume", 0.0, -10.0, 10.0, 1, "Volume");

  GetParam(kKnob16)->InitDouble("O2VF", 0.5, 0, 1, 0.001, "Osc2 VF");
  GetParam(kKnob16)->SetShape(2.);
  GetParam(kKnob17)->InitDouble("O2N", 0.25, 0, 0.5, 0.001, "Osc2 Number");
  GetParam(kKnob18)->InitDouble("O2D", 0.001, 0.0, 1.0, 0.001, "Osc2 Decay");
  GetParam(kKnob18)->SetShape(5.);

  GetParam(kKnob19)->InitDouble("O3VF", 0.5, 0, 1, 0.001, "Osc3 VF");
  GetParam(kKnob19)->SetShape(2.);
  GetParam(kKnob20)->InitDouble("O3N", 0.25, 0, 0.5, 0.001, "Osc3 Number");
  GetParam(kKnob21)->InitDouble("O3D", 0.001, 0.0, 1.0, 0.001, "Osc3 Decay");
  GetParam(kKnob21)->SetShape(5.);

  GetParam(kWedgeSwitch1)->InitInt("Oversampling", 0, 0, 1, "Oversampling");

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_BLACK);

  IBitmap wedgeswitch2p = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
  IBitmap toggleswitch3p = pGraphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnob1Frames);

  attachKnob(pGraphics, this, 0, 0, kKnob1, &knob);
  attachKnob(pGraphics, this, 0, 1, kKnob2, &knob);
  attachKnob(pGraphics, this, 0, 3, kKnob3, &knob);
  attachKnob(pGraphics, this, 0, 2, kKnob4, &knob); 
  attachKnob(pGraphics, this, 0, 4, kKnob5, &knob);
  attachKnob(pGraphics, this, 0, 5, kKnob6, &knob);
  attachKnob(pGraphics, this, 0, 6, kKnob7, &knob);
  attachKnob(pGraphics, this, 4, 0, kKnob8, &knob);
  attachKnob(pGraphics, this, 5, 0, kKnob9, &knob);
  //attachKnob(pGraphics, this, 1, 4, kKnob10, &knob);
  //attachKnob(pGraphics, this, 2, 4, kKnob11, &knob);
  attachKnob(pGraphics, this, 1, 0, kKnob12, &knob);
  attachKnob(pGraphics, this, 1, 1, kKnob13, &knob);
  attachKnob(pGraphics, this, 1, 3, kKnob14, &knob);
  attachKnob(pGraphics, this, 0, 7, kKnob15, &knob);

  attachKnob(pGraphics, this, 1, 4, kKnob16, &knob);
  attachKnob(pGraphics, this, 1, 5, kKnob17, &knob);
  attachKnob(pGraphics, this, 1, 6, kKnob18, &knob);

  attachKnob(pGraphics, this, 2, 4, kKnob19, &knob);
  attachKnob(pGraphics, this, 2, 5, kKnob20, &knob);
  attachKnob(pGraphics, this, 2, 6, kKnob21, &knob);

  pGraphics->AttachControl(new ISwitchControl(this, 700, 10, kWedgeSwitch1, &wedgeswitch2p));
  //pGraphics->AttachControl(new ISwitchControl(this, 600, 10, kWedgeSwitch1, &toggleswitch3p));

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
  mVoiceManager.setNumVoices(NUM_VOICES);
  mMIDIReceiver.noteOn.Connect(&mVoiceManager, &VoiceManager::TriggerNote);
  mMIDIReceiver.noteOff.Connect(&mVoiceManager, &VoiceManager::ReleaseNote);
}

VOSIMSynth::~VOSIMSynth() {}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.
	double *leftOutput = outputs[0];
	double *rightOutput = outputs[1];
	uint8_t j;
	for (int s = 0; s < nFrames; s++) {
		leftOutput[s] = 0;
		mMIDIReceiver.advance();
		leftOutput[s] = mVoiceManager.tick()*mVolume;
		rightOutput[s] = leftOutput[s];
	}
	mMIDIReceiver.Flush(nFrames);
}

void VOSIMSynth::ProcessMidiMsg(IMidiMsg* pMsg) {
	mMIDIReceiver.onMessageReceived(pMsg);
}

void VOSIMSynth::Reset()
{
  TRACE;
  IMutexLock lock(this);
  uint8_t i = NUM_VOICES;
  double fs = GetSampleRate();
  mVoiceManager.setFs(fs);
}

void VOSIMSynth::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  uint8_t n = NUM_VOICES;
  switch (paramIdx)
  {
    case kKnob1:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setPeriod(0, GetParam(kKnob1)->Value(), GetParam(kKnob12)->Value());
		break;
	case kKnob2:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setPeriod(1, GetParam(kKnob2)->Value(), GetParam(kKnob13)->Value());
		break;
	case kKnob3:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setPeriod(2, GetParam(kKnob3)->Value(), GetParam(kKnob14)->Value());
		break;
	case kKnob4:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setPoint(1, GetParam(kKnob4)->Value());
		break;
	case kKnob5:
		while (n--)
			mVoiceManager.voices[n].mOsc[0].scalePFreq(GetParam(kKnob5)->Value());
		break;
	case kKnob6:
		while (n--)
			mVoiceManager.voices[n].mOsc[0].setNumber(GetParam(kKnob6)->Value());
		break;
	case kKnob7:
		while (n--)
			mVoiceManager.voices[n].mOsc[0].setDecay(pow(10,-3*(GetParam(kKnob7)->Value())));
		break;
	case kKnob8:
		while (n--)
			mVoiceManager.voices[n].mLFOPitch.setFreq(pow(2, 5 * GetParam(kKnob8)->Value()));
		break;
	case kKnob9:
		while (n--)
			mVoiceManager.voices[n].mLFOPitch.setGain(pow(2, -12*(1-GetParam(kKnob9)->Value())));
		break;
	case kKnob10:
		break;
	case kKnob11:
		break;
	case kKnob12:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setShape(0, GetParam(kKnob12)->Value());
		break;
	case kKnob13:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setShape(1, GetParam(kKnob13)->Value());
		break;
	case kKnob14:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setShape(2, GetParam(kKnob14)->Value());
		break;
	case kKnob15:
		mVolume = pow(10.0, 0.05*GetParam(kKnob15)->Value());
		break;
	case kWedgeSwitch1:
		mVoiceManager.mOversampling = 1 + GetParam(kWedgeSwitch1)->Value() * 7;
		Reset();
		break;
	case kKnob16:
		while (n--)
			mVoiceManager.voices[n].mOsc[1].scalePFreq(GetParam(kKnob16)->Value());
		break;
	case kKnob17:
		while (n--)
			mVoiceManager.voices[n].mOsc[1].setNumber(GetParam(kKnob17)->Value());
		break;
	case kKnob18:
		while (n--)
			mVoiceManager.voices[n].mOsc[1].setDecay(pow(10, -3 * (GetParam(kKnob18)->Value())));
		break;
	case kKnob19:
		while (n--)
			mVoiceManager.voices[n].mOsc[2].scalePFreq(GetParam(kKnob19)->Value());
		break;
	case kKnob20:
		while (n--)
			mVoiceManager.voices[n].mOsc[2].setNumber(GetParam(kKnob20)->Value());
		break;
	case kKnob21:
		while (n--)
			mVoiceManager.voices[n].mOsc[2].setDecay(pow(10, -3 * (GetParam(kKnob21)->Value())));
		break;
    default:
      break;
  }
}
