#pragma once

#include "MemberVariable.h"
#include <vector>

class DataType
{
public:
	DataType(const std::initializer_list<MemberVariable>& inMVs)
		: mMemberVariables(inMVs) {}

	const std::vector<MemberVariable>& GetMemberVariables() const { return mMemberVariables; }

private:
	std::vector<MemberVariable> mMemberVariables;
};
