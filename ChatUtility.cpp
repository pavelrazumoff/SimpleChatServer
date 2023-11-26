#include "ChatUtility.h"

#include "Serialization/MemoryStream.h"

#if USE_REFLECTION_SYSTEM_DATA()

DataType* ChatObject::sDataType = nullptr;

DataType* ChatObject::GetDataType()
{
	if (sDataType) return sDataType;

	sDataType = new DataType(
		{
			MemberVariable("mMessage",
				EPrimitiveType::EPT_String, mvOffsetOf(ChatObject, mMessage)),
			MemberVariable("bLastMessageInQueue",
				EPrimitiveType::EPT_Bool, mvOffsetOf(ChatObject, bLastMessageInQueue)),
		}
	);

	return sDataType;
}

// -----------------------------------------------------------------------------------------

bool SerializeObject(MemoryStream* ioStream, const DataType* inDataType, uint8_t* inData)
{
	if (!ioStream || !inDataType || !inData) return false;

	for (auto& mv : inDataType->GetMemberVariables())
	{
		void* mvData = inData + mv.GetOffset();
		bool bSuccess = false;

		switch (mv.GetPrimitiveType())
		{
			case EPrimitiveType::EPT_Int:
				bSuccess = ioStream->Serialize(*(int*)mvData);
				break;
			case EPrimitiveType::EPT_String:
				bSuccess = ioStream->Serialize(*(std::string*)mvData);
				break;
			case EPrimitiveType::EPT_Float:
				bSuccess = ioStream->Serialize(*(float*)mvData);
				break;
			case EPrimitiveType::EPT_Bool:
				bSuccess = ioStream->Serialize(*(bool*)mvData);
				break;
			default:
				break;
		}

		if (!bSuccess) return false;
	}

	return true;
}
#else // USE_REFLECTION_SYSTEM_DATA()

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
#endif // USE_REFLECTION_SYSTEM_DATA()
