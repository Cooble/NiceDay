#include "ndpch.h"
#include "player.h"
#include "core/App.h"
#include "files/FUtil.h"
#include <portaudio.h>

namespace nd {
static int paSoundCallback(const void* inputBuffer, void* outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData)
{
	/* Cast data passed through stream to our structure. */
	auto data = (SoundData*)userData;
	auto out = (float*)outputBuffer;
	int channels = data->sound_buff->buffer.getChannels();
	float* samples = data->sound_buff->buffer.getData();
	size_t totalSamples = data->sound_buff->buffer.getTotalSamples();
	size_t samplesPerChannel = data->sound_buff->buffer.getSamplesPerChannel();

	float f = data->volume;
	data->logVolume = f * f * f * f;
	float leftDir = -glm::max(data->spatialDirection - 1, -1.f);
	float rightDir = glm::min(data->spatialDirection + 1, 1.f);

	PaStreamCallbackResult result = paContinue;
	int restZeros = 0;
	if (!data->isPaused && !data->shouldClose)
	{
		for (unsigned int i = 0; i < framesPerBuffer; i++)
		{
			*out++ = data->left_phase * data->logVolume * data->spatialMultiplier * leftDir;
			if (channels == 2)
				*out++ = data->right_phase * data->logVolume * data->spatialMultiplier * rightDir;

			if ((size_t)data->currentFloatSampleIndex >= samplesPerChannel)
			{
				data->currentFloatSampleIndex = 0;
				if (!data->shouldLoop)
				{
					result = paComplete;
					restZeros = channels * (framesPerBuffer - i - 1);
					break;
				}
			}
			if (data->volume != data->targetVolume)
			{
				float signum = data->targetVolume - data->volume > 0 ? 1.f : -1.f;
				data->volume = data->volume + data->deltaRate * signum;
				if (data->volume * signum >= data->targetVolume * signum)
					data->volume = 0 + data->targetVolume;
				float f = data->volume;
				data->logVolume = f * f * f * f;
			}

			size_t realOffset = (size_t)data->currentFloatSampleIndex * channels;
			data->left_phase = samples[realOffset];
			data->right_phase = samples[realOffset + channels - 1];
			data->currentFloatSampleIndex += data->pitch;
		}
	}
	else restZeros = framesPerBuffer * channels;

	for (int i = 0; i < restZeros; ++i)
		*out++ = 0;

	if (data->volume == 0 && data->terminateOnFadeOut || data->shouldClose)
	{
		data->shouldLoop = false;
		result = paComplete;
	}
	return result;
}

static int paMusicCallback(const void* inputBuffer, void* outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData)
{
	/* Cast data passed through stream to our structure. */
	auto data = (MusicData*)userData;
	auto out = (float*)outputBuffer;
	//int floatsInFrame = data->ring_buffer->getFrameSize() / sizeof(float);
	int channels = data->file_stream.getChannels();
	int samplesInFrame = data->ring_buffer->getFrameSize() / sizeof(float) / channels;
	auto currentFrame = (float*)data->ring_buffer->read();

	//float f = 0.1f + (data->volume) * 0.9;
	//if (data->volume < 0.003)
	//	f = 0;
	float f = data->volume;
	data->logVolume = f * f * f * f;
	float leftDir = -glm::max(data->spatialDirection - 1, -1.f);
	float rightDir = glm::min(data->spatialDirection + 1, 1.f);

	PaStreamCallbackResult result = paContinue;
	int restZeros = 0;
	if (!data->isPaused && !data->shouldClose && currentFrame)
	{
		for (unsigned int i = 0; i < framesPerBuffer; i++)
		{
			*out++ = data->left_phase * data->logVolume * data->spatialMultiplier * leftDir;
			if (channels == 2)
				*out++ = data->right_phase * data->logVolume * data->spatialMultiplier * rightDir;

			if ((size_t)data->currentFloatSampleIndex >= samplesInFrame)
			{
				data->currentFloatSampleIndex -= samplesInFrame;
				data->ring_buffer->pop();
				currentFrame = (float*)data->ring_buffer->read();
				if (!currentFrame)
				{
					data->eof = true;
					result = paComplete;
					restZeros = channels * (framesPerBuffer - i - 1);
					break;
				}
			}
			if (data->volume != data->targetVolume)
			{
				float signum = data->targetVolume - data->volume > 0 ? 1.f : -1.f;
				data->volume = data->volume + data->deltaVolumeRate * signum;
				if (data->volume * signum >= data->targetVolume * signum)
					data->volume = 0 + data->targetVolume;
				float f = data->volume;
				data->logVolume = f * f * f * f;
			}
			if (data->pitch != data->targetPitch)
			{
				float signum = data->targetPitch - data->pitch > 0 ? 1.f : -1.f;
				data->pitch = data->pitch + data->deltaPitchRate * signum;
				if (data->pitch * signum >= data->targetPitch * signum)
					data->pitch = 0 + data->targetPitch;
			}
			size_t realOffset = (size_t)data->currentFloatSampleIndex * channels;
			data->left_phase = currentFrame[realOffset];
			data->right_phase = currentFrame[realOffset + channels - 1];
			data->currentFloatSampleIndex += data->pitch;
		}
	}
	else restZeros = framesPerBuffer * channels;

	for (int i = 0; i < restZeros; ++i)
		*out++ = 0;

	if (data->volume == 0 && data->terminateOnFadeOut
		|| data->shouldClose
		|| !currentFrame && data->has_started) { result = paComplete; }

	data->has_started = currentFrame;
	return result;
}

Sounder::Sounder()
	:
	m_rings(SOUNDER_RING_BUFF_COUNT),
	m_sound_buff_pool(SOUNDER_SOUND_BUFF_COUNT) {}

static PaError paStartSoundAudioStream(Sound* sound)
{
	int defaultOut = Pa_GetDefaultOutputDevice();

	PaStreamParameters* inParam = nullptr;

	auto outParam = new PaStreamParameters();
	outParam->channelCount = sound->soundBuffer()->buffer.getChannels();
	outParam->device = defaultOut;
	outParam->sampleFormat = paFloat32;
	outParam->suggestedLatency = 0.1;


	auto error = Pa_OpenStream(&sound->pa_stream,
	                           inParam,
	                           outParam,
	                           sound->soundBuffer()->buffer.getRate(),
	                           paFramesPerBufferUnspecified,
	                           paNoFlag,
	                           paSoundCallback,
	                           &sound->data);
	ASSERT(error == paNoError, "caanot open pa stream");
	if (error != paNoError) { return error; }
	error = Pa_StartStream(sound->pa_stream);
	ASSERT(error == paNoError, "caanot open pa stream");
	return error;
}

static PaError paStartMusicAudioStream(Music* music)
{
	int defaultOut = Pa_GetDefaultOutputDevice();

	PaStreamParameters* inParam = nullptr;

	//todo who is responsible for deleting PaStreamParameters?
	auto outParam = new PaStreamParameters();
	outParam->channelCount = music->musicStream().getChannels();
	outParam->device = defaultOut;
	outParam->sampleFormat = paFloat32;
	outParam->suggestedLatency = 0.05;


	auto error = Pa_OpenStream(&music->pa_stream,
	                           inParam,
	                           outParam,
	                           music->data.file_stream.getRate(),
	                           paFramesPerBufferUnspecified,
	                           paNoFlag,
	                           paMusicCallback,
	                           &music->data);
	if (error != paNoError)
		return error;

	Pa_StartStream(music->pa_stream);
	if (error != paNoError)
		return error;
	return paNoError;
}

void SoundFileOpener::proccessAssignments(std::vector<FileOpenAssignment>& assignments)
{
	for (auto& assignment : assignments)
	{
		auto job = assignment.job;
		job->m_variable = assignment.music_stream->initFromFile(assignment.filepath.c_str())
			                  ? JobAssignment::JOB_SUCCESS
			                  : JobAssignment::JOB_FAILURE;
		job->markDone();
	}
}

void Sounder::loopRingFill()
{
	std::vector<Music*> m_ringfill_buffer;
	m_should_ringfill_run = true;
	m_is_ringfill_running = true;
	while (m_should_ringfill_run)
	{
		//feed the ringbuffers with file data and remove music streams if done
		for (int i = m_ringfill_buffer.size() - 1; i >= 0; --i)
		{
			auto music = m_ringfill_buffer[i];

			if (music->shouldTerminate)
			{
				music->hasLoopRing = false;
				m_ringfill_buffer.erase(m_ringfill_buffer.begin() + i);
				continue;
			}


			//fill the ringbuffer with sweet data and possibly fill soundbuff as well
			if (music->data.file_stream.readNext(
				music->data.ring_buffer->getFrameCount(),
				*music->data.ring_buffer,
				music->optional_sound_buffer != nullptr
					? &music->optional_sound_buffer->buffer
					: nullptr))
			{
				//we are done, lets detach the soundbuffer
				if (music->optional_sound_buffer)
				{
					--music->optional_sound_buffer->usages;
					music->optional_sound_buffer->isBeingFilled = false;
				}
				music->optional_sound_buffer = nullptr;
				music->hasLoopRing = false;
				m_ringfill_buffer.erase(m_ringfill_buffer.begin() + i);
			}
			else music->lastFeedTime = TimerStaper::getNowMicros();
		}

		{
			std::lock_guard<std::mutex> guard(m_ringfill_mutex);
			while (!m_ringfill_queue.empty())
			{
				m_ringfill_buffer.push_back(m_ringfill_queue.front());
				m_ringfill_queue.pop();
			}
		}
	}
	m_is_ringfill_running = false;
}

void Sounder::startRingFill()
{
	auto t = std::thread(&Sounder::loopRingFill, this);
	t.detach();
}

void Sounder::prepareMusic(Music** musi, const SoundAssignment& command, bool buffFill, bool justLoad)
{
	//try to obtain ringbuffer
	//forgive me but I know what I am doing (I hope)
	auto ring = reinterpret_cast<Utils::RingBufferLite*>(m_rings.allocate());
	if (!ring)
	{
		//we don't have enough ringbuffers
		*musi = nullptr;
		{
			//remove entry in states
			std::lock_guard<std::mutex> guard(m_states_mutex);
			m_states.erase(m_states.find(command.id));
		}
		return;
	}

	*musi = new Music;
	auto music = *musi;
	music->data.ring_buffer = ring;
	music->optional_sound_buffer = buffFill ? (SoundBuff*)1 : nullptr;
	music->fileName = command.soundFile;
	music->sid = SID(music->fileName);
	JobAssignmentP job = APsched().allocateJob();
	job->assign();
	music->optional_job = job;
	music->data.volume = 0;
	music->data.targetVolume = justLoad ? 0 : command.volume;
	music->data.shouldLoop = !justLoad && command.loop;
	music->data.pitch = justLoad ? 2 : command.pitch; //loading twice as fast
	music->data.targetPitch = justLoad ? 2 : command.pitch;
	music->isFileOpened = false;
	music->data.deltaVolumeRate = command.timeToChangeVolume; //will be converted after file load
	music->spatialData = command.spatialData;
	m_file_opener.assignWork({&music->musicStream(), job, command.soundFile});
	m_music_streams.push_back(music);
	m_sound_ids[command.id] = music;
	music->soundid = command.id;
	{
		std::lock_guard<std::mutex> guard(m_states_mutex);
		m_states[command.id] = true;
	}
#if SOUNDER_DEBUG_INFO
	AudioState state;
	state.volume = command.volume;
	state.pitch = command.pitch;
	state.looping = command.loop;
	state.filePath = command.soundFile;
	state.isSound = false;
	{
		std::lock_guard<std::mutex> guard(m_debug_states_mutex);
		m_debug_states[command.id] = state;
	}
#endif
}

void Sounder::preparePlaySound(Sound** soun, const SoundAssignment& command)
{
	SoundBuff* buff = m_sound_buff_map[SID(command.soundFile)];
	++buff->usages;
	buff->lastTimeUsage = TimerStaper::getNowMicros();


	auto sound = new Sound;
	*soun = sound;
	m_sound_streams.push_back(sound);
	sound->data.sound_buff = buff;
	if (command.timeToChangeVolume)
		sound->data.deltaRate = 1.f / command.timeToChangeVolume / sound
		                                                           ->data.sound_buff->buffer.
		                                                           getRate();
	else sound->data.deltaRate = 10000;
	sound->data.volume = 0;
	sound->data.targetVolume = command.volume;
	sound->data.pitch = command.pitch;
	sound->data.shouldLoop = command.loop;
	sound->spatialData = command.spatialData;

	auto error = paStartSoundAudioStream(sound);
	ASSERT(error == paNoError, "cannot open audio stream: {}", command.soundFile);

	m_sound_ids[command.id] = sound;
	sound->soundid = command.id;
	{
		std::lock_guard<std::mutex> guard(m_states_mutex);
		m_states[command.id] = true;
	}
#if SOUNDER_DEBUG_INFO
	AudioState state;
	state.volume = command.volume;
	state.pitch = command.pitch;
	state.looping = command.loop;
	state.filePath = command.soundFile;
	state.isSound = true;
	{
		std::lock_guard<std::mutex> guard(m_debug_states_mutex);
		m_debug_states[command.id] = state;
	}
#endif
}

static void getVolume(const SpatialData& target, const SpatialData& source, float& outVolume, float& outDirection)
{
	float dist = glm::distance(target.pos, source.pos);
	float minDist = glm::max(dist, source.maxDistances.x);
	outVolume = glm::max(0.f, 1 - (minDist - source.maxDistances.x) / (source.maxDistances.y - source.maxDistances.x));

	outDirection = source.pos.x - target.pos.x;
	if (outDirection == 0)
		return;
	///prizpusobit radiusnormal kdyz se bude delit glmabs(outdir)
	outDirection = glm::clamp(
			(glm::abs(outDirection) - source.maxDistances.x) / (source.maxDistances.y - source.maxDistances.x), 0.f, 1.f)
		* (glm::abs(outDirection) / outDirection);
}

void Sounder::recalculateSpatialData()
{
	for (auto& sound : m_music_streams)
	{
		if (sound->spatialData.isValid())
		{
			float vol;
			float dir;
			getVolume(m_target_data, sound->spatialData, vol, dir);
			sound->data.spatialMultiplier = vol * vol * vol * vol;
			sound->data.spatialDirection = dir;
		}
	}
	for (auto& sound : m_sound_streams)
	{
		if (sound->spatialData.isValid())
		{
			float vol;
			float dir;
			getVolume(m_target_data, sound->spatialData, vol, dir);
			sound->data.spatialDirection = 1;
		}
	}
}

void Sounder::loopInternal()
{
	m_is_running = true;

	m_file_opener.start();
	startRingFill();

	std::queue<SoundAssignment> commands;
	auto lastCheckCommandTime = std::chrono::high_resolution_clock::now();
#if SOUNDER_DEBUG_INFO
	uint64_t lastUpdate = TimerStaper::getNowMicros();
#endif
	while (!m_should_stop)
	{
		m_music_streams_size = m_music_streams.size();
		m_sound_streams_size = m_sound_streams.size();
		m_sound_buffers_count = m_sound_buff_pool.getCurrentSize();
		auto nowTime = std::chrono::high_resolution_clock::now();
		//check every one tick of app
		//get commands
		if (commands.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - lastCheckCommandTime).
			count() > 1000 / App::get().getTPS())
		{
			//60 tps
			{
				lastCheckCommandTime = nowTime;
				std::lock_guard<std::mutex> guard(m_queue_mutex);

				while (!m_queue.empty())
				{
					commands.push(m_queue.front());
					m_queue.pop();
				}
			}
		}
		// proccess commands
		while (!commands.empty())
		{
			long long totalTime = 0;
			TimerStaper staper("comman");

			//take only one command per update
			auto command = commands.front();
			commands.pop();
			switch (command.type)
			{
			case SoundAssignment::STOP_ALL:
				{
					for (auto music_stream : m_music_streams)
					{
						if (music_stream->optional_sound_buffer)
						{
							music_stream->data.targetVolume = 0;
							music_stream->data.deltaVolumeRate = 10000;
						}
						else
							music_stream->data.shouldClose = true;
						music_stream->data.shouldLoop = false;
					}
					for (auto sound_stream : m_sound_streams) { sound_stream->data.shouldClose = true; }
				}
				break;
				//just load soundbuffer if doesn't exist already
			case SoundAssignment::LOAD:
				{
					auto sid = SID(command.soundFile);
					auto it = m_sound_buff_map.find(sid);
					if (it == m_sound_buff_map.end())
					{
						auto music = new Music;
						prepareMusic(&music, command, true, true);
					}
				}
				break;
				//play sound or music or resume if paused
			case SoundAssignment::PLAY:
				{
					//check if was only paused to resume
					SoundID id = command.id;
					auto it = m_sound_ids.find(id);
					if (it != m_sound_ids.end())
					{
						//forgive me! Devil shall call my name again
						auto sound = (Sound*)it->second;
						auto music = (Music*)it->second;
						if (sound->isSound)
						{
							if (sound->data.isPaused)
							{
								sound->data.isPaused = false;
								break;
							}
						}
						else if (music->data.isPaused)
						{
							music->data.isPaused = false;
							break;
						}
					}

					if (command.sound_or_music)
					{
						auto sid = SID(command.soundFile);
						auto it = m_sound_buff_map.find(sid);
						if (it == m_sound_buff_map.end())
						{
							//not sound buffer ready? create a music stream instead
							Music* music;
							prepareMusic(&music, command, true, false);
						}
						else
						{
							//we have already buffer ready
							Sound* sound;
							preparePlaySound(&sound, command);
						}
					}
					else
					{
						//play music -> no sound buffer will be stored for later usage
						Music* music;
						prepareMusic(&music, command, false, false);
					}
				}
				break;
			case SoundAssignment::PAUSE:
				{
					SoundID id = command.id;
					auto it = m_sound_ids.find(id);
					if (it == m_sound_ids.end())
						break;
					//forgive me! Devil shall call my name again
					auto sound = (Sound*)it->second;
					auto music = (Music*)it->second;
					if (sound->isSound)
						sound->data.isPaused = true;
					else
						music->data.isPaused = true;
				}
				break;
			case SoundAssignment::FADE:
				{
					SoundID id = command.id;
					auto it = m_sound_ids.find(id);
					if (it == m_sound_ids.end())
						break;
					//forgive me! Devil shall call my name again
					auto sound = (Sound*)it->second;
					auto music = (Music*)it->second;
					if (sound->isSound)
					{
						if (command.timeToChangeVolume >= 0)
						{
							if (command.timeToChangeVolume)
								sound->data.deltaRate = 1.f / command.timeToChangeVolume / sound
									->soundBuffer()->buffer.
									getRate();
							else sound->data.deltaRate = 10000;
							sound->data.targetVolume = command.volume;
						}
						sound->data.pitch = command.pitch;
					}
					else
					{
						if (command.timeToChangeVolume >= 0)
						{
							if (command.timeToChangeVolume)
								music->data.deltaVolumeRate = 1.f / command.timeToChangeVolume / music
									->musicStream().
									getRate();
							else music->data.deltaVolumeRate = 10000;
							music->data.targetVolume = command.volume;
						}
						if (command.timeToChangePitch >= 0)
						{
							if (command.timeToChangePitch)
								music->data.deltaPitchRate = 1.f / command.timeToChangePitch / music
									->musicStream().
									getRate();
							else music->data.deltaPitchRate = 10000;
							music->data.targetPitch = command.pitch;
						}
					}
				}
				break;
				//kill stream
			case SoundAssignment::CLOSE:
				{
					SoundID id = command.id;
					auto it = m_sound_ids.find(id);
					if (it == m_sound_ids.end())
						break;
					//forgive me! Devil shall call my name again
					auto sound = (Sound*)it->second;
					auto music = (Music*)it->second;
					if (sound->isSound)
					{
						if (command.timeToChangeVolume > 0)
							sound->data.deltaRate = 1.f / command.timeToChangeVolume / sound
								->soundBuffer()->buffer.
								getRate();
						else sound->data.deltaRate = 10000;

						sound->data.targetVolume = 0;
						sound->data.terminateOnFadeOut = true;
					}
					else
					{
						music->data.shouldLoop = false;

						if (command.timeToChangeVolume > 0)
							music->data.deltaVolumeRate = 1.f / command.timeToChangeVolume / music
								->musicStream().getRate();
						else sound->data.deltaRate = 10000;
						music->data.targetVolume = 0;
						music->data.terminateOnFadeOut = !music->optional_sound_buffer;
					}
				}
				break;
			case SoundAssignment::SET_SPATIAL_DATA:
				{
					SoundID id = command.id;
					if (id == PLAYER_ID)
					{
						m_target_data = command.spatialData;
						break;
					}
					auto it = m_sound_ids.find(id);
					if (it == m_sound_ids.end())
						break;
					//forgive me! Devil shall call my name again
					auto sound = (Sound*)it->second;
					auto music = (Music*)it->second;
					if (sound->isSound)
						sound->spatialData = command.spatialData;
					else
						music->spatialData = command.spatialData;
				}
				break;
			case SoundAssignment::FLUSH_SPATIAL_DATA: { recalculateSpatialData(); }
				break;
			default: ;
			}

			totalTime = staper.getUS();
			if (totalTime > 10000)
			{
				//ND_WARN("too long total {} ms", totalTime / 1000.f);
				//ND_WARN("init {} ms", initTime);
				//ND_WARN("startStreamTime {} ms", startStreamTime);
			}
		}
		{
			//check if file open is done and play the stream
			for (int i = m_music_streams.size() - 1; i >= 0; --i)
			{
				auto& music = m_music_streams[i];
				if (music->isFileOpened) //ignore already opened streams
					continue;
				auto job = music->optional_job;
				if (!job->isDone())
					continue;
				if (job->isSuccess())
				{
					//compute deltaRate once we know the stream rate
					if (music->data.deltaVolumeRate)
						music->data.deltaVolumeRate = 1.f / music->data.deltaVolumeRate / music
							->musicStream().getRate();
					else music->data.deltaVolumeRate = 10000;


					music->isFileOpened = true;
					// sound buffer to be filled if marked as such
					if (music->optional_sound_buffer == (SoundBuff*)1)
					{
						auto buff = allocateSoundBuffer(music->sid);
						if (buff != nullptr)
						{
							music->optional_sound_buffer = buff;
							m_sound_buff_map[music->sid] = buff;
							buff->usages++;
							buff->lastTimeUsage = TimerStaper::getNowMicros();

							buff->isBeingFilled = true;
							auto& fStream = music->data.file_stream;

							float* memory = buff->memory;
							//if the default sound buffer is too small we will leave allocation to SoundBuffer instead
							if (fStream.getTotalSamples() * fStream.getChannels() * sizeof(float) >
								SOUNDER_SOUND_BUFF_SAMPLE_MEMORY_SIZE)
								memory = nullptr;
							buff->buffer.allocate(music->data.file_stream.getTotalSamples(),
							                      music->data.file_stream.getVorbisInfo(), memory);
						}
						else music->optional_sound_buffer = nullptr; // no memory left for cache
					}
					auto error = paStartMusicAudioStream(music);
					ASSERT(error == paNoError, "cannot open audio stream, {}", music->fileName);

					//insert music stream into ring filler
					std::lock_guard<std::mutex> guard(m_ringfill_mutex);
					music->hasLoopRing = true;
					m_ringfill_queue.push(music);
				}
				else
				{
					{
						std::lock_guard<std::mutex> guard(m_states_mutex);
						m_states.erase(m_states.find(music->soundid));
					}
#if SOUNDER_DEBUG_INFO

					{
						std::lock_guard<std::mutex> guard(m_debug_states_mutex);
						m_debug_states.erase(m_debug_states.find(music->soundid));
					}
#endif


					m_music_streams.erase(m_music_streams.begin() + i);
					//ND_WARN("cannot play music {}", music->fileName);
					m_rings.deallocate(reinterpret_cast<RingBufferSounder*>(music->data.ring_buffer));
					delete music;
				}
				APsched().deallocateJob(job);
			}
			//remove music streams if done
			for (int i = m_music_streams.size() - 1; i >= 0; --i)
			{
				auto music = m_music_streams[i];
				if (!music->isFileOpened) //ignore not opened streams
					continue;
				if (!Pa_IsStreamActive(music->pa_stream))
				{
					//wait for ringfill to remove the music before deleting it
					if (music->hasLoopRing)
					{
						music->shouldTerminate = true;
						continue;
					}
					Pa_CloseStream(music->pa_stream);
					music->pa_stream = nullptr;
					if (music->data.ring_buffer->getAvailableReads() != 0 && music->data.eof)
					{
						ND_WARN(
							"Music stream could not read further, closed itself. Has waited for {}ms wth {} available reads",
							(TimerStaper::getNowMicros() - music->lastFeedTime) / 1000.f,
							music->data.ring_buffer->getAvailableReads());
					}
					if (music->data.shouldLoop)
					{
						SoundAssignment s;
						s.type = SoundAssignment::PLAY;
						s.id = music->soundid;
						s.soundFile = music->fileName;
						s.loop = true;
						s.pitch = music->data.pitch;
						s.volume = music->data.volume;
						s.timeToChangePitch = 0;
						s.timeToChangeVolume = 0;
						s.sound_or_music = m_sound_buff_map.find(music->sid) != m_sound_buff_map.end();
						{
							std::lock_guard<std::mutex> guard(m_queue_mutex);
							m_queue.push(s);
						}
					}
					else
					{
						{
							std::lock_guard<std::mutex> guard(m_states_mutex);
							m_states.erase(m_states.find(music->soundid));
						}
#if SOUNDER_DEBUG_INFO

						{
							std::lock_guard<std::mutex> guard(m_debug_states_mutex);
							m_debug_states.erase(m_debug_states.find(music->soundid));
						}
#endif
					}

					m_music_streams.erase(m_music_streams.begin() + i);
					m_sound_ids.erase(music->soundid);
					m_rings.deallocate(reinterpret_cast<RingBufferSounder*>(music->data.ring_buffer));
					delete music;
				}
			}
			// removes sound streams if done
			for (int i = m_sound_streams.size() - 1; i >= 0; --i)
			{
				auto sound = m_sound_streams[i];
				if (!Pa_IsStreamActive(sound->pa_stream))
				{
					Pa_CloseStream(sound->pa_stream);
					sound->pa_stream = nullptr;
					m_sound_streams.erase(m_sound_streams.begin() + i);
					sound->data.sound_buff->usages--;
					sound->data.sound_buff->lastTimeUsage = TimerStaper::getNowMicros();
					m_sound_ids.erase(sound->soundid);
					{
						std::lock_guard<std::mutex> guard(m_states_mutex);
						m_states.erase(m_states.find(sound->soundid));
					}
#if SOUNDER_DEBUG_INFO
					{
						std::lock_guard<std::mutex> guard(m_debug_states_mutex);
						m_debug_states.erase(m_debug_states.find(sound->soundid));
					}
#endif
					delete sound;
				}
			}
		}
#if SOUNDER_DEBUG_INFO
		auto now = TimerStaper::getNowMicros();
		if (now - lastUpdate > 1000 * (1000.f / 60) * 15)
		{
			lastUpdate = now;
			{
				std::lock_guard<std::mutex> guarde(m_debug_states_mutex);
				for (auto& sound_id : m_debug_states)
				{
					AudioState& state = sound_id.second;
					auto it = m_sound_ids.find(sound_id.first);
					if (it == m_sound_ids.end())
						continue;
					void* p = it->second;
					if (p == nullptr)
						continue;
					auto sound = (Sound*)p;
					auto music = (Music*)p;
					if (sound->isSound)
					{
						state.volume = sound->data.volume;
						state.pitch = sound->data.pitch;
						state.isPaused = sound->data.isPaused;
						state.looping = sound->data.shouldLoop;
						state.spatialMultiplier = glm::sqrt(glm::sqrt(sound->data.spatialMultiplier));
						//needs to revert back from multiplication
						//this is solely for debug purposes
					}
					else
					{
						state.volume = music->data.volume;
						state.pitch = music->data.pitch;
						state.isPaused = music->data.isPaused;
						state.looping = music->data.shouldLoop;
						state.timestamp = music->musicStream().getCurrentMillis();
						state.spatialMultiplier = glm::sqrt(glm::sqrt(music->data.spatialMultiplier));
					}
				}
			}
		}

#endif
	}
	Pa_Terminate();

