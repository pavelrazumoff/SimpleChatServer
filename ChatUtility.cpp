#include "ChatUtility.h"

#include "Serialization/MemoryStream.h"

bool ChatSyncData::Serialize(MemoryStream* ioStream)
{
	if (!ioStream) return false;

	uint16_t msgLen = ioStream->IsInput() ? 0 :
		static_cast<uint16_t>(strlen(message)) + 1;

	if (!ioStream->Serialize(msgLen) ||
		!ioStream->Serialize(message, msgLen) ||
		!ioStream->Serialize(bFinalMessageInQueue))
		return false;

	return true;
}
