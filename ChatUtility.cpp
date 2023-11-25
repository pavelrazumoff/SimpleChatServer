#include "ChatUtility.h"

#include "Serialization/MemoryStream.h"

bool ChatSyncData::Write(OutputMemoryStream& outStream)
{
	uint16_t msgLen = static_cast<uint16_t>(strlen(message)) + 1;

	if (!outStream.Write(msgLen) ||
		!outStream.Write(message, msgLen) ||
		!outStream.Write(bFinalMessageInQueue))
		return false;

	return true;
}

bool ChatSyncData::Read(InputMemoryStream& inStream)
{
	uint16_t msgLen;
	if (!inStream.Read(msgLen) ||
		!inStream.Read(message, msgLen) ||
		!inStream.Read(bFinalMessageInQueue))
		return false;

	return true;
}
