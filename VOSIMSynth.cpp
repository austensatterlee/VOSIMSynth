#include "VOSIMSynth.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include "UI.h"

const int kNumPrograms = 1;

enum EParams
{
    // Volume envelope
	kEnv1Atk = 0,
	kEnv1Dec,
	kEnv1Rel,
	kEnv1Sus,
    // Pitch LFO
    kOsc1PMF,
    kOsc1PMG,
    // Oscillator 1 params
	kOsc1VF,
	kOsc1Num,
	kOsc1Dec,
    kOsc1Vol,
    kEnv2Atk,
    kEnv2Dec,
    kEnv2Rel,
    kEnv2Sus,
    // Oscillator 2 params
    kOsc2VF,
    kOsc2Num,
    kOsc2Dec,
    kOsc2Vol,
    kEnv3Atk,
    kEnv3Dec,
    kEnv3Rel,
    kEnv3Sus,
    // Oscillator 3 params
    kOsc3VF,
    kOsc3Num,
    kOsc3Dec,
	kOsc3Vol,
    kEnv4Atk,
    kEnv4Dec,
    kEnv4Rel,
    kEnv4Sus,
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
	mVolume(1.0),
	mOversampling(1)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam( kEnv1Atk )->InitDouble( "Vol A", 0.1, 0.0, 1.0, 0.001, "Vol A" ); GetParam( kEnv1Atk )->SetShape( 3. );
  GetParam( kEnv1Dec )->InitDouble( "Vol D", 0.5, 0.0, 1.0, 0.001, "Vol D" ); GetParam( kEnv1Dec )->SetShape( 3. );
  GetParam( kEnv1Rel )->InitDouble( "Vol R", 0.1, 0.0, 1.0, 0.001, "Vol R" ); GetParam( kEnv1Rel )->SetShape( 3. );
  GetParam( kEnv1Sus )->InitDouble( "Vol S", 0.8, 0.0, 1.0, 0.001, "Vol S" );

  GetParam( kEnv2Atk )->InitDouble( "Osc1 VF A", 0.1, 0.0, 1.0, 0.001, "Osc1 VF A" ); GetParam( kEnv2Atk )->SetShape( 3. );
  GetParam( kEnv2Dec )->InitDouble( "Osc1 VF D", 0.5, 0.0, 1.0, 0.001, "Osc1 VF D" ); GetParam( kEnv2Dec )->SetShape( 3. );
  GetParam( kEnv2Rel )->InitDouble( "Osc1 VF R", 0.1, 0.0, 1.0, 0.001, "Osc1 VF R" ); GetParam( kEnv2Rel )->SetShape( 3. );
  GetParam( kEnv2Sus )->InitDouble( "Osc1 VF S", 0.8, 0.0, 1.0, 0.001, "Osc1 VF S" );

  GetParam( kEnv3Atk )->InitDouble( "Osc2 VF A", 0.1, 0.0, 1.0, 0.001, "Osc2 VF A" ); GetParam( kEnv3Atk )->SetShape( 3. );
  GetParam( kEnv3Dec )->InitDouble( "Osc2 VF D", 0.5, 0.0, 1.0, 0.001, "Osc2 VF D" ); GetParam( kEnv3Dec )->SetShape( 3. );
  GetParam( kEnv3Rel )->InitDouble( "Osc2 VF R", 0.1, 0.0, 1.0, 0.001, "Osc2 VF R" ); GetParam( kEnv3Rel )->SetShape( 3. );
  GetParam( kEnv3Sus )->InitDouble( "Osc2 VF S", 0.8, 0.0, 1.0, 0.001, "Osc2 VF S" );

  GetParam( kEnv4Atk )->InitDouble( "Osc3 VF A", 0.1, 0.0, 1.0, 0.001, "Osc3 VF A" ); GetParam( kEnv4Atk )->SetShape( 3. );
  GetParam( kEnv4Dec )->InitDouble( "Osc3 VF D", 0.5, 0.0, 1.0, 0.001, "Osc3 VF D" ); GetParam( kEnv4Dec )->SetShape( 3. );
  GetParam( kEnv4Rel )->InitDouble( "Osc3 VF R", 0.1, 0.0, 1.0, 0.001, "Osc3 VF R" ); GetParam( kEnv4Rel )->SetShape( 3. );
  GetParam( kEnv4Sus )->InitDouble( "Osc3 VF S", 0.8, 0.0, 1.0, 0.001, "Osc3 VF S" );

