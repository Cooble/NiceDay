#include "ndpch.h"
#include "SoundLayer.h"
#include "portaudio.h"
#include "imgui.h"
#include "memory/RingBuffer.h"
#include "audio/player.h"
#include "imguifiledialog/ImGuiFileDialog.h"

#define SAMPLE_RATE (44100)
//#define SAMPLE_RATE (22050)
constexpr int musicBufferSize = 256 * 1000;
constexpr int SAMPLE_SIZE = SAMPLE_RATE / 10;
constexpr int FRAME_SAMPLE_SIZE = 256;

//DanielsonLanczos


typedef struct
{
	float left_phase;
	float right_phase;
	float time = 0;
	float timeSpeed = 0.0001;
	float volume = 0.05;
	bool isPlaying = false;
	int sampleIndex = 0;
	float samples[SAMPLE_SIZE * 2]; //one tenth of second
	int bufferIndex = 0;
	RingBufferLite* buffer;
	int valuesReadInCurrentFrame = 0;
	bool error = false;
	int channels = 1;
}
paTestData;

/* This routine will be called by the PortAudio engine when audio is needed.
   It may called at interrupt level on some machines so don't do anything
   that could mess up the system like calling malloc() or free().
*/
static int patestCallback(const void* inputBuffer, void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData)
{
	/* Cast data passed through stream to our structure. */
	paTestData* data = (paTestData*)userData;
	float* out = (float*)outputBuffer;

	for (unsigned int i = 0; i < framesPerBuffer; i++)
	{
		data->sampleIndex++;
		data->sampleIndex %= SAMPLE_SIZE * 2;

		data->samples[data->sampleIndex] = data->left_phase * data->volume;

		*out++ = data->left_phase * data->volume;
		*out++ = data->right_phase * data->volume;
		if (!data->isPlaying)
			continue;
		data->time += data->timeSpeed;
		if (data->time > 10000)
			data->time = 0;

		if (data->isPlaying)
		{
			data->left_phase =
				glm::sin(data->time) * data->volume +
				glm::sin(data->time * 2) * data->volume * 0.5 +
				glm::sin(data->time * 4) * data->volume * 0.25 +
				glm::sin(data->time * 8) * data->volume * 0.12;
			data->right_phase = data->left_phase;
			//data->right_phase = glm::sin(data->time*2) * data->volume*0.5;
		}
	}
	/*for (unsigned i = 0; i < framesPerBuffer; i++)
	{
		data->sampleIndex++;
		data->sampleIndex %= SAMPLE_SIZE;
		
		data->samples[data->sampleIndex] = data->left_phase * data->volume;
		
		*out++ = data->left_phase*data->volume;  
		*out++ = data->right_phase * data->volume;  
		if (!data->isPlaying)
			continue;
		//Generate simple sawtooth phaser that ranges between -1.0 and 1.0. 
		data->left_phase += 0.01f;
		// When signal reaches top, drop back down.
		if (data->left_phase >= 1.0f) data->left_phase -= 2.0f;
		// higher pitch so we can distinguish left and right.
		data->right_phase += 0.01f;
		if (data->right_phase >= 1.0f) data->right_phase -= 2.0f;
	}*/
	return 0;
}

static int ft(const float* timeBuffer, size_t timeBuffSize, float* freqBuffer, size_t freqBuffSize, size_t sampleFreq,
              float maxFreq)
{
	for (unsigned int freqIdx = 0; freqIdx < freqBuffSize; freqIdx++)
	{
		float freq = (float)freqIdx / freqBuffSize * maxFreq;

		float sumX = 0;
		float sumY = 0;
		for (unsigned int timeIdx = 0; timeIdx < timeBuffSize; timeIdx++)
		{
			float time = (float)timeIdx / sampleFreq;

			sumY += glm::sin(2 * 3.14159f * freq * time) * timeBuffer[timeIdx];
			sumX += glm::cos(2 * 3.14159f * freq * time) * timeBuffer[timeIdx];
		}
		sumX /= timeBuffSize;
		sumY /= timeBuffSize;
		float out = glm::sqrt(sumX * sumX + sumY * sumY);
		freqBuffer[freqIdx] = out;
	}
	return 0;
}

static int paMusicCallback(const void* inputBuffer, void* outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData)
{
	/* Cast data passed through stream to our structure. */
	paTestData* data = (paTestData*)userData;
	float* out = (float*)outputBuffer;

	auto currentFrame = (float*)data->buffer->read();
	if (!currentFrame)
	{
		//shit no data available
		data->error = true;
		return paComplete;
	}

	for (unsigned int i = 0; i < framesPerBuffer; i++)
	{
		data->sampleIndex++;
		data->sampleIndex %= SAMPLE_SIZE * 2;

		data->samples[data->sampleIndex] = data->left_phase * data->volume;

		*out++ = data->left_phase * data->volume;
		if (data->channels == 2)
			*out++ = data->right_phase * data->volume;
		if (!data->isPlaying)
		{
			data->left_phase = 0;
			data->right_phase = 0;
			continue;
		}

		data->left_phase = currentFrame[data->valuesReadInCurrentFrame++];
		if (data->channels == 2)
			data->right_phase = currentFrame[data->valuesReadInCurrentFrame++];

		if (data->valuesReadInCurrentFrame >= (data->buffer->getFrameSize() / sizeof(float)))
		{
			data->valuesReadInCurrentFrame = 0;
			data->buffer->pop();
			currentFrame = (float*)data->buffer->read();
			if (!currentFrame)
			{
				//shit no data available
				data->error = true;
				return paComplete;
			}
		}
	}
	return 0;
}

