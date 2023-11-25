#pragma once

#include <cstdint>
#include <type_traits>

#include "ByteSwap.h"

class OutputMemoryBitStream
{
public:
	OutputMemoryBitStream() {}
	~OutputMemoryBitStream() {}

	void WriteBits(uint8_t inData, size_t inBitCount);
	void WriteBits(const void* inData, size_t inBitCount);

	void WriteBytes(const void* inData, size_t inByteCount)
	{
		WriteBits(inData, inByteCount << 3);
	}

	void Write(bool inData)
	{
		WriteBits(&inData, 1);
	}

	template <typename T>
	void Write(T inData, size_t inBitCount = sizeof(T) * 8)
	{
		static_assert(std::is_arithmetic<T>::value ||
			std::is_enum<T>::value,
			"Generic Write only supports primitive data types");

		// TODO: bit swap if necessary.
		WriteBits(&inData, inBitCount);
	}

	const char* GetBufferPtr() const { return mBuffer; }
	uint32_t GetBitLength() const { return mBitHead; }
	uint32_t GetByteLength() const { return (mBitHead + 7) >> 3; }

private:
	void ReallocBuffer(uint32_t inNewBitCapacity);

private:
	char* mBuffer = nullptr;
	uint32_t mBitHead = 0;
	uint32_t mBitCapacity = 0;
};
