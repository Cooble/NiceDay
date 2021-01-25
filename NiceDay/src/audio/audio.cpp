#include "ndpch.h"
#include "audio.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "core/App.h"


class ResizableBuffer
{
	float* buff;
	size_t size;
	size_t offset;

public:
	ResizableBuffer(size_t defaultSamples)
	{
		buff = (float*)malloc(defaultSamples * sizeof(float));
		size = defaultSamples;
		offset = 0;
	}

	~ResizableBuffer()
	{
		if (buff)
			free(buff);
	}

	void clearBuffer()
	{
		offset = 0;
	}

	void appendData(const float* data, size_t sampleNumber)
	{
		if (offset + sampleNumber > size)
		{
			auto newSize = glm::max(size * 2, offset + sampleNumber);
			auto newBuff = (float*)malloc(newSize * sizeof(float));
			memcpy(newBuff, buff, size * sizeof(float));
			memcpy(newBuff + size, data, sampleNumber * sizeof(float));
			free(buff);
			buff = newBuff;
			size = newSize;
			offset += sampleNumber;
			ND_WARN("Sound file is extremely big, consider using MusicStream instead.");
		}
		else
		{
			memcpy(buff + offset, data, sampleNumber * sizeof(float));
			offset += sampleNumber;
		}
	}

	size_t getSampleNumber() const { return offset; }
	size_t getMaxSize() const { return size; }
	float* getData() { return buff; }
};


constexpr int OGG_ERROR = 1;
constexpr int OGG_NO_ERROR = 0;
//max bytes to read from file at once
constexpr int OGG_FILE_BUFF_SIZE = 4096;

//constexpr size_t LOAD_SOUND_BUFFER_DEFAULT_SIZE = 5000000;
//static ResizableBuffer RESIZABLE_L(LOAD_SOUND_BUFFER_DEFAULT_SIZE);
//static ResizableBuffer RESIZABLE_R(LOAD_SOUND_BUFFER_DEFAULT_SIZE);

//target needs to be twice as big as srcBuff (size in floats)
static void mergeBuffers(float* target, const float* src0, const float* src1, size_t srcBuffSize)
{
	size_t i = 0, j = 0, k = 0;
	while (i < srcBuffSize * 2)
	{
		target[i++] = src0[j++];
		target[i++] = src1[k++];
	}
}


