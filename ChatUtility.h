#pragma once

class OutputMemoryStream;
class InputMemoryStream;

struct ChatSyncData
{
	char message[1024];
	bool bFinalMessageInQueue = false;

	bool Write(OutputMemoryStream& outStream);
	bool Read(InputMemoryStream& inStream);
};
