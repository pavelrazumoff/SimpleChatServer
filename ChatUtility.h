#pragma once

class MemoryStream;

struct ChatSyncData
{
	char message[1024];
	bool bFinalMessageInQueue = false;

	bool Serialize(MemoryStream* ioStream);
};
