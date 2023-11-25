#include "MemoryBitStream.h"
#include <algorithm>

void OutputMemoryBitStream::WriteBits(uint8_t inData, size_t inBitCount)
{
	uint32_t nextBitHead = mBitHead + static_cast<uint32_t>(inBitCount);
	if (nextBitHead > mBitCapacity)
	{
		ReallocBuffer(std::max(mBitCapacity * 2, nextBitHead));
	}

	uint32_t byteOffset = mBitHead >> 3;
	uint32_t bitOffset = mBitHead & 0x7;

	uint8_t currentMask = ~(0xff << bitOffset);
	mBuffer[byteOffset] = (mBuffer[byteOffset] & currentMask) |
		(inData << bitOffset);

	uint32_t bitsFreeThisByte = 8 - bitOffset;

	if (bitsFreeThisByte < inBitCount)
	{
		mBuffer[byteOffset + 1] = inData >> bitsFreeThisByte;
	}

	mBitHead = nextBitHead;
}

void OutputMemoryBitStream::WriteBits(const void* inData, size_t inBitCount)
{
	const char* srcByte = static_cast<const char*>(inData);

	while (inBitCount > 8)
	{
		WriteBits(*srcByte, 8);
		++srcByte;
		inBitCount -= 8;
	}

	if (inBitCount > 0)
		WriteBits(*srcByte, inBitCount);
}

void OutputMemoryBitStream::ReallocBuffer(uint32_t inNewBitCapacity)
{
	if (!mBuffer)
	{
		mBuffer = static_cast<char*>(std::malloc(inNewBitCapacity >> 3));
		if (mBuffer)
			memset(mBuffer, 0, inNewBitCapacity >> 3);
		else
		{
			// TODO: Handle error.
			return;
		}
	}
	else
	{
		char* tempBuffer = static_cast<char*>(std::malloc(inNewBitCapacity >> 3));
		if (tempBuffer)
		{
			memset(tempBuffer, 0, inNewBitCapacity >> 3);
			memcpy(tempBuffer, mBuffer, mBitCapacity >> 3);
			std::free(mBuffer);
			mBuffer = tempBuffer;
		}
		else
		{
			// TODO: Handle error.
			return;
		}
	}

	mBitCapacity = inNewBitCapacity;
}
