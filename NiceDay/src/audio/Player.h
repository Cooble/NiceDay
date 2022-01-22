#pragma once
#include "audio.h"
#include "memory/Pool.h"
#include "audio_header.h"

namespace nd {
// enables possibility to get currently playing list of streams using Sounder::getDebugInfo()
#define SOUNDER_DEBUG_INFO 1

//how big one soundbuffer can be 
constexpr int SOUNDER_SOUND_BUFF_SAMPLE_MEMORY_SIZE = 44100 * 4 * 2; //rate * seconds * stereo

struct SoundBuff
{
	std::atomic_int usages = 0;
	uint64_t lastTimeUsage = 0;
	bool isBeingFilled = false;
	SoundBuffer buffer;
	float memory[SOUNDER_SOUND_BUFF_SAMPLE_MEMORY_SIZE / sizeof(float)];
};

struct SoundData
{
	float left_phase;
	float right_phase;

	SoundBuff* sound_buff;
	std::atomic<float> volume;
	float logVolume;
	std::atomic<float> targetVolume;
	float currentFloatSampleIndex = 0;
	//how much should volume change per sample if(targetVolume!=volume) always positive
	std::atomic<float> deltaRate = 0;
	//is already log based
	std::atomic<float> spatialMultiplier = 1;
	std::atomic<float> spatialDirection = 0;
	std::atomic<bool> isPaused;
	std::atomic_bool shouldClose = false;
	std::atomic_bool terminateOnFadeOut = false;
	bool shouldLoop = false;

	float pitch = 1;
};

struct MusicData
{
	float left_phase;
	float right_phase;


	MusicStream file_stream;
	Utils::RingBufferLite* ring_buffer;
	std::atomic<float> volume;
	std::atomic<float> pitch;
	float logVolume;
	std::atomic<float> targetVolume;
	std::atomic<float> targetPitch;
	//how much should volume change per sample if(targetVolume!=volume) always positive
	std::atomic<float> deltaVolumeRate = 10000;
	std::atomic<float> deltaPitchRate = 10000;
	//is already log based
	std::atomic<float> spatialMultiplier = 1;
	std::atomic<float> spatialDirection = 0;
	std::atomic<bool> isPaused;
	std::atomic_bool shouldClose = false;
	std::atomic_bool terminateOnFadeOut = false;
	bool shouldLoop;
	float currentFloatSampleIndex = 0;
	std::atomic_bool eof = false;
	bool has_started = false;
};

typedef void PaStream;

struct Sound
{
	const bool isSound = true;
	SoundID soundid = -1;
	PaStream* pa_stream = nullptr;
	SoundData data{};
	SpatialData spatialData;
	auto& soundBuffer() { return data.sound_buff; }
};

struct Music
{
	const bool isSound = false;
	SoundID soundid = -1;
	//if music stream has been opened
	bool isFileOpened = false;
	std::atomic<uint64_t> lastFeedTime = 0;
	PaStream* pa_stream = nullptr;
	MusicData data{};
	SoundBuff* optional_sound_buffer = nullptr;
	JobAssignmentP optional_job = nullptr;
	std::string fileName;
	Strid sid = 0;
	std::atomic_bool hasLoopRing = false;
	std::atomic_bool shouldTerminate = false;
	SpatialData spatialData;

	auto& musicStream() { return data.file_stream; }
};

inline bool operator==(const SpatialData& lhs, const SpatialData& rhs)
{
	return lhs.pos == rhs.pos && rhs.maxDistances == lhs.maxDistances;
}

struct SoundAssignment
{
	enum
	{
		LOAD,
		PLAY,
		SET_SPATIAL_DATA,
		FLUSH_SPATIAL_DATA,
		PAUSE,
		FADE,
		CLOSE,
		STOP_ALL,
	} type;

	std::string soundFile;
	bool sound_or_music;
	bool loop;
	float volume, pitch;
	SoundID id;
	float timeToChangeVolume = -1;
	float timeToChangePitch = -1;
	SpatialData spatialData;
};

struct FileOpenAssignment
{
	MusicStream* music_stream = nullptr;
	JobAssignmentP job = nullptr;
	std::string filepath;
};

class SoundFileOpener : public Worker<FileOpenAssignment>
{
	void proccessAssignments(std::vector<FileOpenAssignment>& assignments) override;
};

//number of streams (not sounds) that can be played at once
constexpr int SOUNDER_RING_BUFF_COUNT = 32;
// sound cache count
constexpr int SOUNDER_SOUND_BUFF_COUNT = 32;

constexpr int SOUNDER_RING_BUFF_SIZE = 1024 * 32;
constexpr int SOUNDER_RING_BUFF_FRAME_COUNT = 32;
typedef Utils::RingBuffer<SOUNDER_RING_BUFF_SIZE, SOUNDER_RING_BUFF_FRAME_COUNT> RingBufferSounder;

struct AudioState
{
	bool isActive;
	bool isSound;
	bool looping;
	float volume;
	float pitch;
	float spatialMultiplier;
	float spatialDistance;
	glm::vec2 spatialMaxes;

