#pragma once
#include "ndpch.h"
#include "audio_header.h"


/**
 * Represents Audio which can be played, paused, stopped
 * It is only a handle, true audio proccessing is done on the separate thread in Sounder
 * It's possible to call Sounder directly with Sounder::submit()
 */
class AudioHandle
{
private:
	SoundID m_handle = -1;
	float m_volume = 1;
	float m_pitch = 1;
	bool m_loop = false;
	std::string m_file_path;
	bool m_is_playing = false;
	const bool m_is_sound;
	bool m_is_paused = false;
	bool m_is_spatial;
	SpatialData m_spatial_data;
public:
	AudioHandle(bool isSound, bool isSpatial = false);
	// sets the filePath of the audio clip (nothing else)
	void open(const std::string& filePath);
	// starts playing with fading-in from silence in fadeTime seconds
	// use fadeTime = 0 to get instant full volume 
	void play(float fadeTime = 0);
	// pauses currently playing clip
	// to play it again use simply play(0)
	void pause();
	// stops playing with fading-out into silence in fadeTime seconds
	// use fadeTime = 0 to stop immediately
	void stop(float fadeTime = 0);
	// sets the target volume for clip to reach in fadeTime seconds
	// can be used before playing and during playing
	void setVolume(float volume, float fadeTime = 0);
	// sets the target pitch/speed for clip to reach in fadeTime seconds
	// can be used before playing
	// can be also used during playing if it's Music and not Sound
	void setPitch(float f, float fadeTime = 0);
	// sets looping of clip (replaying when it reaches end)
	// can be set only before calling play()
	inline void setLoop(bool loop) { m_loop = loop; }
	// true if file is playing
	bool isPlaying();

	void updateSpatialData(const SpatialData& data);
};


/**
 * Represents audio that will be cached into the buffer for later (and faster) usage
 * Use with short audio clips (like 4sec max) (it's not mandatory yet advisable)
 */
class SoundHandle :public AudioHandle
{
public:
	SoundHandle();
};
/**
 * Represents audio stream which will be read from file directly into the sound proccessing
 * Won't be loaded all at once
 * Use with songs, ambient sounds and longer audio clips (or with sounds which won't be needed again in the near future)
 */
class MusicHandle :public AudioHandle
{
public:
	MusicHandle();
};