static int paMicrophoneCallback(const void* inputBuffer, void* outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo* timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void* userData)
{
	/* Cast data passed through stream to our structure. */
	paTestData* data = (paTestData*)userData;
	float* out = (float*)outputBuffer;


	auto inputFloatBuff = (float*)inputBuffer;

	for (unsigned int i = 0; i < framesPerBuffer; i++)
	{
		if (data->sampleIndex < 0)
			data->sampleIndex = 0;

		data->sampleIndex %= SAMPLE_SIZE * 2;

		data->samples[data->sampleIndex++] = inputFloatBuff[i] * data->volume;
	}
	return paContinue;
}


static paTestData data;

static void checkSoundError(PaError error)
{
	if (error != paNoError)
	{
		ND_ERROR("PortAudio error: {}", Pa_GetErrorText(error));
		ASSERT(false, "");
	}
}

std::vector<std::string> m_audioDevices;
char** m_audioDevicesStrings;

SoundLayer::SoundLayer()
{
}

static SoundBuffer soundBuff;
static MusicStream musicStream;
static size_t soundBuffOffset = 0;

void SoundLayer::onAttach()
{
	Sounder::get().init();
	Sounder::get().start();
	/*ND_INFO("Sound start");
	auto success = musicStream.initFromFile("D:/Dev/C++/NiceDay/nogit/tower_clock.ogg");
	ASSERT(success, "cannot load music strream");
	data.channels = musicStream.getChannels();
	data.buffer = new RingBuffer(FRAME_SAMPLE_SIZE * sizeof(float), 24 * 4);

	std::thread first([]()
	{
		while (!musicStream.readNext(data.buffer->getFrameCount()*1000,*data.buffer))
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			//ND_INFO("Feeding the beást");
		}
		musicStream.close();
		ND_INFO("and music is done:D");
	});
	first.detach();


	/*data.buffer = new RingBuffer(FRAME_SIZE * sizeof(float), 24);

	std::thread first([]()
	{
		std::ifstream myfile("C:/Users/Matej/Desktop/sounds/headerless.raw", std::ios::binary);
		if (!myfile.is_open())
		{
			ND_WARN("Cannot open file sound");
			return;
		}

		int offste = 0;
		while (!myfile.eof())
		{
			auto frame = data.buffer->write();
			if (frame)
			{
				myfile.read(frame, FRAME_SIZE * sizeof(float));
				data.buffer->push();
				frame = data.buffer->write();
			}
			else std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		myfile.close();
		ND_INFO("File was read");
	});
	first.detach();
	*/
	/*
		auto err = Pa_Initialize();
		checkSoundError(err);
	
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
		}
	
		/* Open an audio I/O stream. */
	return;
	auto err = Pa_OpenDefaultStream(&m_stream,
	                                0, /* no input channels */
	                                musicStream.getChannels(), /* stereo output */
	                                paFloat32, /* 32 bit floating point output */
	                                musicStream.getRate(),
	                                FRAME_SAMPLE_SIZE, /* frames per buffer, i.e. the number
						   of sample frames that PortAudio will
						   request from the callback. Many apps
						   may want to use
						   paFramesPerBufferUnspecified, which
						   tells PortAudio to pick the best,
						   possibly changing, buffer size.*/
	                                paMusicCallback, /* this is your callback function */
	                                &data); /*This is a pointer that will be passed to
						   your callback*/


	checkSoundError(err);


	err = Pa_StartStream(m_stream);
	checkSoundError(err);
}

static PaStream* microStream = nullptr;

void SoundLayer::onDetach()
{
	ND_PROFILE_END_SESSION();
	/*PaError err = paNoError;
	if (m_stream && Pa_IsStreamActive(m_stream))
		err = Pa_StopStream(m_stream);
	checkSoundError(err);
	if (microStream && Pa_IsStreamActive(microStream))
		err = Pa_StopStream(microStream);
	checkSoundError(err);
	err = Pa_Terminate();
	checkSoundError(err);*/
	Sounder::get().stop();
	while (Sounder::get().isRunning());
	ND_TRACE("Sounder stopped");
}

void SoundLayer::onUpdate()
{
	Sounder::get().flushSpatialData();
}

