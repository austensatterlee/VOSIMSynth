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
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kKnob1)->InitDouble("Env1A", 0.1, 0.01, 10.0, 0.01, "Env1 A");
  GetParam(kKnob1)->SetShape(3.);
  GetParam(kKnob2)->InitDouble("Env1D", 0.5, 0.01, 10.0, 0.01, "Env1 D");
  GetParam(kKnob2)->SetShape(3.);
  GetParam(kKnob3)->InitDouble("Env1R", 0.1, 0.01, 10.0, 0.01, "Env1 R");
  GetParam(kKnob3)->SetShape(3.);
  GetParam(kKnob4)->InitDouble("Env1S", -5, -60, 0, 1, "Env1 S");
  GetParam(kKnob5)->InitDouble("O1VF", 0.5, 0, 1, 0.001, "Osc1 VF");
  GetParam(kKnob6)->InitInt("O1N", 1, 1, 64, "Osc1 Number");
  GetParam(kKnob6)->SetShape(2.);
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

  GetParam(kWedgeSwitch1)->InitInt("VOSIMWidthMode", 0, 0, 1, "VOSIM Width Mode");

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_BLACK);

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
  attachKnob(pGraphics, this, 1, 4, kKnob10, &knob);
  attachKnob(pGraphics, this, 2, 4, kKnob11, &knob);
  attachKnob(pGraphics, this, 1, 0, kKnob12, &knob);
  attachKnob(pGraphics, this, 1, 1, kKnob13, &knob);
  attachKnob(pGraphics, this, 1, 3, kKnob14, &knob);
  attachKnob(pGraphics, this, 0, 7, kKnob15, &knob);

  IBitmap wedgeswitch = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
  pGraphics->AttachControl(new ISwitchControl(this, 700, 10, kWedgeSwitch1, &wedgeswitch));

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
  mMIDIReceiver.noteOn.Connect(this, &VOSIMSynth::TriggerNote);
  mMIDIReceiver.noteOff.Connect(this, &VOSIMSynth::ReleaseNote);

  mOsc = new VOSIM[NUM_VOICES];
  mAmpEnv = new Envelope[NUM_VOICES];
  mLFOVWidth.mWaveform = SINE_WAVE;
  mLFOPitch.mWaveform = SINE_WAVE;
  mVolume = 1.0;
  mOversampling = 1;
}

VOSIMSynth::~VOSIMSynth() {}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.
	double *leftOutput = outputs[0];
	double *rightOutput = outputs[1];
	uint8_t i,j;
	for (int s = 0; s < nFrames; s++) {
		mSampleCount++;
		leftOutput[s] = 0;
		mMIDIReceiver.advance();
		if (!(mSampleCount & MOD_FS_RAT)) {
			mLFOPitch.tick();
			mLFOVWidth.tick();
		}

		i = NUM_VOICES;
		while (i--) {
			if (!mAmpEnv[i].mIsDone) {
				if ( !( mSampleCount & MOD_FS_RAT ) ) {
					mAmpEnv[i].tick();
					mOsc[i].modFreq(mLFOPitch.getOutput());
					mOsc[i].modPFreq(mLFOVWidth.getOutput());
					mOsc[i].modGain(mAmpEnv[i].mOutput*mVolume);
					mOsc[i].applyMods();
				}
				j = mOversampling;
				while (j--) {
					mOsc[i].tick();
					leftOutput[s] += mOsc[i].getOutput()/mOversampling;
				}
			} else {
				mVoiceStack[i][0] = 0;
				mVoiceStack[i][1] = 0;
			}
		}			
		leftOutput[s] *= 1.0 / NUM_VOICES;
		rightOutput[s] = leftOutput[s];
	}
	mMIDIReceiver.Flush(nFrames);
}

void VOSIMSynth::TriggerNote(const int noteNumber, const int velocity) {
	uint8_t i = NUM_VOICES;
	while (i--) {
		if (mVoiceStack[i][0] == 0 && mVoiceStack[i][1] == 0) {
			mVoiceStack[i][0] = noteNumber;
			mVoiceStack[i][1] = velocity;
			mAmpEnv[i].trigger();
			mOsc[i].setFreq(noteNumberToFrequency(noteNumber));
			mOsc[i].setGain(velocity*0.0078125);
			mOsc[i].modGain(mAmpEnv[i].mOutput);
			mOsc[i].applyMods();
			mOsc[i].sync();
			break;
		}
	}
}

void VOSIMSynth::ReleaseNote(const int noteNumber, const int velocity) {
	uint8_t i = NUM_VOICES;
	while (i--) {
		if (mVoiceStack[i][0] == noteNumber) {
			mAmpEnv[i].release();
		}
	}
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
  while (i--) {
	  mOsc[i].setFs(fs*mOversampling);
	  mAmpEnv[i].setFs(fs / (MOD_FS_RAT + 1));
  }
  mLFOPitch.setFs(fs / (MOD_FS_RAT+1));
  mLFOVWidth.setFs(fs / (MOD_FS_RAT+1));
}

void VOSIMSynth::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  uint8_t n = NUM_VOICES;
  switch (paramIdx)
  {
    case kKnob1:
		while (n--)
			mAmpEnv[n].setPeriod(0, GetParam(kKnob1)->Value(), GetParam(kKnob12)->Value());
		break;
	case kKnob2:
		while (n--)
			mAmpEnv[n].setPeriod(1, GetParam(kKnob2)->Value(), GetParam(kKnob13)->Value());
		break;
	case kKnob3:
		while (n--)
			mAmpEnv[n].setPeriod(2, GetParam(kKnob3)->Value(), GetParam(kKnob14)->Value());
		break;
	case kKnob4:
		while (n--)
			mAmpEnv[n].setPoint_dB(1, GetParam(kKnob4)->Value());
		break;
	case kKnob5:
		while (n--)
			mOsc[n].scalePFreq(GetParam(kKnob5)->Value());
		break;
	case kKnob6:
		while (n--)
			mOsc[n].setNumber(GetParam(kKnob6)->Value());
		break;
	case kKnob7:
		while (n--)
			mOsc[n].setDecay(pow(10,-3*(GetParam(kKnob7)->Value())));
		break;
	case kKnob8:
		mLFOPitch.setFreq(pow(2, 5 * GetParam(kKnob8)->Value()));
		break;
	case kKnob9:
		mLFOPitch.setGain(pow(2, -12*(1-GetParam(kKnob9)->Value())));
		break;
	case kKnob10:
		mLFOVWidth.setFreq(pow(2, 5 * GetParam(kKnob10)->Value()));
		break;
	case kKnob11:
		mLFOVWidth.setGain(pow(2, -1 - 12 * (1 - GetParam(kKnob11)->Value())));
		break;
	case kKnob12:
		while (n--)
			mAmpEnv[n].setShape(0, GetParam(kKnob12)->Value());
		break;
	case kKnob13:
		while (n--)
			mAmpEnv[n].setShape(1, GetParam(kKnob13)->Value());
		break;
	case kKnob14:
		while (n--)
			mAmpEnv[n].setShape(2, GetParam(kKnob14)->Value());
		break;
	case kKnob15:
		mVolume = pow(10.0,0.05*GetParam(kKnob15)->Value());
		break;
	case kWedgeSwitch1:
		mOversampling = 1 + GetParam(kWedgeSwitch1)->Value() * 7;
		Reset();
		break;
    default:
      break;
  }
}