	m_should_ringfill_run = false;
	m_file_opener.stop();
	while (!m_file_opener.isStopped());
	while (m_is_ringfill_running);

	m_is_running = false;
}

SoundBuff* Sounder::allocateSoundBuffer(Strid id)
{
	if (m_sound_buff_pool.getFreeSize() > 0) { return m_sound_buff_pool.allocate(); }
	//pick the buffer which nobody uses and which lastUsage was long time ago
	float now = TimerStaper::getNowMicros();
	float lastUsed = 0;
	for (auto& sound_buff : m_sound_buff_map)
	{
		if (sound_buff.second->usages == 0)
			lastUsed = glm::max(now - sound_buff.second->lastTimeUsage, lastUsed);
	}
	for (auto& sound_buff : m_sound_buff_map)
	{
		if (sound_buff.second->usages == 0)
			if (now - sound_buff.second->lastTimeUsage == lastUsed)
			{
				m_sound_buff_pool.deallocate(sound_buff.second);
				m_sound_buff_map.erase(m_sound_buff_map.find(sound_buff.first));
				return m_sound_buff_pool.allocate();
			}
	}
	return nullptr;
}

void Sounder::init()
{
	auto err = Pa_Initialize();
	if (err == 0) { ND_TRACE("Sounder Initialized"); }
	else { ND_ERROR("Cannot initialize sounder!"); }
	/*checkSoundError(err);

	int maxDevices = Pa_GetDeviceCount();
	int stringSize = 0;
	for (int i = 0; i < maxDevices; ++i)
	{
		auto info = Pa_GetDeviceInfo(i);
		m_audioDevices.insert(m_audioDevices.begin() + i, info->name);
		stringSize += strlen(info->name) + 1;
	}
	char* bu = new char[stringSize];
	m_audioDevicesStrings = new char* [maxDevices];
	int buffidx = 0;
	for (int i = 0; i < maxDevices; ++i)
	{
		m_audioDevicesStrings[i] = bu + buffidx;
		auto info = Pa_GetDeviceInfo(i);
		memcpy(bu + buffidx, info->name, strlen(info->name) + 1);
		buffidx += strlen(info->name) + 1;
	}*/
}