  GetParam( kOsc1PMF )->InitDouble( "OPMf", 0.001, 0.0, 1.0, 0.001, "Pitch Mod F" );
  GetParam( kOsc1PMG )->InitDouble( "OPMg", 0.001, 0.0, 1.0, 0.001, "Pitch Mod G" );

  GetParam( kOsc1VF )->InitDouble( "O1VF", 0.5, 0, 1, 0.000001, "Osc1 VF" );
  GetParam( kOsc1VF )->SetShape( 2. );
  GetParam( kOsc1Num )->InitDouble( "O1N", 0.25, 0, 1, 0.000001, "Osc1 Number" );
  GetParam( kOsc1Dec )->InitDouble( "O1D", 0.001, 0.0, 1.0, 0.000001, "Osc1 Decay" );
  GetParam( kOsc1Dec )->SetShape( 5. );
  GetParam( kOsc1Vol )->InitDouble( "O1Vol", 1, 0, 1, 0.001, "Osc1 Vol" );

  GetParam( kOsc2VF )->InitDouble( "O2VF", 0.5, 0, 1, 0.000001, "Osc2 VF" );
  GetParam( kOsc2VF )->SetShape( 2. );
  GetParam( kOsc2Num )->InitDouble( "O2N", 0.25, 0, 1, 0.000001, "Osc2 Number" );
  GetParam( kOsc2Dec )->InitDouble( "O2D", 0.001, 0.0, 1.0, 0.000001, "Osc2 Decay" );
  GetParam( kOsc2Dec )->SetShape( 5. );
  GetParam( kOsc2Vol )->InitDouble( "O2Vol", 1, 0, 1, 0.001, "Osc2 Vol" );

  GetParam( kOsc3VF )->InitDouble( "O3VF", 0.5, 0, 1, 0.000001, "Osc3 VF" );
  GetParam( kOsc3VF )->SetShape( 2. );
  GetParam( kOsc3Num )->InitDouble( "O3N", 0.25, 0, 1, 0.000001, "Osc3 Number" );
  GetParam( kOsc3Dec )->InitDouble( "O3D", 0.001, 0.0, 1.0, 0.000001, "Osc3 Decay" );
  GetParam( kOsc3Dec )->SetShape( 5. );
  GetParam( kOsc3Vol )->InitDouble( "O3Vol", 1, 0, 1, 0.001, "Osc3 Vol" );

