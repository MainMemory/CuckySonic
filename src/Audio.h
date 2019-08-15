#pragma once
#include <stdint.h>

#define AUDIO_FREQUENCY 44100
#define AUDIO_SAMPLES	0x200

//Music ids
enum MUSICID
{
	MUSICID_GHZ,
	MUSICID_MAX,
};

//Sound ids
enum SOUNDID
{
	SOUNDID_NULL,
	SOUNDID_JUMP,
	SOUNDID_ROLL,
	SOUNDID_SKID,
	SOUNDID_SPINDASH_REV,
	SOUNDID_SPINDASH_RELEASE,
	SOUNDID_DEATH,
	SOUNDID_RING,
	SOUNDID_RING_LEFT,
	SOUNDID_RING_RIGHT,
	SOUNDID_SPLASHJINGLE,
	SOUNDID_MAX,
};

//Definitions
enum SOUNDCHANNEL
{
	SOUNDCHANNEL_NULL,
	SOUNDCHANNEL_PSG0,
	SOUNDCHANNEL_PSG1,
	SOUNDCHANNEL_PSG2,
	SOUNDCHANNEL_PSG3,
	SOUNDCHANNEL_FM0,
	SOUNDCHANNEL_FM1,
	SOUNDCHANNEL_FM2,
	SOUNDCHANNEL_FM3,
	SOUNDCHANNEL_FM4,
	SOUNDCHANNEL_FM5,
	SOUNDCHANNEL_DAC,
};

struct MUSICDEFINITION
{
	const char *path;
	size_t loopSample;	//Frames for the loop point, in-case the song has an intro, this can be 0
};

struct SOUNDDEFINITION
{
	SOUNDCHANNEL channel;
	const char *path;
	SOUNDID parent;
};

//Sound class
class SOUND
{
	public:
		const char *fail;
		
		//Our actual buffer data
		float *buffer;
		int size;
		
		//Playback position and frequency
		bool playing;
		
		double sample;
		double frequency;
		double baseFrequency;
		
		float volume;
		float volumeL;
		float volumeR;
		
		SOUND *next;
		SOUND *parent;
		
	public:
		SOUND(const char *path);
		SOUND(SOUND *ourParent);
		~SOUND();
		
		void Play();
		void Stop();
		
		void SetVolume(float setVolume);
		void SetPan(float setVolumeL, float setVolumeR);
		void SetFrequency(double setFrequency);
		
		void Mix(float *stream, int samples);
};

//Audio functions
void PlaySound(SOUNDID id);
void StopSound(SOUNDID id);

bool InitializeAudio();
void QuitAudio();
