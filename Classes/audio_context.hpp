//
//  audio_context.hpp
//  Koumachika
//
//  Created by Toni on 5/21/19.
//
//

#ifndef audio_context_hpp
#define audio_context_hpp

#if use_sound

class audio_context
{
public:
	friend class App;

	static bool check_error(const string& msg);

	static const vector<string> soundFiles;

	inline audio_context() {}
	~audio_context();

	void initAudio();
	ALuint getSoundSource();
	ALuint initSoundSource(const Vec3& pos, const Vec3& vel, bool relative);
	void loadSound(const string& path);
	ALuint playSound(const string& path, float volume, bool loop = false);
	ALuint playSoundSpatial(
		const string& path,
		const Vec3& pos,
		const Vec3& vel,
		float volume = 1.0f,
		bool loop = false
	);
	void endSound(ALuint source);
	void pauseSounds();
	void resumeSounds();
	void setSoundListenerPos(SpaceVect pos, SpaceVect vel, SpaceFloat angle);
	bool setSoundSourcePos(ALuint source, SpaceVect pos, SpaceVect vel, SpaceFloat angle);
	bool isSoundSourceActive(ALuint source);
protected:
	void update();

	ALCdevice* audioDevice = nullptr;
	ALCcontext* audioContext = nullptr;

	unordered_map<string, ALuint> loadedBuffers;
	unordered_set<ALuint> activeSources;
	list<ALuint> availableSources;
	mutex audioMutex;
};
#endif

#endif /* audio_context_hpp */