  GetParam(kWedgeSwitch1)->InitInt("Oversampling", 0, 0, 1, "Oversampling");

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_BLACK);

  IBitmap wedgeswitch2p = pGraphics->LoadIBitmap(WEDGE_SWITCH_2P_ID, WEDGE_SWITCH_2P_FN, 2);
  IBitmap toggleswitch3p = pGraphics->LoadIBitmap(TOGGLE_SWITCH_3P_ID, TOGGLE_SWITCH_3P_FN, 3);
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnob1Frames);

  attachKnob(pGraphics, this, 0, 0, kEnv1Atk, &knob);
  attachKnob(pGraphics, this, 0, 1, kEnv1Dec, &knob);
  attachKnob(pGraphics, this, 0, 3, kEnv1Rel, &knob);
  attachKnob(pGraphics, this, 0, 2, kEnv1Sus, &knob);

  attachKnob( pGraphics, this, 1, 0, kEnv2Atk, &knob );
  attachKnob( pGraphics, this, 1, 1, kEnv2Dec, &knob );
  attachKnob( pGraphics, this, 1, 3, kEnv2Rel, &knob );
  attachKnob( pGraphics, this, 1, 2, kEnv2Sus, &knob );

  attachKnob( pGraphics, this, 2, 0, kEnv3Atk, &knob );
  attachKnob( pGraphics, this, 2, 1, kEnv3Dec, &knob );
  attachKnob( pGraphics, this, 2, 3, kEnv3Rel, &knob );
  attachKnob( pGraphics, this, 2, 2, kEnv3Sus, &knob );

  attachKnob( pGraphics, this, 3, 0, kEnv4Atk, &knob );
  attachKnob( pGraphics, this, 3, 1, kEnv4Dec, &knob );
  attachKnob( pGraphics, this, 3, 3, kEnv4Rel, &knob );
  attachKnob( pGraphics, this, 3, 2, kEnv4Sus, &knob );

  attachKnob(pGraphics, this, 4, 0, kOsc1PMF, &knob);
  attachKnob(pGraphics, this, 5, 0, kOsc1PMG, &knob);

  attachKnob( pGraphics, this, 1, 4, kOsc1VF, &knob );
  attachKnob( pGraphics, this, 1, 5, kOsc1Num, &knob );
  attachKnob( pGraphics, this, 1, 6, kOsc1Dec, &knob );
  attachKnob( pGraphics, this, 1, 7, kOsc1Vol, &knob );

  attachKnob(pGraphics, this, 2, 4, kOsc2VF, &knob);
  attachKnob(pGraphics, this, 2, 5, kOsc2Num, &knob);
  attachKnob(pGraphics, this, 2, 6, kOsc2Dec, &knob);
  attachKnob( pGraphics, this, 2, 7, kOsc2Vol, &knob );

  attachKnob(pGraphics, this, 3, 4, kOsc3VF, &knob);
  attachKnob(pGraphics, this, 3, 5, kOsc3Num, &knob);
  attachKnob(pGraphics, this, 3, 6, kOsc3Dec, &knob);
  attachKnob( pGraphics, this, 3, 7, kOsc3Vol, &knob );

  pGraphics->AttachControl(new ISwitchControl(this, 700, 10, kWedgeSwitch1, &wedgeswitch2p));
  //pGraphics->AttachControl(new ISwitchControl(this, 600, 10, kWedgeSwitch1, &toggleswitch3p));

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
  mVoiceManager.setNumVoices(NUM_VOICES);
  mMIDIReceiver.noteOn.Connect(&mVoiceManager, &VoiceManager::TriggerNote);
  mMIDIReceiver.noteOff.Connect(&mVoiceManager, &VoiceManager::ReleaseNote);
  double xcoefs[7] = { 4.760635e-1, 2.856281, 7.140952, 9.521270, 7.140952, 2.856281, 4.760635e-1 };
  double ycoefs[6] = { -4.522403, -8.676844, -9.007512, -5.328429, -1.702543, -2.303303e-1 };
  LP4 = new Filter(xcoefs, ycoefs, 7, 6);
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
		j = mOversampling;
		while (j--) {
			leftOutput[s] = LP4->getOutput(mVoiceManager.tick());
		}
		leftOutput[s] *= mVolume;
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
  double fs = GetSampleRate()*mOversampling;
  mVoiceManager.setFs(fs);
}