static int loadVorbisFileVorbisFile(const char* filePath, vorbis_info& vorbisInfo, vorbis_comment& vorbisComment,
	float*& sampleBuffer,
	size_t& pcmTotal)
{
	OggVorbis_File vf;
	int eof = 0;
	int current_section;
	int64_t sampleSize = 0;

	FILE* fileIn;
	fileIn = fopen(filePath, "rb");
	if (fileIn == nullptr)
		return OGG_ERROR;


	if (ov_open_callbacks(fileIn, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
	{
		ND_WARN("Cannot open sound stream {}", filePath);
		//	fclose(fileIn);
		return OGG_ERROR;
	}

	vorbisComment = *ov_comment(&vf, -1);
	vorbisInfo = *ov_info(&vf, -1);
	pcmTotal = ov_pcm_total(&vf, -1);

	sampleBuffer = (float*)malloc(pcmTotal * sizeof(float) * vorbisInfo.channels);
	size_t outBuffIndex = 0;

	while (!eof)
	{
		float** data;
		long samplesRet = ov_read_float(&vf, &data, 4096, &current_section);
		if (samplesRet == 0)
			eof = 1;
		else if (samplesRet > 0)
		{
			if (samplesRet)
			{
				if (vorbisInfo.channels == 1)
					memcpy(sampleBuffer + outBuffIndex, data[0], samplesRet * sizeof(float));
				else if (vorbisInfo.channels == 2)
					mergeBuffers(sampleBuffer + outBuffIndex, data[0], data[1], samplesRet);
				outBuffIndex += (size_t)samplesRet * vorbisInfo.channels;
			}
		}
	}
	ov_clear(&vf);
	//fclose(fileIn);
	return OGG_NO_ERROR;
}

/*
static int loadVorbisFile(const std::string& filePath, vorbis_info& vorbisInfo, vorbis_comment& vorbisComment,
	float*& sampleBuffer,
	size_t& sampleLength)
{
	RESIZABLE_L.clearBuffer();
	RESIZABLE_R.clearBuffer();
	int TOTAL_ERROR = OGG_NO_ERROR;
	using namespace std;

	StreamMap streams;
	ogg_sync_state state{};
	ogg_stream_state streamState;
	int error;
	error = ogg_sync_init(&state);
	if (error != 0)
		return OGG_ERROR;
	ogg_page page{};

	auto src = ND_RESLOC(string(filePath));
	FILE* file = fopen(src.c_str(), "rb");
	ASSERT(file, "");

	bool contuni = true;
	//page load and store in the stream
Lapel:
	while (contuni)
	{
		//wait to gather whole first page

		while (ogg_sync_pageout(&state, &page) != 1)
		{
			char* buffer = ogg_sync_buffer(&state, OGG_FILE_BUFF_SIZE);
			ASSERT(buffer, "Cannot get a buffer");

			auto bytes = fread(buffer, 1, OGG_FILE_BUFF_SIZE, file);
			if (bytes == 0)
			{
				contuni = false;
				goto Lapel;
			}
			int ret = ogg_sync_wrote(&state, bytes);
			ASSERT(ret == 0, "Something's fishy");
		}

		int serial = ogg_page_serialno(&page);
		ogg_stream* stream = 0;
		if (ogg_page_bos(&page))
		{
			stream = new ogg_stream;
			stream->serial = serial;

			error = ogg_stream_init(&stream->state, serial);
			ASSERT(error == 0, "Cannot init ogg stream");
			streams[serial] = stream;
		}
		else
			stream = streams[serial];

		//put loaded page into the stream, yay
		error = ogg_stream_pagein(&stream->state, &page);
		ASSERT(error == 0, "");

		ogg_packet packet;
		int response;

		//static int samples = 0;
		while ((response = ogg_stream_packetout(&stream->state, &packet)) == 1)
		{
			auto headerOut = vorbis_synthesis_headerin(&stream->vorbis.info, &stream->vorbis.comment, &packet);
			if (stream->stream_type == OGG_STREAM_TYPE_VORBIS && headerOut == OV_ENOTVORBIS)
			{
				if (stream->vorbis.is_first_packet)
				{
					vorbisInfo = stream->vorbis.info;
					vorbisComment = stream->vorbis.comment;
					stream->vorbis.initBlockState();
				}
				if (vorbis_synthesis(&stream->vorbis.block, &packet) == 0)
					vorbis_synthesis_blockin(&stream->vorbis.state, &stream->vorbis.block);
				float** buff;
				auto floatsAvailable = vorbis_synthesis_pcmout(&stream->vorbis.state, &buff);
				if (floatsAvailable)
				{
					RESIZABLE_L.appendData(buff[0], floatsAvailable);
					if (stream->vorbis.info.channels > 1)
						RESIZABLE_R.appendData(buff[1], floatsAvailable);
					vorbis_synthesis_read(&stream->vorbis.state, floatsAvailable);
				}

				//normal packet
			}
			else if (headerOut == 0)
			{
				stream->stream_type = OGG_STREAM_TYPE_VORBIS;
			}
			// A packet is available, this is what we pass to the vorbis or
			// theora libraries to decode.
			stream->packet_count++;
		}
		if (response == 0)
		{
			// Need more data to be able to complete the packet
			continue;
		}
		else if (response == -1)
		{
			// We are out of sync and there is a gap in the data.
			// We lost a page somewhere.
			TOTAL_ERROR = OGG_ERROR;
			break;
		}
	}
	fclose(file);
	ogg_sync_clear(&state);

	if (TOTAL_ERROR == OGG_NO_ERROR)
	{
		float* outBuff = nullptr;
		if (vorbisInfo.channels == 2)
		{
			outBuff = (float*)malloc(RESIZABLE_L.getSampleNumber() * sizeof(float) * 2);
			mergeBuffers(outBuff, RESIZABLE_L.getData(), RESIZABLE_R.getData(), RESIZABLE_L.getSampleNumber());
		}
		else
		{
			outBuff = (float*)malloc(RESIZABLE_L.getSampleNumber() * sizeof(float));
			memcpy(outBuff, RESIZABLE_L.getData(), RESIZABLE_L.getSampleNumber() * sizeof(float));
		}
		sampleLength = RESIZABLE_L.getSampleNumber();
		sampleBuffer = outBuff;
	}
	return TOTAL_ERROR;
}*/

/*static int loadVorbisMemory(const void* data, size_t dataSize, vorbis_info& vorbisInfo, vorbis_comment& vorbisComment,
	float*& sampleBuffer,
	size_t& sampleLength)
{
	RESIZABLE_L.clearBuffer();
	RESIZABLE_R.clearBuffer();
	int TOTAL_ERROR = OGG_NO_ERROR;
	using namespace std;

	StreamMap streams;
	ogg_sync_state state{};
	ogg_stream_state streamState;
	int error = ogg_sync_init(&state);
	if (error != 0)
		return OGG_ERROR;
	ogg_page page{};

	size_t memOffset = 0;

	bool contuni = true;
	//page load and store in the stream
Lapel:
	while (contuni)
	{
		//wait to gather whole first page

		while (ogg_sync_pageout(&state, &page) != 1)
		{
			char* buffer = ogg_sync_buffer(&state, OGG_FILE_BUFF_SIZE);
			ASSERT(buffer, "Cannot get a buffer");

			size_t bytesToRead = std::min((unsigned long long)OGG_FILE_BUFF_SIZE, dataSize - memOffset);
			if (bytesToRead == 0)
			{
				contuni = false;
				goto Lapel;
			}
			memcpy(buffer, (char*)data + memOffset, bytesToRead);
			memOffset += bytesToRead;
			int ret = ogg_sync_wrote(&state, bytesToRead);
			ASSERT(ret == 0, "Something's fishy");
		}

		int serial = ogg_page_serialno(&page);
		ogg_stream* stream = 0;
		if (ogg_page_bos(&page))
		{
			stream = new ogg_stream;
			stream->serial = serial;

			error = ogg_stream_init(&stream->state, serial);
			ASSERT(error == 0, "Cannot init ogg stream");
			streams[serial] = stream;
		}
		else
			stream = streams[serial];

		//put loaded page into the stream, yay
		error = ogg_stream_pagein(&stream->state, &page);
		//ND_INFO("next page {}",RESIZABLE.getSampleNumber());
		ASSERT(error == 0, "");

		ogg_packet packet;
		int response;
		//static int samples = 0;
		while ((response = ogg_stream_packetout(&stream->state, &packet)) == 1)
		{
			auto headerOut = vorbis_synthesis_headerin(&stream->vorbis.info, &stream->vorbis.comment, &packet);
			if (stream->stream_type == OGG_STREAM_TYPE_VORBIS && headerOut == OV_ENOTVORBIS)
			{
				if (stream->vorbis.is_first_packet)
				{
					vorbisInfo = stream->vorbis.info;
					vorbisComment = stream->vorbis.comment;
					stream->vorbis.initBlockState();
				}
				if (vorbis_synthesis(&stream->vorbis.block, &packet) == 0)
					vorbis_synthesis_blockin(&stream->vorbis.state, &stream->vorbis.block);
				float** buff;
				auto floatsAvailable = vorbis_synthesis_pcmout(&stream->vorbis.state, &buff);
				if (floatsAvailable)
				{
					RESIZABLE_L.appendData(buff[0], floatsAvailable);
					if (stream->vorbis.info.channels == 2)
						RESIZABLE_R.appendData(buff[1], floatsAvailable);
					vorbis_synthesis_read(&stream->vorbis.state, floatsAvailable);
				}

				//normal packet
			}
			else if (headerOut == 0)
			{
				stream->stream_type = OGG_STREAM_TYPE_VORBIS;
			}
			// A packet is available, this is what we pass to the vorbis or
			// theora libraries to decode.
			stream->packet_count++;
		}
		if (response == 0)
		{
			// Need more data to be able to complete the packet
			continue;
		}
		else if (response == -1)
		{
			// We are out of sync and there is a gap in the data.
			// We lost a page somewhere.
			TOTAL_ERROR = OGG_ERROR;
			break;
		}
	}
	ogg_sync_clear(&state);

	if (TOTAL_ERROR == OGG_NO_ERROR)
	{
		float* outBuff = nullptr;
		if (vorbisInfo.channels == 2)
		{
			outBuff = (float*)malloc(RESIZABLE_L.getSampleNumber() * sizeof(float) * 2);
			mergeBuffers(outBuff, RESIZABLE_L.getData(), RESIZABLE_R.getData(), RESIZABLE_L.getSampleNumber());
		}
		else
		{
			outBuff = (float*)malloc(RESIZABLE_L.getSampleNumber() * sizeof(float));
			memcpy(outBuff, RESIZABLE_L.getData(), RESIZABLE_L.getSampleNumber() * sizeof(float));
		}
		sampleLength = RESIZABLE_L.getSampleNumber();
		sampleBuffer = outBuff;
	}
	return TOTAL_ERROR;
}*/

SoundBuffer::~SoundBuffer()
{
	if (m_is_malloc)
		free(m_buffer);
}

bool SoundBuffer::loadFromFile(const char* filePath)
{
	//ND_INFO("Sample number of file {}, is {}", filePath, getNumberOfSamples(filePath.c_str()));
	vorbis_comment comment;
	vorbis_comment_init(&comment);
	vorbis_info_init(&m_info);
	int success = loadVorbisFileVorbisFile(filePath, m_info, comment, m_buffer, m_samples_total);
	if (success != OGG_NO_ERROR)
	{
		m_samples_total = 0;
		m_buffer = nullptr;
		ND_ERROR("cannot load sound {}", filePath);
		return false;
	}
	m_id = SID(filePath);
	m_is_malloc = true;
	return true;
}

bool SoundBuffer::loadFromMemory(const void* data, size_t length)
{
	return false;
	/*vorbis_comment comment;
	vorbis_comment_init(&comment);
	vorbis_info_init(&m_info);
	int success = loadVorbisMemory(data, length, m_info, comment, m_buffer, m_samples_total);
	if (success != OGG_NO_ERROR)
	{
		m_samples_total = 0;
		m_buffer = nullptr;
		ND_ERROR("cannot load sound");
		return false;
	}
	m_is_malloc = true;
	return true;*/
}

void SoundBuffer::allocate(size_t samplesTotal, const vorbis_info& info, float* memory)
{
	m_info = info;
	if (m_is_malloc)
		free(m_buffer);
	if (memory)
		m_buffer = memory;
	else
		m_buffer = (float*)malloc(samplesTotal * info.channels * sizeof(float));
	m_current_write_head = 0;
	m_samples_total = samplesTotal;
	m_is_malloc = memory == nullptr;
}

void SoundBuffer::loadRaw(const float* data, size_t sampleFrames)
{
	if (m_current_write_head + sampleFrames > m_samples_total)
	{
		ND_WARN("Trying to insert more data than was allocated, ignoring");
		return;
	}
	//ASSERT(m_current_write_head + sampleFrames <= m_samples_total, "Trying to insert more data than was allocated");
	memcpy(m_buffer + m_current_write_head * m_info.channels, data, sampleFrames * m_info.channels * sizeof(float));
	m_current_write_head += sampleFrames;
}

MusicStream::~MusicStream()
{
	close();
}

bool MusicStream::initFromFile(const char* filePath)
{
	m_file_stream = fopen(filePath, "rb");

	if (m_file_stream == nullptr)
	{
		ND_WARN("Cannot open sound file {}", filePath);
		return false;
	}

	if (ov_open_callbacks(m_file_stream, &m_vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
	{
		ND_WARN("Cannot open sound stream {}", filePath);
		fclose(m_file_stream);
		return false;
	}

	m_comment = *ov_comment(&m_vf, -1);
	m_info = *ov_info(&m_vf, -1);
	m_total_samples = ov_pcm_total(&m_vf, -1);
	debug_pcm_by_time = ov_time_total(&m_vf, -1) * m_info.rate;
	m_is_opened = true;
	return true;
}

bool MusicStream::initFromFileOld(const char* filePath)
{
	/*using namespace std;
	m_is_page_loaded = false;
	auto src = ND_RESLOC(string(filePath));
	m_file_stream = fopen(src.c_str(), "rb");

	if (!m_file_stream)
	{
		ND_WARN("Cannot open file sound");
		return false;
	}

	int error = ogg_sync_init(&m_state);
	ASSERT(error == 0, "");

	//page load and store in the stream
	bool success = true;

	int vorbisHeaderCount = 0;
	while (success)
	{
		//wait to gather whole first page
		while (ogg_sync_pageout(&m_state, &m_page) != 1)
		{
			char* buffer = ogg_sync_buffer(&m_state, OGG_FILE_BUFF_SIZE);
			ASSERT(buffer, "Cannot get a buffer");

			auto bytes = fread(buffer, 1, OGG_FILE_BUFF_SIZE, m_file_stream);
			if (bytes == 0)
			{
				success = false;
				break;
			}
			int ret = ogg_sync_wrote(&m_state, bytes);
			ASSERT(ret == 0, "Something's fishy");
		}

		int serial = ogg_page_serialno(&m_page);
		ogg_stream* stream = 0;
		if (ogg_page_bos(&m_page))
		{
			stream = new ogg_stream;
			stream->serial = serial;

			auto error = ogg_stream_init(&stream->state, serial);
			ASSERT(error == 0, "Cannot init ogg stream");
			m_streams[serial] = stream;
		}
		else
			stream = m_streams[serial];

		//put loaded page into the stream, yay
		auto error = ogg_stream_pagein(&stream->state, &m_page);
		ASSERT(error == 0, "");


		int response;
		//static int samples = 0;
		while ((response = ogg_stream_packetout(&stream->state, &m_packet)) == 1)
		{
			auto headerOut = vorbis_synthesis_headerin(&stream->vorbis.info, &stream->vorbis.comment, &m_packet);
			if (headerOut == 0)
			{
				stream->stream_type = OGG_STREAM_TYPE_VORBIS;
				m_ogg_stream = stream;
				if ((++vorbisHeaderCount) == 3)
				{
					//loaded all the three kings, wait a minute... the three headers of vorbis
					//end of current page -> on next page starts audio

					m_ogg_stream->vorbis.initBlockState();
					//m_total_samples = ov_pcm_total(OggVorbis_File)
					return true;
				}
			}
			// A packet is available, this is what we pass to the vorbis or
			// theora libraries to decode.
			stream->packet_count++;
		}
		if (response == -1 || stream->stream_type != OGG_STREAM_TYPE_VORBIS)
		{
			success = false;
			break;
		}
	}
	close();*/
	return false;
}

bool MusicStream::readNext(int maxFrames, NDUtil::RingBufferLite& ringBuffer, SoundBuffer* optionalConsumer)
{
	if (m_is_done)
		return true;
	if (ringBuffer.isFull())
		return false;

	ASSERT(!optionalConsumer || (optionalConsumer->getChannels() == m_info.channels),
		"Sound buffer has different number of channels");


	int currentFrameNumber = 0;
	int samplesPerFrame = ringBuffer.getFrameSize() / sizeof(float) / m_info.channels;
	while (true)
	{
		if (m_samples_ret <= 0)
		{
			//read new only if we have consumed all the previous data from buff
			m_samples_ret = ov_read_float(&m_vf, &m_data, 4096, &m_current_section);
			debug_samples_written += m_samples_ret;
			m_data_offset = 0;
		}
		if (m_samples_ret == 0)
		{
			//we are done = eof
			//now we need to fill last frame with partial data
			if (m_samples_written)
			{
				auto ringBuff = (float*)ringBuffer.write();
				//no need to check if m_samples_written is not zero
				//if (ringBuff == nullptr)
				//	return false; //the ringbuffer is full

				//fill the rest with zeros
				ZeroMemory(ringBuff + m_samples_written * m_info.channels,
					(samplesPerFrame - m_samples_written) * m_info.channels * sizeof(float));
				if (optionalConsumer)
					optionalConsumer->loadRaw(ringBuff, m_samples_written);
				ringBuffer.push();
				m_samples_written = 0; //just to be sure
			}
			m_is_done = true;
			return true;
		}
		while (m_samples_ret > 0)
		{
			auto ringBuff = (float*)ringBuffer.write();
			if (ringBuff == nullptr)
				return false; //the ringbuffer is full

			ringBuff += m_samples_written * m_info.channels;
			int samplesToWrite = std::min(m_samples_ret, (long)(samplesPerFrame - m_samples_written));
			if (m_info.channels == 1)
				memcpy(ringBuff, m_data[0] + m_data_offset, samplesToWrite * sizeof(float));
			else if (m_info.channels == 2)
				mergeBuffers(ringBuff, m_data[0] + m_data_offset, m_data[1] + m_data_offset, samplesToWrite);

			m_samples_written += samplesToWrite;
			bool framedone = m_samples_written == samplesPerFrame;
			m_samples_ret -= samplesToWrite;
			m_data_offset += samplesToWrite;

			//called every time the ring frame was filled and is ready to be pushed
			if (framedone)
			{
				m_samples_written = 0;
				m_current_time_millis += 1000.f / m_info.rate * samplesPerFrame;
				if (optionalConsumer)
				{
					if (!optionalConsumer->isFilled())
					{
						optionalConsumer->loadRaw((float*)ringBuffer.write(), samplesPerFrame);
					}
					else
					{
						debug_oversamples += samplesPerFrame;
						ND_WARN("Overflow of soundbuff");
					}
				}
				ringBuffer.push();

				if (++currentFrameNumber == maxFrames)
					return false;
			}
		}
	}
}

bool MusicStream::readNextOld(int maxFrames, NDUtil::RingBufferLite& ringBuffer)
{
	return true;
	/*if (ringBuffer.isFull())
		return false;

	using namespace std;
	int totalFrames = 0;

	//page load and store in the stream
	while (true)
	{
		ogg_stream* stream = nullptr;
		//wait to gather whole first page
		if (!m_is_page_loaded)
		{
			while (ogg_sync_pageout(&m_state, &m_page) != 1)
			{
				char* buffer = ogg_sync_buffer(&m_state, OGG_FILE_BUFF_SIZE);
				ASSERT(buffer, "Cannot get a buffer");

				auto bytes = fread(buffer, 1, OGG_FILE_BUFF_SIZE, m_file_stream);

				if (bytes == 0)
				{
					//were have read whole file
					m_is_done = true;
					return true;
				}
				int ret = ogg_sync_wrote(&m_state, bytes);
				ASSERT(ret == 0, "Something's fishy");
			}
			int serial = ogg_page_serialno(&m_page);
			if (ogg_page_bos(&m_page))
			{
				stream = new ogg_stream;
				stream->serial = serial;

				auto error = ogg_stream_init(&stream->state, serial);
				ASSERT(error == 0, "Cannot init ogg stream");
				m_streams[serial] = stream;
			}
			else
				stream = m_streams[serial];

			//put loaded page into the stream, yay
			auto error = ogg_stream_pagein(&stream->state, &m_page);
			ASSERT(error == 0, "");
			m_is_page_loaded = true;
		}

		stream = m_streams[ogg_page_serialno(&m_page)];

		int response = 0;
		int channels = getChannels();
		// get one packet from stream if necessary
		while (m_is_packet_loaded || (response = ogg_stream_packetout(&stream->state, &m_packet)) == 1)
		{
			//check if necessary to check for vorbis_headerin in vorbis known stream
			auto headerOut = m_is_packet_loaded
								 ? OV_ENOTVORBIS
								 : vorbis_synthesis_headerin(&stream->vorbis.info, &stream->vorbis.comment, &m_packet);
			if (headerOut == OV_ENOTVORBIS)
			{
			if (!m_is_packet_loaded)
				if (vorbis_synthesis(&stream->vorbis.block, &m_packet) == 0)
					vorbis_synthesis_blockin(&stream->vorbis.state, &stream->vorbis.block);

			m_is_packet_loaded = true;
			float** buff;
			//	auto samplesAvailable = vorbis_synthesis_pcmout(&stream->vorbis.state, &buff);
			const size_t samplesPerFrame = ringBuffer.getFrameSize() / sizeof(float) / channels;
			int availableSamples;
			while ((availableSamples = vorbis_synthesis_pcmout(&stream->vorbis.state, &buff)) > 0)
			{
				auto outBuff = (float*)ringBuffer.write();
				if (outBuff == nullptr)
					return false; //the ringbuffer is full

				outBuff += m_samples_written * channels;
				int samplesToWrite = min((uint64_t)availableSamples, samplesPerFrame - m_samples_written);
				if (channels == 1)
					memcpy(outBuff, buff[0], samplesToWrite * sizeof(float));
				else
					mergeBuffers(outBuff, buff[0], buff[1], samplesToWrite);

				m_samples_written += samplesToWrite;
				bool framedone = m_samples_written == samplesPerFrame;

				//called every time the ring frame was filled and is ready to be pushed
				if (framedone)
				{
					m_samples_written = 0;
					m_current_time_millis += 1000.f / stream->vorbis.info.rate * samplesPerFrame;
					ringBuffer.push();
				}

				vorbis_synthesis_read(&stream->vorbis.state, samplesToWrite);

				if (framedone && ((++totalFrames) >= maxFrames))
					return false; //reached max number of frames
			}
			m_is_packet_loaded = false;
			//}
		}
		if (response == 0)
		{
			m_is_page_loaded = false;
			// Need more data to be able to complete the packet
			continue;
		}
		if (response == -1)
		{
			ASSERT(false, "Oh damn, error in music stream");
			return true;
		}
	}*/
}

void MusicStream::close()
{
	/*
	 //old
	 if (m_file_stream)
	{
		fclose(m_file_stream);
		m_file_stream = nullptr;
	}
	ogg_sync_clear(&m_state);
	for (auto& pair : m_streams)
		delete pair.second;
	m_streams.clear();
	m_ogg_stream = nullptr;*/
	if (m_is_opened)
	{
		ov_clear(&m_vf);
		fclose(m_file_stream);
		m_is_opened = false;
	}
}