	float timestamp;
	std::string filePath;
	bool isPaused;
};

typedef std::unordered_map<SoundID, AudioState> AudioStateMap;

/**
 * Manages all the audio in the game
 * Uses 3 threads:
 *					1. main thread which takes commands from game thread and executes them
 *					2. opens filestreams
 *					3. feeds RingBuffers from filestreams
 *
 * Uses MusicStream and SoundBuffer from audio.h (which is independent on everything)
 * Manages whole sound memory management with caching sounds which are heavily used
 * Supports playing, pausing and changing pitch/speed and volume continuously over time - fading
 *
 * To play anything use Sounder::submit() or Sounder::playAudio()
 * or use AudioHandle!
 */
class Sounder
{
public:
	static const SoundID PLAYER_ID = 1;

	inline static Sounder& get()
	{
		static auto t = new Sounder;
		return *t;
	}

private:
	SoundFileOpener m_file_opener;
	Pool<RingBufferSounder> m_rings;
	Pool<SoundBuff> m_sound_buff_pool;
	std::unordered_map<Strid, SoundBuff*> m_sound_buff_map;
	std::unordered_map<SoundID, void*> m_sound_ids;

	std::vector<Sound*> m_sound_streams;
	std::vector<Music*> m_music_streams;
	std::atomic_int m_sound_streams_size;
	std::atomic_int m_music_streams_size;
	std::atomic_int m_sound_buffers_count;

	std::atomic_bool m_is_running = false;
	std::atomic_bool m_should_stop = false;
	std::mutex m_queue_mutex;
	std::queue<SoundAssignment> m_queue; // Have I found everybody a fun assignment to do today?
	std::mutex m_states_mutex;
	std::unordered_map<SoundID, bool> m_states;
	bool m_disabled_new_sounds = false;
#if SOUNDER_DEBUG_INFO == 1
	std::mutex m_debug_states_mutex;
	AudioStateMap m_debug_states;
#endif
	SoundID m_current_id = 2;
	SpatialData m_target_data = {{0, 0}, {0, 0}};

	void loopInternal();
	//will return new soundbuff
	// in case soundbuffPool is full, nullptr
	SoundBuff* allocateSoundBuffer(Strid id);
	Sounder();


	//ringfill section

	std::atomic_bool m_is_ringfill_running = false;
	std::atomic_bool m_should_ringfill_run = true;
	std::mutex m_ringfill_mutex;
	std::queue<Music*> m_ringfill_queue;
	void loopRingFill();
	void startRingFill();

	void prepareMusic(Music** musi, const SoundAssignment& command, bool buffFill, bool justLoad);
	void preparePlaySound(Sound** soun, const SoundAssignment& command);
	void recalculateSpatialData();
public:
	// prepares portaudio
	void init();
	// starts new thread and returns
	void start();
	// signalizes the sound thread to stop
	void stop();
	// returns true when sound thread stopped
	bool isRunning() const { return m_is_running; }
	// all calls to playAudio will be ignored
	void disableNewSounds(bool disable)
	{
		ND_TRACE("Sounds dissabled: {}", disable);
		this->m_disabled_new_sounds = disable;
	}

	// sets pos and range of sound source or target
	// When specifying target:
	//			Use Player::PLAYER_ID to set location target
	// Note:
	//		To apply spatial data call flushSpatialData()!
	void updateSpatialData(SoundID id, const SpatialData& data);

	// will recalculate all spatial sounds to target
	// Should be called once per tick
	void flushSpatialData();

	SoundID playAudio(const std::string& filePath, bool sound_or_music, float volume, float pitch, bool loop,
	                  float fadeTime, const SpatialData& data);
	SoundID playSound(const std::string& filePath, float volume = 1, float pitch = 1, bool loop = false,
	                  float fadeTime = 0, const SpatialData& data = SpatialData());
	SoundID playMusic(const std::string& filePath, float volume = 1, float pitch = 1, bool loop = false,
	                  float fadeTime = 0, const SpatialData& data = SpatialData());
	bool isPlaying(SoundID id);
	void stopAllMusic();
	void submit(const SoundAssignment& assignment);

	int getNumberOfSoundStreams() const { return m_sound_streams_size; }
	int getNumberOfMusicStreams() const { return m_music_streams_size; }
	int getSoundBufferCount() const { return m_sound_buffers_count; }

	bool isEnabled() { return !m_disabled_new_sounds; }

#if SOUNDER_DEBUG_INFO == 1
	AudioStateMap getDebugInfo();
#endif
};
}
