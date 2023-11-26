#pragma once

#include <cstdint>
#include <string>

#define mvOffsetOf(c, mv) ((size_t) & (static_cast<c*>(nullptr)->mv))

enum class EPrimitiveType
{
	EPT_Int,
	EPT_String,
	EPT_Float,
	EPT_Bool,
};

class MemberVariable
{
public:
	MemberVariable(const char* inName, EPrimitiveType inPrimitiveType, uint32_t inOffset)
		: mName(inName), mPrimitiveType(inPrimitiveType), mOffset(inOffset) {}

	EPrimitiveType GetPrimitiveType() const { return mPrimitiveType; }
	uint32_t GetOffset() const { return mOffset; }

private:
	std::string mName;
	EPrimitiveType mPrimitiveType;
	uint32_t mOffset;
};
