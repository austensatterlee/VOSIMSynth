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
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kKnob1X = 10,
  kKnob1Y = 250,
  kKnob1Frames = 60
};

VOSIMSynth::VOSIMSynth(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kKnob1)->InitDouble("Knob1", 0.1, 0.1, 1.0, 0.01, "%");
  GetParam(kKnob1)->SetShape(1.);
  GetParam(kKnob2)->InitDouble("Knob2", 0.1, 0.0, 1.0, 0.001, "%");
  GetParam(kKnob2)->SetShape(1.);
  GetParam(kKnob3)->InitDouble("Knob3", 0.5, 0.0, 1.0, 0.001, "%");
  GetParam(kKnob3)->SetShape(1.);
  GetParam(kKnob4)->InitDouble("Knob4", 0.9, 0.0, 1.0, 0.001, "%");
  GetParam(kKnob4)->SetShape(1.);

  GetParam(kKnob5)->InitDouble("VOSIM Width", 0.0001, 0.0001, 0.001250, 0.00001, "vosim");
  GetParam(kKnob5)->SetShape(1.);
  GetParam(kKnob6)->InitInt("VOSIM Number", 5, 1, 100, "vosim");
  GetParam(kKnob6)->SetShape(1.);
  GetParam(kKnob7)->InitDouble("VOSIM Decay", 0.01, 0.0, 1.0, 0.0001, "vosim");
  GetParam(kKnob7)->SetShape(2.);

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_WHITE);

  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnob1Frames);

  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X, kKnob1Y, kKnob1, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X+knob.W, kKnob1Y, kKnob2, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 2, kKnob1Y, kKnob3, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W, kKnob1Y - knob.H / knob.N, kKnob4, &knob));

  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 4, kKnob1Y, kKnob5, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 5, kKnob1Y, kKnob6, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kKnob1X + knob.W * 6, kKnob1Y, kKnob7, &knob));

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
		mMIDIReceiver.advance();
		leftOutput[s] = 0;
		i = NUM_VOICES;
		while (i--) {
			if (!mAmpEnv[i].mIsDone) {
				if ( !( mSampleCount & MOD_FS_RAT ) ) {
					mAmpEnv[i].tick();
				}
				mOsc[i].tick();				
				leftOutput[s] += mOsc[i].getOutput()*mAmpEnv[i].mOutput;
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
			mVoiceStack[i] = 0;
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
}

void VOSIMSynth::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  uint8_t n = NUM_VOICES;
  switch (paramIdx)
  {
    case kKnob1:
		while (n--)
			mAmpEnv[n].setPeriod(0, GetParam(kKnob1)->Value());
		break;
	case kKnob2:
		while (n--)
			mAmpEnv[n].setPeriod(1, GetParam(kKnob2)->Value());
		break;
	case kKnob3:
		while (n--)
			mAmpEnv[n].setPeriod(2, GetParam(kKnob3)->Value());
		break;
	case kKnob4:
		while (n--)
			mAmpEnv[n].setPoint_dB(1, -60*(1-GetParam(kKnob4)->Value()));
		break;
	case kKnob5:
		while (n--)
			mOsc[n].setWidth(GetParam(kKnob5)->Value());
		break;
	case kKnob6:
		while (n--)
			mOsc[n].setNumber(GetParam(kKnob6)->Value());
		break;
	case kKnob7:
		while (n--)
			mOsc[n].setDecay(1.0-GetParam(kKnob7)->Value());
		break;
    default:
      break;
  }
}
