#include "MemoryStream.h"
#include <algorithm>

#include "LinkingContext.h"

bool IsPlatformLittleEndian()
{
	uint16_t number = 0x1;
	char* numPtr = reinterpret_cast<char*>(&number);
	return (numPtr[0] == 1);
}

bool MemoryStream::ReallocBuffer(uint32_t inNewLength)
{
	char* newBuffer = static_cast<char*>(std::realloc(mBuffer, inNewLength));
	if (newBuffer)
	{
		mBuffer = newBuffer;
		newBuffer = nullptr;
	}
	else
	{
		// TODO: Handle realloc failure.
		return false;
	}

	mCapacity = inNewLength;
	return true;
}

// ---------------------------------------------------------------------------

bool OutputMemoryStream::Write(const void* inData, size_t inByteCount)
{
	uint32_t resultHead = mHead + static_cast<uint32_t>(inByteCount);
	if (resultHead > mCapacity)
	{
		if (!ReallocBuffer(std::max(mCapacity * 2, resultHead)))
			return false;
	}

	std::memcpy(mBuffer + mHead, inData, inByteCount);
	mHead = resultHead;

	return true;
}

bool OutputMemoryStream::Write(const ChatObject* inChatObject)
{
	if (!mLinkingContext)
	{
		// TODO: Handle error.
		return false;
	}

	uint32_t networkId = mLinkingContext->GetNetworkId(inChatObject);
	Write(networkId);

	return true;
}

// ---------------------------------------------------------------------------

bool InputMemoryStream::Read(void* outData, uint32_t outByteCount)
{
	uint32_t resultHead = mHead + outByteCount;
	if (resultHead > mCapacity)
	{
		// TODO: Handle buffer overrun error.
		return false;
	}

	std::memcpy(outData, mBuffer + mHead, outByteCount);
	mHead = resultHead;

	return true;
}

bool InputMemoryStream::Read(ChatObject*& outChatObject)
{
	if (!mLinkingContext)
	{
		// TODO: Handle error.
		return false;
	}

	uint32_t networkId;
	Read(networkId);

	outChatObject = mLinkingContext->GetChatObject(networkId);

	return true;
}

void InputMemoryStream::Reset()
{
	mHead = 0;
}