void SoundLayer::onImGuiRender()
{
	static bool sessionstarted = false;
	if (!sessionstarted)
	{
		ND_PROFILE_BEGIN_SESSION("soundSession", "sound");
		sessionstarted = true;
	}
	static bool isRecording = false;

	static AudioStateMap audioMap;
	static int currentTick=0;
	static bool open = true;
	static float vol = 0.2;
	//ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
	int delayPlay = 0;
	if (ImGui::Begin("Sound Editor", &open))
	{
		if (ImGui::SmallButton("PlaySound"))
		{
			if (delayPlay == 0)
			{
				ND_PROFILE_SCOPE("soundscope");
				Sounder::get().playSound("D:/Dev/C++/NiceDay/res/audio/sound.ogg", vol);
				delayPlay = 10;
			}
		}
		if (ImGui::SmallButton("PlayMusic"))
		{
			if (delayPlay == 0)
			{
				ND_PROFILE_SCOPE("musicspoce");
				Sounder::get().playMusic("D:/Dev/C++/NiceDay/res/audio/tower_clock.ogg", vol);
				delayPlay = 10;
			}
		}
		if (ImGui::SmallButton("StopAllMusic"))
		{
			Sounder::get().stopAllMusic();
		}

		if (ImGui::SmallButton("OpenMusic"))
		{
			ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey2", "Open Music", ".ogg\0\0", ".");
		}
		// display
		if (ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey2"))
		{
			// action if OK
			if (ImGuiFileDialog::Instance()->IsOk == true)
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
				Sounder::get().playMusic(filePathName, vol);
			}
			// close
			ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey2");
		}

		if (delayPlay > 0)
			delayPlay--;
		ImGui::SliderFloat("Volume", &vol, 0, 2);

		
			
		
		
		/*if (ImGui::SmallButton("Play"))
		{
			data.isPlaying = true;
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Pause"))
		{
			data.isPlaying = false;
		}

		static float volumeZoom = 1;
		
		ImGui::SliderFloat("Frequency", &data.timeSpeed, 0, 0.1);
		int startIndex = 0;
		float threshold = 0.05;
		float* samples = data.samples + (data.sampleIndex < SAMPLE_SIZE ? SAMPLE_SIZE : 0);
		for (int i = 0; i < SAMPLE_SIZE / 10; ++i)
		{
			if ((samples[i] < threshold && samples[i] > -threshold) && (samples[i] < samples[i + 2]))
			{
				startIndex = i;
				break;
			}
		}

		ImGui::PlotLines("LeftChannel", samples + startIndex, SAMPLE_SIZE * 0.9f, 0, 0, -1 / volumeZoom, 1 / volumeZoom,
		                 ImVec2(0, 70));
		ImGui::SliderFloat("VoluZoom", &volumeZoom, 1, 10);
		ImGui::Value("Time: ", data.time);
		ImGui::Value("TimeOfMusic: ", (int)(musicStream.getCurrentMillis()/1000));
		if (ImGui::SmallButton("Reset time"))
		{
			data.time = 0;
		}
		ImGui::Value("Error: ", data.error);

		static bool wasZero = false;
		if (data.buffer->getAvailableReads() == 0)
			wasZero = true;
		if (wasZero)
			ImGui::LabelText("jv", "was zero");
		
		ImGui::Value("AvailableFrames: ", (int)data.buffer->getAvailableReads());

		static int lastSampleIndex = 0;
		static float freqVolume = 1;
		static float freqReach = 1000;
		constexpr size_t freqSize = 256;
		static float frequencies[freqSize];
		//static float frequencies2[freqSize];
		//static float frequenciesAbsolute[freqSize/2];

		if ((lastSampleIndex >= SAMPLE_SIZE && data.sampleIndex < SAMPLE_SIZE) || (lastSampleIndex < SAMPLE_SIZE && data
			.sampleIndex >= SAMPLE_SIZE))
		{
			//ft(samples, SAMPLE_SIZE, frequencies, freqSize, SAMPLE_RATE, freqReach);
		}
		lastSampleIndex = data.sampleIndex;
		ImGui::PlotLines("FT", frequencies, freqSize, 0, 0, 0, 1 / freqVolume,
		                 ImVec2(0, 100));
		

		ImGui::SliderFloat("FreqVol", &freqVolume, 1, 50);
		ImGui::SliderFloat("FreqReach", &freqReach, 10, 5000);
		*/

		ImGui::Value("soundStreams:", Sounder::get().getNumberOfSoundStreams());
		ImGui::Value("musicStreams:", Sounder::get().getNumberOfMusicStreams());
		ImGui::Value("cached sounds:", Sounder::get().getSoundBufferCount());
		ImGui::Separator();
#if SOUNDER_DEBUG_INFO
		ImGui::TextColored({ 0,1,0,1 }, "Current Streams:");
		if (currentTick++ > 15)
		{
			currentTick = 0;
			audioMap = Sounder::get().getDebugInfo();
		}
		for (auto& audio : audioMap)
		{
			ImGui::Text(audio.second.filePath.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Value("isSound", audio.second.isSound);
				ImGui::Value("Vol", audio.second.volume);
				ImGui::Value("Pitch", audio.second.pitch);
				ImGui::Value("Loop", audio.second.looping);
				ImGui::Value("Timestamp", (int)(audio.second.timestamp/1000));
				ImGui::Value("Spatial", audio.second.spatialMultiplier);
				ImGui::EndTooltip();
			}
		}
#endif

	

		}
	ImGui::End();
}
