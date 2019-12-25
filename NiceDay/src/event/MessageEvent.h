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
	int m_id;
	char m_title[MESSAGE_EVENT_TITLE_MAX_LENGTH+1]{};
	uint64_t m_metadata;
	void* m_data;

public:
	MessageEvent(const char* title,uint64_t meta=0,void* data=nullptr):
		m_metadata(meta),m_data(data)
	{
		ASSERT(strlen(title) < MESSAGE_EVENT_TITLE_MAX_LENGTH,"Message title too long");
		cpyString(title, &m_title[0]);
	}
	MessageEvent(int id, uint64_t meta = 0, void* data = nullptr) :
		m_id(id),m_metadata(meta), m_data(data)
	{
	}
	inline const char* getTitle() const { return m_title; }
	inline bool isTitle(const char* title)const
	{
		return std::strcmp(title, m_title);
	}
	inline uint64_t getMetadata() const { return m_metadata; }
	inline void* getData() const { return m_data; }
	inline int getID() const { return m_id; };

	EVENT_TYPE_BUILD(Message)
	EVENT_CATEGORY_BUILD(Message)
	EVENT_COPY(MessageEvent)
	
};

typedef std::function<void(MessageEvent&)> MessageConsumer;