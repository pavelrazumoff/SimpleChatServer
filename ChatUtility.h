#pragma once

struct ChatSyncData
{
	char message[1024];
	bool bFinalMessageInQueue = false;
};
