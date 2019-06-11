//
//  audio_context.cpp
//  Koumachika
//
//  Created by Toni on 5/21/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "Resources.hpp"
#include "audio_context.hpp"
#include "util.h"

const vector<string> audio_context::soundFiles = {
	"sfx/bomb_explosion1.wav",
	"sfx/bomb_explosion2.wav",
	"sfx/bomb_fuse.wav",
	"sfx/enemy_damage.wav",
	"sfx/footstep_cave.wav",
	"sfx/footstep_grass.wav",
	"sfx/footstep_ice.wav",
	"sfx/footstep_sand.wav",
	"sfx/footstep_stone.wav",
	"sfx/graze.wav",
	"sfx/player_damage.wav",
	"sfx/player_death.wav",
	"sfx/player_power_attack.wav",
	"sfx/player_spellcard.wav",
	"sfx/powerup.wav",
	"sfx/red_fairy_explosion.wav",
	"sfx/shot.wav",
};

audio_context::~audio_context()
{
	alcMakeContextCurrent(nullptr);
	if (audioContext)
		alcDestroyContext(audioContext);
	if (audioDevice)
		alcCloseDevice(audioDevice);
}

void audio_context::initAudio()
{
	audioDevice = alcOpenDevice(nullptr);
	if (!audioDevice) {
		log("Failed to open audio device");
		return;
	}

	audioContext = alcCreateContext(audioDevice, nullptr);
	if (!audioContext) {
		log("Failed to open audio context.");
		return;
	}

	if (!alcMakeContextCurrent(audioContext)) {
		log("Failed to set current context.");
		return;
	}

	alDistanceModel(AL_EXPONENT_DISTANCE);

	for (string s : soundFiles) {
		loadSound(s);
	}
}

ALuint audio_context::initSoundSource(const Vec3& pos, const Vec3& vel, bool relative)
{
	ALuint result = 0;

	alGenSources(1, &result);
	alSource3f(result, AL_POSITION, pos.x, pos.y, pos.z);
	alSource3f(result, AL_VELOCITY, vel.x, vel.y, vel.z);
	if (relative)
		alSourcei(result, AL_SOURCE_RELATIVE, AL_TRUE);

	return result;
}

void audio_context::loadSound(const string& path)
{
	SF_INFO info;
	SNDFILE* file = nullptr;
	if (App::getFileUtils()) {
		file = App::getFileUtils()->openSoundFile(path, &info);
	}

	if (!file) {
		file = sf_open(path.c_str(), SFM_READ, &info);
	}
	ALuint bufferID;

	if (!file) {
		log("Failed to load audio file %s.", path.c_str());
		return;
	}

	if (info.channels != 1) {
		log("Sound file %s is multi-channel!", path.c_str());
		sf_close(file);
		return;
	}

	short* buf = new short[info.frames];
	sf_count_t bufPos = 0;

	while (true) {
		if (bufPos >= info.frames) break;

		sf_count_t readSize = sf_read_short(file, buf + bufPos, info.frames - bufPos);

		if (readSize > 0) bufPos += readSize;
		else break;
	}

	alGenBuffers(1, &bufferID);

	if (bufferID == AL_INVALID_VALUE) {
		log("Failed to create sound buffer.");
		return;
	}

	alBufferData(
		bufferID,
		AL_FORMAT_MONO16,
		buf,
		info.frames,
		info.samplerate
	);

	loadedBuffers.insert_or_assign(path, bufferID);
	delete[] buf;

	if (App::getFileUtils()) {
		App::getFileUtils()->closeSoundFile(path);
		App::getFileUtils()->unloadFile(path);
	}
}

ALuint audio_context::playSound(const string& path, float volume)
{
	audioMutex.lock();

	ALuint source = initSoundSource(Vec3::ZERO, Vec3::ZERO, true);
	ALuint sound = getOrDefault(loadedBuffers, path, to_uint(0));

	if (source != 0 && sound != 0) {
		alSourcei(source, AL_BUFFER, sound);
		alSourcePlay(source);
		activeSources.insert(source);
	}

	audioMutex.unlock();

	return source;
}

ALuint audio_context::playSoundSpatial(const string& path, const Vec3& pos, const Vec3& vel, float volume, bool loop)
{
	audioMutex.lock();

	ALuint bufferID = getOrDefault(loadedBuffers, path, to_uint(0));
	ALuint source = 0;

	if (bufferID == 0) {
		log("Unknown audio file %s", path.c_str());
	}
	else {
		source = initSoundSource(pos, vel, false);
	}

	if (source != 0) {
		alSourcei(source, AL_BUFFER, bufferID);
		alSourcef(source, AL_ROLLOFF_FACTOR, boost::math::float_constants::root_two);
		alSourcei(source, AL_LOOPING, loop);
		alSourcePlay(source);
		activeSources.insert(source);
	}

	audioMutex.unlock();

	return source;
}

void audio_context::endSound(ALuint source)
{
	audioMutex.lock();
	auto it = activeSources.find(source);
	if (it != activeSources.end()) {
		alSourceStop(*it);
		activeSources.erase(it);
	}
	audioMutex.unlock();
}

void audio_context::pauseSounds()
{
	audioMutex.lock();
	for (ALuint sourceID : activeSources)
	{
		ALenum state;
		alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
		if (state != AL_STOPPED && state != AL_PAUSED) {
			alSourcePause(sourceID);
		}
	}
	audioMutex.unlock();
}

void audio_context::resumeSounds()
{
	audioMutex.lock();
	for (ALuint sourceID : activeSources)
	{
		ALenum state;
		alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
		if (state == AL_PAUSED) {
			alSourcePlay(sourceID);
		}
	}
	audioMutex.unlock();
}

void audio_context::setSoundListenerPos(SpaceVect pos, SpaceVect vel, SpaceFloat angle)
{
	audioMutex.lock();
	Vec3 _pos = toVec3(pos);
	Vec3 _vel = toVec3(vel);
	array<float, 6> orientation = {
		0.0f, 0.0f, -1.0f,
		0.0f, 1.0f, 0.0f
	};

	alListenerfv(AL_POSITION, &_pos.x);
	alListenerfv(AL_VELOCITY, &_vel.x);
	alListenerfv(AL_ORIENTATION, &orientation[0]);
	audioMutex.unlock();
}

bool audio_context::setSoundSourcePos(ALuint source, SpaceVect pos, SpaceVect vel, SpaceFloat angle)
{
	bool valid = false;
	Vec3 _pos = toVec3(pos);
	Vec3 _vel = toVec3(vel);
	audioMutex.lock();

	if (activeSources.find(source) != activeSources.end()) {
		alSourcefv(source, AL_POSITION, &_pos.x);
		alSourcefv(source, AL_VELOCITY, &_vel.x);
		valid = true;
	}

	audioMutex.unlock();
	return valid;
}

bool audio_context::isSoundSourceActive(ALuint source)
{
	bool result;
	audioMutex.lock();
	result = activeSources.find(source) != activeSources.end();
	audioMutex.unlock();
	return result;
}

void audio_context::update()
{
	audioMutex.lock();

	auto it = activeSources.begin();
	while (it != activeSources.end()) {
		ALenum state;
		alGetSourcei(*it, AL_SOURCE_STATE, &state);
		if (state == AL_STOPPED) {
			alDeleteSources(1, &(*it));
			it = activeSources.erase(it);
		}
		else {
			++it;
		}
	}
	audioMutex.unlock();
}
