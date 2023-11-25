#pragma once

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <vector>

#include "ByteSwap.h"

class ChatObject;
class LinkingContext;

bool IsPlatformLittleEndian();

class MemoryStream
{
public:
	MemoryStream() {}
	virtual ~MemoryStream() { std::free(mBuffer); }

	void SetLinkingContext(LinkingContext* inLinkingContext) { mLinkingContext = inLinkingContext; }

	bool ReallocBuffer(uint32_t inNewLength);

protected:
	char* mBuffer = nullptr;
	uint32_t mHead = 0;
	uint32_t mCapacity = 0;

	LinkingContext* mLinkingContext = nullptr;
};

class OutputMemoryStream : public MemoryStream
{
public:
	OutputMemoryStream()
	{
		ReallocBuffer(32);
	}

	bool Write(const void* inData, size_t inByteCount);

	template<typename T>
	bool Write(T inData)
	{
		static_assert(std::is_arithmetic<T>::value ||
			std::is_enum<T>::value,
			"Generic Write only supports primitive data types");

		// Always write in little endian format.
		if (IsPlatformLittleEndian())
			return Write(&inData, sizeof(inData));

		T swappedData = ByteSwap(inData);
		return Write(&swappedData, sizeof(swappedData));
	}

	template <typename T>
	bool Write(const std::vector<T>& inVector)
	{
		size_t elementCount = inVector.size();
		if (!Write(elementCount)) return false;

		for (const T& element : inVector)
			if (!Write(element)) return false;

		return true;
	}

	bool Write(const ChatObject* inChatObject);

	const char* GetBufferPtr() const { return mBuffer; }
	uint32_t GetLength() const { return mHead; }
};

class InputMemoryStream : public MemoryStream
{
public:
	InputMemoryStream() = delete;

	InputMemoryStream(char* inBuffer, uint32_t inByteCount)
	{
		mBuffer = inBuffer;
		mCapacity = inByteCount;
		mHead = 0;
	}
	InputMemoryStream(uint32_t maxByteCount)
	{
		ReallocBuffer(maxByteCount);
		mHead = 0;
	}

	char* GetBufferPtr() const { return mBuffer; }
	uint32_t GetRemainingDataSize() const { return mCapacity - mHead; }
	uint32_t GetCapacity() const { return mCapacity; }

	bool Read(void* outData, uint32_t outByteCount);

	template<typename T>
	bool Read(T& outData)
	{
		static_assert(std::is_arithmetic<T>::value ||
			std::is_enum<T>::value,
			"Generic Read only supports primitive data types");

		if (!Read(&outData, sizeof(outData))) return false;

		// We always sure, that data is in little endian format.
		// So if this platform is not we should swap the endianness.
		if (!IsPlatformLittleEndian())
			outData = ByteSwap(outData);
		return true;
	}

	template <typename T>
	bool Read(std::vector<T> outVector)
	{
		size_t elementCount;
		if (!Read(elementCount)) return false;

		outVector.resize(elementCount);
		for (const T& element : outVector)
			if (!Read(element)) return false;
		return true;
	}

	bool Read(ChatObject*& outChatObject);

	void Reset();
};
