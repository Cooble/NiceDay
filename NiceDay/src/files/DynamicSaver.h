#pragma once
#include "ndpch.h"

namespace nd {
struct ChunkSegmentHeader
{
	uint64_t next_index = std::numeric_limits<uint64_t>::max();

	inline bool hasNext() const { return next_index != std::numeric_limits<uint64_t>::max(); }
};

constexpr uint64_t DYNAMIC_SAVER_FREE_SEGMENTS = 10000;
constexpr uint64_t DYNAMIC_SAVER_CHUNK_ID_COUNT = 10000;
constexpr uint64_t DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE = 2000 + sizeof(ChunkSegmentHeader);

typedef int64_t _DSChunkID;
// (uses linked list)
// capable of saving objects with different sizes
struct pai
{
	_DSChunkID id;
	uint64_t offset;
};


//	====DynamicSaver=======
//	
// Capable of saving objects with unique IDs with different sizes to file
// (uses linked list mechanism)
// No upper limit of how big objects can be
// 
// NOTE:
//		Writing mode completely overwrites previously written data of specific object.
//		Appending more data is impossible. (you need to read all, modify, and save all)
// 
// How to use DynamicSaver:
// 	
//		init();//only once for instance
//		
//		beginSession();
//		
//			// writing
//			setWriteChunkID(smth);
//				write(smth,somth); ......
//				write(smth,somth); ......
//			flushWrite();
//		
//			// reading
//			setReadChunkID(smth);
//				read(smth,srhthr); .....
//				read(smth,srhthr); .....
//		
//		endSession();
class DynamicSaver
{
private:
	bool m_is_opened = false;
	bool m_has_flushed = true;
	// offset of start of dynamic data of dynamic saver (starts after DynamicSaverHeader section)
	uint64_t m_TOTAL_OFFSET;
	// offset of whole dynamic saver in file
	uint64_t m_BASE_TOTAL_OFFSET;
	std::string m_path;

	uint64_t m_p_offset, m_p_byte_reciproc;
	uint64_t m_g_offset, m_g_byte_reciproc;
	ChunkSegmentHeader m_write_header;
	ChunkSegmentHeader m_read_header;
	std::fstream* m_stream = nullptr;
	uint64_t m_segment_count = 0;
	_DSChunkID m_writeChunkID = 0;
	bool m_dirty_vtable = false;


	std::deque<uint64_t> m_free_offsets;
	// <chunkID, offset>
	std::unordered_map<_DSChunkID, uint64_t> m_chunk_offsets;

	uint64_t getChunkOffset(_DSChunkID chunkID);

	void eraseSegment(uint64_t offset); //kills segment and all his children

	void loadVTable();

public:
	DynamicSaver(std::string path, uint64_t totalOffset = 0);
	~DynamicSaver();

	// to setup everything (will read from file to update its tables)
	// will create new file if neccessary
	// no beginSession() needed
	void init();

	// to setup everything (will read from file to update its tables)
	// will not create file
	// no beginSession() needed
	// returns true if file exists
	bool initIfExist();

	// should be called often to save changes to vtable
	// NOTE: 
	//		Without table, data is completely useless
	// (called within begin/endSession)
	void saveVTable();


	// to read/write anything you need use begin() and end()
	// creates r/w session
	// opens stream if is closed
	void beginSession();

	// to read/write anything you need use begin() and end()
	// ends r/w session
	// saves vtable if neccessary
	// flushes write if neccessary
	// closes stream if opened
	void endSession();

public:
	// initiates write mode to specified chunk
	// NOTE: 
	//		-> all data on chunkID will be overwritten!
	//		-> don't forget to call flushWrite() after write()s
	//
	// (called within begin/endSession)
	void setWriteChunkID(_DSChunkID chunkID);

	// very important to call after writing to chunk is done
	// kills all segment children if they are not used
	void flushWrite();

	// initiates read mode on specified chunk
	// returns false if chunk doesn't exist
	// (called within begin/endSession)
	bool setReadChunkID(_DSChunkID chunkID);

	// there is no upper limit of how big array can be
	// it will automaticaly create another segment if currently used one is full
	void write(const char* b, uint64_t length);
	inline void writeI(const char* b, uint64_t length) { write(b, length); }

	// returns false if next segment doesn't exist (doesn't care if no other data is available)
	// when reading you need to know the size of data beforehand 
	// otherwise you could read random stuff to the end of the segment
	bool read(char* b, uint64_t length);
	inline void readI(char* b, uint64_t length) { read(b, length); }

	template <typename T>
	void write(const T& t)
	{
		write((const char*)&t, sizeof(T));
	}

	template <typename T>
	void read(T& t)
	{
		read((char*)&t, sizeof(T));
	}

	inline uint64_t getSegmentCount() const { return m_segment_count; }
	inline uint64_t getFreeSegmentCount() const { return m_free_offsets.size(); }
	void clearEverything();
	inline bool isOpened() const { return m_is_opened; }
};
}
