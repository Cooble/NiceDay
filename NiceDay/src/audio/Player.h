#pragma once
#include "audio.h"
#include "memory/Pool.h"
// enables possibility to get currently playing list of streams using Sounder::getDebugInfo()
#define SOUNDER_DEBUG_INFO 1
// unique identifier of each audio stream of Sounder
typedef int64_t SoundID;

//how big one soundbuffer can be 
constexpr int SOUNDER_SOUND_BUFF_SAMPLE_MEMORY_SIZE = 44100 * 4 * 2;//rate * seconds * stereo

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
	float currentFloatSampleIndex=0;
	//how much should volume change per sample if(targetVolume!=volume) always positive
	std::atomic<float> deltaRate = 0;
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
	RingBufferLite* ring_buffer;
	std::atomic<float> volume;
	std::atomic<float> pitch;
	float logVolume;
	std::atomic<float> targetVolume;
	std::atomic<float> targetPitch;
	//how much should volume change per sample if(targetVolume!=volume) always positive
	std::atomic<float> deltaVolumeRate = 10000;
	std::atomic<float> deltaPitchRate = 10000;
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
	std::atomic_bool hasLoopRing=false;
	std::atomic_bool shouldTerminate=false;
	
	auto& musicStream() { return data.file_stream; }
};
struct SoundAssignment
{
	enum
	{
		LOAD,
		PLAY,
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

};

struct FileOpenAssignment
{
	MusicStream* music_stream = nullptr;
	JobAssignmentP job = nullptr;
	std::string filepath;
};
class SoundFileOpener :public Worker<FileOpenAssignment>
{
	void proccessAssignments(std::vector<FileOpenAssignment>& assignments) override;
};

//number of streams (not sounds) that can be played at once
constexpr int SOUNDER_RING_BUFF_COUNT = 32;
// sound cache count
constexpr int SOUNDER_SOUND_BUFF_COUNT = 32;

constexpr int SOUNDER_RING_BUFF_SIZE = 1024 * 32;
constexpr int SOUNDER_RING_BUFF_FRAME_COUNT = 32;
typedef RingBuffer<SOUNDER_RING_BUFF_SIZE, SOUNDER_RING_BUFF_FRAME_COUNT> RingBufferSounder;

struct AudioState
{
	bool isActive;
	bool isSound;
	bool looping;
	float volume;
	float pitch;
	
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
#if SOUNDER_DEBUG_INFO == 1
	std::mutex m_debug_states_mutex;
	AudioStateMap m_debug_states;
#endif
	SoundID m_current_id;
	
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

	void prepareMusic(Music** musi, const SoundAssignment& command,bool buffFill,bool justLoad);
	void preparePlaySound(Sound** soun, const SoundAssignment& command);
public:

	// prepares portaudio
	void init();
	// starts new thread and returns
	void start();
	// signalizes the sound thread to stop
	void stop();
	// returns true when sound thread stopped
	inline bool isRunning() const { return m_is_running; }
	
	SoundID playAudio(const std::string& filePath, bool sound_or_music, float volume, float pitch, bool loop, float fadeTime);
	SoundID playSound(const std::string& filePath, float volume = 1, float pitch = 1, bool loop = false, float fadeTime = 0);
	SoundID playMusic(const std::string& filePath, float volume = 1, float pitch = 1, bool loop = false, float fadeTime = 0);
	inline bool isPlaying(SoundID id);
	void stopAllMusic();
	void submit(const SoundAssignment& assignment);

	int getNumberOfSoundStreams()const { return m_sound_streams_size; }
	int getNumberOfMusicStreams()const { return m_music_streams_size; }
	int getSoundBufferCount()const { return m_sound_buffers_count; }

#if SOUNDER_DEBUG_INFO == 1
	AudioStateMap getDebugInfo();
#endif
};


/**
 * Represents Audio which is can be played, paused, stopped
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
public:
	AudioHandle(bool isSound);
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


