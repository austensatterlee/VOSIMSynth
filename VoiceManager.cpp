#include "VoiceManager.h"

void VoiceManager::TriggerNote(uint8_t noteNumber, uint8_t velocity) {
	uint8_t i = mNumVoices;
	while (i--) {
		if (!voices[i].isActive()) {
			voices[i].trigger(noteNumber,velocity);
			break;
		}
	}
}

void VoiceManager::ReleaseNote(uint8_t noteNumber, uint8_t velocity) {
	uint8_t i = mNumVoices;
	while (i--) {
		if (voices[i].mNote == noteNumber) {
			voices[i].release();
		}
	}
}

void VoiceManager::setFs(double fs)
{
	uint8_t i = mNumVoices;
	while (i--) {
		voices[i].setAudioFs(fs);
		voices[i].setModFs(fs/(MOD_FS_RAT+1));
	}
}

void VoiceManager::setNumVoices(uint8_t numVoices)
{
	mNumVoices = numVoices;
	voices = new Voice[numVoices];
}

double VoiceManager::tick() {
	mSampleCount++;
	static uint8_t i;
	static uint8_t j;
    static double finalOutput, voiceOutput;
    i = mNumVoices;
    finalOutput = 0;
	while (i--) {
		if (voices[i].isActive()) {
			if (!(mSampleCount & MOD_FS_RAT)) {
				voices[i].tickMod();
			}
			voices[i].tickAudio();
            finalOutput += voices[i].getOutput();
		}
		else {
			voices[i].reset();
		}
	}
    finalOutput = finalOutput * 1.0/ mNumVoices;
	return finalOutput;
}

void Voice::trigger(uint8_t noteNumber, uint8_t velocity)
{
	mNote = noteNumber;
	mVelocity = velocity*0.0078125;
	mOsc[0].setFreq(noteNumberToFrequency(mNote));
	mOsc[1].setFreq(noteNumberToFrequency(mNote));
	mOsc[2].setFreq(noteNumberToFrequency(mNote));
	mOsc[0].sync();
	mOsc[1].sync();
	mOsc[2].sync();
	mLFOPitch.sync();
	mAmpEnv.trigger();
    mVFEnv[0].trigger();
    mVFEnv[1].trigger();
    mVFEnv[2].trigger();
	tickMod();
}

void Voice::release()
{
	mAmpEnv.release();
    mVFEnv[0].release();
    mVFEnv[1].release();
    mVFEnv[2].release();
}

void Voice::reset() {
	mNote = 0;
	mVelocity = 0;
}

void Voice::setAudioFs(double fs)
{
	mOsc[0].setFs(fs);
	mOsc[1].setFs(fs);
	mOsc[2].setFs(fs);
}

void Voice::setModFs(double fs) {
	mAmpEnv.setFs(fs);
	mLFOPitch.setFs(fs);
	mVFEnv[0].setFs(fs);
	mVFEnv[1].setFs(fs);
	mVFEnv[2].setFs(fs);
}

void Voice::tickAudio()
{
	mOsc[0].tick();
	mOsc[1].tick();
	mOsc[2].tick();
}

void Voice::tickMod() {
	mAmpEnv.tick();
	mLFOPitch.tick();
    mVFEnv[0].tick();
    mVFEnv[1].tick();
    mVFEnv[2].tick();

	mOsc[0].modFreq(mLFOPitch.getOutput());
    mOsc[0].modPFreq( mVFEnv[0].getOutput() - 0.9 );
	mOsc[0].applyMods();

	mOsc[1].modFreq(mLFOPitch.getOutput());
    mOsc[1].modPFreq( mVFEnv[1].getOutput() - 0.9 );
	mOsc[1].applyMods();

	mOsc[2].modFreq(mLFOPitch.getOutput());
    mOsc[2].modPFreq( mVFEnv[2].getOutput() - 0.9 );
	mOsc[2].applyMods();
}

double Voice::getOutput() {
	return mVelocity*mAmpEnv.getOutput()*(mOsc[0].getOutput()+mOsc[1].getOutput()+mOsc[2].getOutput())*0.3333333333333333;
}

bool Voice::isActive() {
	return !mAmpEnv.mIsDone;
}