void Sounder::start()
{
	auto t = std::thread(&Sounder::loopInternal, this);
	t.detach();
}

void Sounder::stop() { m_should_stop = true; }

void Sounder::updateSpatialData(SoundID id, const SpatialData& data)
{
	SoundAssignment as;
	as.type = SoundAssignment::SET_SPATIAL_DATA;
	as.id = id;
	as.spatialData = data;
	submit(as);
}

void Sounder::flushSpatialData()
{
	SoundAssignment as;
	as.type = SoundAssignment::FLUSH_SPATIAL_DATA;
	submit(as);
}

SoundID Sounder::playAudio(const std::string& filePath, bool sound_or_music, float volume, float pitch, bool loop,
                           float fadeTime, const SpatialData& data)
{
	if (this->m_disabled_new_sounds)
		return INVALID_SOUND_ID;
	
	FUTIL_ASSERT_EXIST(filePath);

	auto out = m_current_id++;
	submit({
		SoundAssignment::PLAY,
		ND_RESLOC(filePath),
		sound_or_music,
		loop,
		volume,
		pitch,
		out,
		fadeTime,
		-1,
		data
	});
	{
		ND_PROFILE_SCOPE("Waitin for mutex");
		std::lock_guard<std::mutex> guarde(m_states_mutex);
		m_states[out] = true;
	}
	return out;
}

SoundID Sounder::playSound(const std::string& filePath, float volume, float pitch, bool loop, float fadeTime,
                           const SpatialData& data)
{
	return playAudio(filePath, true, volume, pitch, loop, fadeTime, data);
}

SoundID Sounder::playMusic(const std::string& filePath, float volume, float pitch, bool loop, float fadeTime,
                           const SpatialData& data)
{
	return playAudio(filePath, false, volume, pitch, loop, fadeTime, data);
}

bool Sounder::isPlaying(SoundID id)
{
	std::lock_guard<std::mutex> guard(m_states_mutex);
	return m_states.find(id) != m_states.end();
}

void Sounder::stopAllMusic()
{
	SoundAssignment s;
	s.type = SoundAssignment::STOP_ALL;
	submit(s);
}

void Sounder::submit(const SoundAssignment& assignment)
{
	std::lock_guard<std::mutex> guard(m_queue_mutex);
	m_queue.push(assignment);
}

#if SOUNDER_DEBUG_INFO == 1
AudioStateMap Sounder::getDebugInfo()
{
	std::lock_guard<std::mutex> guard(m_debug_states_mutex);
	return m_debug_states;
}

#endif
}
