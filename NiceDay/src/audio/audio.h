#pragma once
#include <vorbis/codec.h>
#include <ogg/ogg.h>
#include <unordered_map>
#include <vorbis/vorbisfile.h>

#include "memory/RingBuffer.h"
#include "core/sids.h"

struct vorbis_stream
{
	vorbis_info info;
	vorbis_comment comment;
	vorbis_dsp_state state;
	vorbis_block block;
	bool is_first_packet = true;

	vorbis_stream()
	{
		vorbis_info_init(&info);
		vorbis_comment_init(&comment);
	}

	void initBlockState()
	{
		vorbis_synthesis_init(&state, &info);
		vorbis_block_init(&state, &block);
		is_first_packet = false;
	}

	void printInfo()
	{
		/*ND_INFO("StreamInfo:");
		ND_INFO("	Channels:\t{}", info.channels);
		ND_INFO("	Rate:\t\t{}", info.rate);
		ND_INFO("	Vendor:\t\t{}", comment.vendor);
		ND_INFO("	Comments:");
		for (int i = 0; i < comment.comments; ++i)
		{
			std::string_view view(comment.user_comments[i], comment.comment_lengths[i]);
			ND_INFO("\t\t {}", view);
		}*/
	}
};

enum ogg_stream_type
{
	OGG_STREAM_TYPE_NONE,
	OGG_STREAM_TYPE_VORBIS
};

struct ogg_stream
{
	ogg_stream_type stream_type = OGG_STREAM_TYPE_NONE;
	int serial;
	ogg_stream_state state;
	int packet_count;
	vorbis_stream vorbis;
};

typedef std::unordered_map<int, ogg_stream*> StreamMap;

/**
 * Represents all data neccesary to play sound
 * stored as float values in mono
 * stored as pairs of left and right float values in stereo
 * (data is interlaced -> stereo [0] = left0 [1] = right0 [2] = left2 [3] = right2 ...)
 * It's RAW -> anyone can play it.
 */
class SoundBuffer
{
private:
	vorbis_info m_info{};
	float* m_buffer;
	bool m_is_malloc = false;
	size_t m_samples_total = 0;
	Strid m_id = 0;
	size_t m_current_write_head = 0;
public:
	~SoundBuffer();
	// first option to create a buffer, will load all data from file
	bool loadFromFile(const char* filePath);
	// not implemented
	// second option to create a buffer, will load itself from memory structured as ogg vorbis file
	bool loadFromMemory(const void* data, size_t length);

	// third option to create a buffer, prepares the buffer for custom write
	// frees all current data
	// pass memory to be used instead of malloc()
	void allocate(size_t samplesTotal, const vorbis_info& info, float* memory = nullptr);
	inline bool isFilled()const { return m_current_write_head >= m_samples_total; }

	// you need to call allocate first
	// will add partial data to buffer
	// be sure to fill only up to the getSamplesPerChannel()
	// NOTE:	sampleFrames refers to number of frames(that consist of 1 float in mono or 2 floats in stereo)
	//			the number of channels doesn't have an effect on this values
	// data is interlaced -> stereo [0] = left0 [1] = right0 [2] = left2 [3] = right2 ...
	void loadRaw(const float* data, size_t sampleFrames);

	int getChannels() const { return m_info.channels; }
	int getRate() const { return m_info.rate; }
	size_t getSamplesPerChannel() const { return m_samples_total; }
	size_t getTotalSamples() const { return m_samples_total * m_info.channels; }
	float* getData() { return m_buffer; }
	Strid getID() const { return m_id; }


};
/**
 * Represents partial data of sound
 * storing as float values in mono
 * storing as pairs of left and right float values in stereo
 *
 * 1. need to initTheStream() which returns true on success
 * 2. based on getRate() and getChannels() prepare the RingBuffer
 * 3. call readNext() until it returns true
 * 4. call destructor or close() to free all the resources
 */
class MusicStream
{
	size_t debug_samples_written = 0;
	size_t debug_pcm_by_time = 0;
	size_t debug_oversamples = 0;
private:
	FILE* m_file_stream = nullptr;
	/*
	// old start
	StreamMap m_streams;
	ogg_sync_state m_state{};
	ogg_stream_state m_stream_state{};
	ogg_page m_page{};
	ogg_stream* m_ogg_stream = nullptr;
	ogg_packet m_packet;

	bool m_is_page_loaded = false;
	bool m_is_packet_loaded = false;
	// old end
	*/
	int m_samples_written = 0;
	float m_current_time_millis = 0;
	size_t m_total_samples = 0;
	bool m_is_opened = false;
	vorbis_comment m_comment;
	vorbis_info m_info;
	OggVorbis_File m_vf;
	int m_current_section;
	float** m_data;
	size_t m_data_offset = 0;
	long m_samples_ret = 0;
	bool m_is_done = false;
public:
	~MusicStream();

	// opens the file stream
	// reads the first header page of vorbis file
	// return true on success
	// you can now call getChannels() and other info methods about stream
	bool initFromFile(const char* filePath);
	// not implemented
	// worked without VorbisFile
	bool initFromFileOld(const char* filePath);

	// fills the ring buffer until its full or maxFrames is reached
	// it can also fill the optional consumer with the same data if not nullptr
	// returns true end of stream
	bool readNext(int maxFrames, RingBufferLite& ringBuffer, SoundBuffer* optionalConsumer = nullptr);
	// not implemented
	// worked without VorbisFile
	bool readNextOld(int maxFrames, RingBufferLite& ringBuffer);

	inline bool isOpened() const { return m_is_opened; }
	inline bool isDone() const { return m_is_done; }

	// close stream and free associated memory
	void close();
	// time in milliseconds of how much data was put into the ringbuffer in total (based on getRate())
	float getCurrentMillis() const { return m_current_time_millis; }
	//cannot access if not opened
	int getChannels() const { return m_info.channels; }
	//cannot access if not opened
	int getRate() const { return m_info.rate; }
	//cannot access if not opened
	const vorbis_info& getVorbisInfo() const { return m_info; }
	//cannot access if not opened
	const vorbis_comment& getVorbisComment() const { return m_comment; }

	size_t getTotalSamples() const { return m_total_samples; }
};
