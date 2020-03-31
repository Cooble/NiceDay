#pragma once
#include "ndpch.h"
#include "core/IBinaryStream.h"

/*
WorldIO::Session is responsible for loading and saving world related data i.e. WorldInfo, Chunks.

Possible errors you might do:

	1. read when write_only mode is set
	2. using write_only when not writing the whole file at once
	3. bad filepath

*/

class World;
class Chunk;
struct WorldInfo;

namespace WorldIO
{

	class Session
	{
	public:
		Session(const std::string& path, bool write_mode, bool write_only = false);
		~Session();
	private:
		std::fstream* m_stream;
		std::string m_file_path;

#ifdef ND_DEBUG
		bool m_write_only;
		bool m_write_mode;
#endif


	public:
		void genWorldFile(const WorldInfo* info);

		void saveWorldMetadata(const WorldInfo* world);

		void saveGenBoolMap(const NDUtil::Bitset* bitset);
		void loadGenBoolMap(NDUtil::Bitset* bitset);

		//return true if success
		bool loadWorldMetadata(WorldInfo* world);

		void loadChunk(Chunk* chunk, int offset);

		void saveChunk(const Chunk* chunk, int offset);

		void close();
	};

	struct ChunkSegmentHeader
	{
		size_t next_index = std::numeric_limits<size_t>::max();

		inline bool hasNext() const { return next_index != std::numeric_limits<size_t>::max(); }
	};

	constexpr uint32_t DYNAMIC_SAVER_FREE_SEGMENTS = 10000;
	constexpr uint32_t DYNAMIC_SAVER_CHUNK_ID_COUNT = 10000;
	constexpr uint32_t DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE = 2000 + sizeof(ChunkSegmentHeader);

	// (uses linked list)
	// capable of saving objects with different sizes
	struct pai
	{
		int id;
		uint32_t offset;
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
		bool m_is_opened=false;
		// offset of start of dynamic data of dynamic saver (starts after DynamicSaverHeader section)
		uint32_t m_TOTAL_OFFSET;
		// offset of whole dynamic saver in file
		uint32_t m_BASE_TOTAL_OFFSET;
		std::string m_path;

		uint32_t m_p_offset, m_p_byte_reciproc;
		uint32_t m_g_offset, m_g_byte_reciproc;
		ChunkSegmentHeader m_write_header;
		ChunkSegmentHeader m_read_header;
		std::fstream* m_stream = nullptr;
		uint32_t m_segment_count = 0;
		int m_writeChunkID = 0;
		bool m_dirty_vtable=false;


		std::deque<uint32_t> m_free_offsets;
		// <chunkID, offset>
		std::unordered_map<int, uint32_t> m_chunk_offsets;

		uint32_t getChunkOffset(int chunkID);

		void eraseSegment(uint32_t offset); //kills segment and all his children

		void loadVTable();

	public:
		DynamicSaver(std::string path, uint32_t totalOffset = 0);
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
		void beginSession();

		// to read/write anything you need use begin() and end()
		// ends r/w session
		void endSession();

	public:
		// initiates write mode to specified chunk
		// NOTE: 
		//		-> all data on chunkID will be overwritten!
		//		-> don't forget to call flushWrite() after write()s
		//
		// (called within begin/endSession)
		void setWriteChunkID(int chunkID);

		// very important to call after writing to chunk is done
		// kills all segment children if they are not used
		void flushWrite();

		// initiates read mode on specified chunk
		// returns false if chunk doesn't exist
		// (called within begin/endSession)
		bool setReadChunkID(int chunkID);

		// there is no upper limit of how big array can be
		// it will automaticaly create another segment if currently used one is full
		void write(const char* b, uint32_t length);
		inline void writeI(const char* b, size_t length) { write(b, (uint32_t)length); }

		// returns false if next segment doesn't exist (doesn't care if no other data is available)
		// when reading you need to know the size of data beforehand 
		// otherwise you could read random stuff to the end of the segment
		bool read(char* b, uint32_t length);
		inline void readI(char* b, size_t length) { read(b, (uint32_t)length); }

		template<typename T>
		void write(const T& t)
		{
			write((const char*)&t, sizeof(T));
		}
		template<typename T>
		void read(T& t)
		{
			read((char*)&t, sizeof(T));
		}

		inline uint32_t getSegmentCount()const { return m_segment_count; }
		inline uint32_t getFreeSegmentCount()const { return m_free_offsets.size(); }
		void clearEverything();
		inline bool isOpened()const { return m_is_opened; }
	};
	IBinaryStream::RWStream streamFuncs(DynamicSaver* saver); 

}
