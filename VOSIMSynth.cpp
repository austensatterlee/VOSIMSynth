#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

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
  GetParam(kKnob4)->InitDouble("Env1S", -5, -30, 0, 1, "Env1 S");
  GetParam(kKnob4)->SetShape(1.);

  GetParam(kKnob5)->InitDouble("VOSIMWidth", 0.5, 0, 1, 1./36, "VOSIM Width");
  GetParam(kKnob5)->SetShape(1.);
  GetParam(kKnob6)->InitInt("VOSIMNumber", 5, 1, 16, "VOSIM Number");
  GetParam(kKnob6)->SetShape(1.);
  GetParam(kKnob7)->InitDouble("VOSIMDecay", 0.001, 0.0, 1.0, 0.001, "VOSIM Decay");
  GetParam(kKnob7)->SetShape(1.);
  GetParam(kKnob10)->InitDouble("VWidthLFOf", 0.001, 0.0, 1.0, 0.001, "VOSIM Width LFO Freq");
  GetParam(kKnob10)->SetShape(1.);
  GetParam(kKnob11)->InitDouble("VWidthLFOg", 0.001, 0.0, 1.0, 0.001, "VOSIM Width LFO Gain");
  GetParam(kKnob11)->SetShape(1.);

  GetParam(kKnob8)->InitDouble("PLFOf", 0.001, 0.0, 1.0, 0.001, "Pitch LFO Freq");
  GetParam(kKnob8)->SetShape(1.);
  GetParam(kKnob9)->InitDouble("PLFOg", 0.001, 0.0, 1.0, 0.001, "Pitch LFO Gain");
  GetParam(kKnob9)->SetShape(1.);

  GetParam(kWedgeSwitch1)->InitInt("VOSIMWidthMode", 0, 0, 1, "VOSIM Width Mode");

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_WHITE);

  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnob1Frames);

  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X, kKnob1Y, kKnob1, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X+knob.W, kKnob1Y, kKnob2, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 2, kKnob1Y, kKnob3, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W, kKnob1Y + knob.H / knob.N, kKnob4, &knob));

  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 4, kKnob1Y, kKnob5, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 5, kKnob1Y, kKnob6, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 6, kKnob1Y, kKnob7, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 4, kKnob1Y + 1 * knob.H / knob.N, kKnob10, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 4, kKnob1Y + 2 * knob.H / knob.N, kKnob11, &knob));

  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 0, kKnob1Y + 4*knob.H / knob.N, kKnob8, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 0, kKnob1Y + 5*knob.H / knob.N, kKnob9, &knob));

  IBitmap wedgeswitch = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
  pGraphics->AttachControl(new ISwitchControl(this, 700, 10, kWedgeSwitch1, &wedgeswitch));

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
  mMIDIReceiver.noteOn.Connect(this, &VOSIMSynth::TriggerNote);
  mMIDIReceiver.noteOff.Connect(this, &VOSIMSynth::ReleaseNote);

  mVoiceStack = new uint8_t[NUM_VOICES];
  for (uint8_t i = 0; i < NUM_VOICES; i++) {
	  mVoiceStack[i] = 0;
  }
  mOsc = new VOSIM[NUM_VOICES];
  mAmpEnv = new Envelope[NUM_VOICES];
  mLFOVWidth.mWaveform = SINE_WAVE;
  mLFOPitch.mWaveform = SINE_WAVE;
}

VOSIMSynth::~VOSIMSynth() {}

void VOSIMSynth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.
	double *leftOutput = outputs[0];
	double *rightOutput = outputs[1];
	uint8_t i;
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
					mOsc[i].applyMods();
				}
				mOsc[i].tick();				
				leftOutput[s] += mOsc[i].getOutput()*mAmpEnv[i].mOutput;
			} else {
				mVoiceStack[i] = 0;
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
		if (mVoiceStack[i] == 0) {
			mVoiceStack[i] = noteNumber;
			mAmpEnv[i].trigger();
			mOsc[i].setFreq(mMIDIReceiver.getLastFrequency());
			mOsc[i].setGain(velocity*0.0078125);
			break;
		}
	}
}

void VOSIMSynth::ReleaseNote(const int noteNumber, const int velocity) {
	uint8_t i = NUM_VOICES;
	while (i--) {
		if (mVoiceStack[i] == noteNumber) {
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
	  mOsc[i].setFs(fs);
	  mAmpEnv[i].setFs(fs/MOD_FS_RAT);
  }
  mLFOPitch.setFs(fs / MOD_FS_RAT);
  mLFOVWidth.setFs(fs / MOD_FS_RAT);
}

void VOSIMSynth::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  uint8_t n = NUM_VOICES;
  switch (paramIdx)
  {
    case kKnob1:
		while (n--)
			mAmpEnv[n].setPeriod(0, GetParam(kKnob1)->Value(), MIN_ENV_SHAPE);
		break;
	case kKnob2:
		while (n--)
			mAmpEnv[n].setPeriod(1, GetParam(kKnob2)->Value(), MIN_ENV_SHAPE);
		break;
	case kKnob3:
		while (n--)
			mAmpEnv[n].setPeriod(2, GetParam(kKnob3)->Value(), MIN_ENV_SHAPE);
		break;
	case kKnob4:
		while (n--)
			mAmpEnv[n].setPoint_dB(1, GetParam(kKnob4)->Value());
		break;
	case kKnob5:
		while (n--)
			mOsc[n].setPFreq(300*pow(2.0, 5*GetParam(kKnob5)->Value()));
		break;
	case kKnob6:
		while (n--)
			mOsc[n].setNumber(GetParam(kKnob6)->Value());
		break;
	case kKnob7:
		while (n--)
			mOsc[n].setDecay(1.0-pow(10,-3*(GetParam(kKnob7)->Value())));
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
		mLFOVWidth.setGain(pow(2, -12 * (1 - GetParam(kKnob11)->Value())));
		break;
	case kWedgeSwitch1:
		//while (n--)
			//mOsc[n].setWidthMode(GetParam(kWedgeSwitch1)->Value());
		break;
    default:
      break;
  }
}
