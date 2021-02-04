#pragma once
#include "Event.h"
#include "core/sids.h"

static void cpyString(const char* src,char* dest)
{
	memcpy(dest, src, strlen(src));
}

constexpr size_t MESSAGE_EVENT_TITLE_MAX_LENGTH = 100;

class MessageEvent:public Event
{
private:
	int m_id=0;
	std::string m_title;
	std::string m_text;
	uint64_t m_metadata;
	void* m_data;

public:
	MessageEvent(const char* title,uint64_t meta=0,void* data=nullptr):
		m_metadata(meta),m_data(data),m_title(title)
	{
	}
	MessageEvent(const char* title, const std::string& text) :
	   m_text(text), m_title(title)
	{
	}
	MessageEvent(int id, uint64_t meta = 0, void* data = nullptr) :
		m_id(id),m_metadata(meta), m_data(data)
	{
	}

	const auto& getText()const { return m_text; }
	inline const std::string& getTitle() const { return m_title; }
	inline bool isTitle(const char* title)const
	{
	   return m_title == title;
	}
	inline uint64_t getMetadata() const { return m_metadata; }
	inline void* getData() const { return m_data; }
	inline int getID() const { return m_id; };

	EVENT_TYPE_BUILD(Message)
	EVENT_CATEGORY_BUILD(Message)
	EVENT_COPY(MessageEvent)
	
};

typedef std::function<void(MessageEvent&)> MessageConsumer;

class DropFilesEvent:public Event
{
private:
	int m_count;
	const char** m_paths;
public:
	DropFilesEvent(int count, const char** paths) :
		m_count(count), m_paths(paths)
	{
		
	}
	constexpr int count() const { return m_count; }
	const char* getPath(int index) const
	{
		ASSERT(index < m_count, "Invalid index");
		return m_paths[index];
	}
	

	EVENT_TYPE_BUILD(Drop)
	EVENT_CATEGORY_BUILD(Window)
	EVENT_COPY(DropFilesEvent)

};