void VOSIMSynth::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  uint8_t n = NUM_VOICES;
  switch (paramIdx)
  {
    case kEnv1Atk:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setPeriod(0, GetParam(kEnv1Atk)->Value(), 0);
		break;
	case kEnv1Dec:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setPeriod(1, GetParam(kEnv1Dec)->Value(), 0);
		break;
	case kEnv1Rel:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setPeriod(2, GetParam(kEnv1Rel)->Value(), 0.5);
		break;
	case kEnv1Sus:
		while (n--)
			mVoiceManager.voices[n].mAmpEnv.setPoint(1, GetParam(kEnv1Sus)->Value());
		break;
    case kEnv2Atk:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[0].setPeriod( 0, GetParam( kEnv2Atk )->Value(), 0 );
        break;
    case kEnv2Dec:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[0].setPeriod( 1, GetParam( kEnv2Dec )->Value(), 0 );
        break;
    case kEnv2Rel:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[0].setPeriod( 2, GetParam( kEnv2Rel )->Value(), 0.5 );
        break;
    case kEnv2Sus:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[0].setPoint( 1, GetParam( kEnv2Sus )->Value() );
        break;
    case kEnv3Atk:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[1].setPeriod( 0, GetParam( kEnv3Atk )->Value(), 0 );
        break;
    case kEnv3Dec:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[1].setPeriod( 1, GetParam( kEnv3Dec )->Value(), 0 );
        break;
    case kEnv3Rel:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[1].setPeriod( 2, GetParam( kEnv3Rel )->Value(), 0.5 );
        break;
    case kEnv3Sus:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[1].setPoint( 1, GetParam( kEnv3Sus )->Value() );
        break;
    case kEnv4Atk:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[2].setPeriod( 0, GetParam( kEnv4Atk )->Value(), 0 );
        break;
    case kEnv4Dec:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[2].setPeriod( 1, GetParam( kEnv4Dec )->Value(), 0 );
        break;
    case kEnv4Rel:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[2].setPeriod( 2, GetParam( kEnv4Rel )->Value(), 0.5 );
        break;
    case kEnv4Sus:
        while ( n-- )
            mVoiceManager.voices[n].mVFEnv[2].setPoint( 1, GetParam( kEnv4Sus )->Value() );
        break;
	case kOsc1VF:
		while (n--)
			mVoiceManager.voices[n].mOsc[0].scalePFreq(GetParam(kOsc1VF)->Value());
		break;
	case kOsc1Num:
		while (n--)
			mVoiceManager.voices[n].mOsc[0].setNumber(GetParam(kOsc1Num)->Value());
		break;
	case kOsc1Dec:
		while (n--)
			mVoiceManager.voices[n].mOsc[0].setDecay(pow(10,-1*(GetParam(kOsc1Dec)->Value())));
		break;
	case kOsc1PMF:
		while (n--)
			mVoiceManager.voices[n].mLFOPitch.setFreq(pow(2, 5 * GetParam(kOsc1PMF)->Value()));
		break;
	case kOsc1PMG:
		while (n--)
			mVoiceManager.voices[n].mLFOPitch.setGain(pow(2, -12*(1-GetParam(kOsc1PMG)->Value())));
		break;
	case kOsc1Vol:
        while ( n-- )
            mVoiceManager.voices[n].mOsc[0].setGain( pow( 10.0, -1.5*(1-GetParam( kOsc1Vol )->Value()) ) );
		break;
    case kOsc2Vol:
        while ( n-- )
            mVoiceManager.voices[n].mOsc[1].setGain( pow( 10.0, -1.5 * (1-GetParam( kOsc2Vol )->Value()) ) );
        break;
    case kOsc3Vol:
        while ( n-- )
            mVoiceManager.voices[n].mOsc[2].setGain( pow( 10.0, -1.5 * (1-GetParam( kOsc3Vol )->Value()) ) );
        break;
	case kOsc2VF:
		while (n--)
			mVoiceManager.voices[n].mOsc[1].scalePFreq(GetParam(kOsc2VF)->Value());
		break;
	case kOsc2Num:
		while (n--)
			mVoiceManager.voices[n].mOsc[1].setNumber(GetParam(kOsc2Num)->Value());
		break;
	case kOsc2Dec:
		while (n--)
			mVoiceManager.voices[n].mOsc[1].setDecay(pow(10, -1 * (GetParam(kOsc2Dec)->Value())));
		break;
	case kOsc3VF:
		while (n--)
			mVoiceManager.voices[n].mOsc[2].scalePFreq(GetParam(kOsc3VF)->Value());
		break;
	case kOsc3Num:
		while (n--)
			mVoiceManager.voices[n].mOsc[2].setNumber(GetParam(kOsc3Num)->Value());
		break;
	case kOsc3Dec:
		while (n--)
			mVoiceManager.voices[n].mOsc[2].setDecay(pow(10, -1 * (GetParam(kOsc3Dec)->Value())));
		break;
    case kWedgeSwitch1:
        mOversampling = 1 + GetParam( kWedgeSwitch1 )->Value() * 5;
		Reset();
        break;
    default:
      break;
  }
}
