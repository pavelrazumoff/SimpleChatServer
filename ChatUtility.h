#pragma once

#include "ReflectionSystem/DataType.h"

class MemoryStream;

#define USE_REFLECTION_SYSTEM_DATA() 1

#if USE_REFLECTION_SYSTEM_DATA()

class ChatObject
{
public:
	ChatObject() {}
	ChatObject(const char* inMessage, bool bLastInQueue = true)
		: mMessage(inMessage), bLastMessageInQueue(bLastInQueue)
	{}

	const char* GetChatMessage() const { return mMessage.c_str(); }
	bool IsLastMessageInQueue() const { return bLastMessageInQueue; }

	static DataType* GetDataType();

protected:
	std::string mMessage;
	bool bLastMessageInQueue = false;

private:
	static DataType* sDataType;
};

// -----------------------------------------------------------------------------------------

bool SerializeObject(MemoryStream* ioStream, const DataType* inDataType, uint8_t* inData);
#else // USE_REFLECTION_SYSTEM_DATA()

struct ChatSyncData
{
	char message[1024];
	bool bFinalMessageInQueue = false;

	bool Serialize(MemoryStream* ioStream);
};
#endif // USE_REFLECTION_SYSTEM_DATA()
