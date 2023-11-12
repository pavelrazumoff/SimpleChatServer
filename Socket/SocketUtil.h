#pragma once

#include <vector>

#include "UDPSocket.h"
#include "TCPSocket.h"
#include "ws2tcpip.h"

class SocketUtil
{
public:
	static UDPSocketPtr CreateUDPSocket(SocketAddressFamily inFamily);
	static TCPSocketPtr CreateTCPSocket(SocketAddressFamily inFamily);

	template<typename SocketType>
	static fd_set* FillSetFromVector(fd_set& outSet, const std::vector<SocketType>* inSockets);
	template<typename SocketType>
	static void FillVectorFromSet(std::vector<SocketType>* outSockets, const std::vector<SocketType>* inSockets,
		const fd_set& inSet);

	template<typename SocketType>
	static int Select(const std::vector<SocketType>* inReadSet, std::vector<SocketType>* outReadSet,
		const std::vector<SocketType>* inWriteSet, std::vector<SocketType>* outWriteSet,
		const std::vector<SocketType>* inExceptSet, std::vector<SocketType>* outExceptSet,
		const struct timeval* timeout = nullptr);

	template<typename SocketType>
	static int GetSocketAddress(const SocketType* socket, SocketAddressPtr& outAddress);

	static void ReportError(const wchar_t* inOperationDesc);
	static int GetLastError();
};

template<typename SocketType>
fd_set* SocketUtil::FillSetFromVector(fd_set& outSet, const std::vector<SocketType>* inSockets)
{
	if (inSockets)
	{
		FD_ZERO(&outSet);
		for (const SocketType& socket : *inSockets)
		{
			FD_SET(socket->mSocket, &outSet);
		}
		return &outSet;
	}

	return nullptr;
}

template<typename SocketType>
void SocketUtil::FillVectorFromSet(std::vector<SocketType>* outSockets, const std::vector<SocketType>* inSockets,
	const fd_set& inSet)
{
	if (inSockets && outSockets)
	{
		outSockets->clear();
		for (const SocketType& socket : *inSockets)
		{
			if (FD_ISSET(socket->mSocket, &inSet))
			{
				outSockets->push_back(socket);
			}
		}
	}
}

template<typename SocketType>
int SocketUtil::Select(const std::vector<SocketType>* inReadSet, std::vector<SocketType>* outReadSet,
	const std::vector<SocketType>* inWriteSet, std::vector<SocketType>* outWriteSet,
	const std::vector<SocketType>* inExceptSet, std::vector<SocketType>* outExceptSet,
	const timeval* timeout)
{
	fd_set read, write, except;

	fd_set* readPtr = FillSetFromVector(read, inReadSet);
	fd_set* writePtr = FillSetFromVector(write, inWriteSet);
	fd_set* exceptPtr = FillSetFromVector(except, inExceptSet);

	int toRet = select(0, readPtr, writePtr, exceptPtr, timeout);
	if (toRet > 0)
	{
		FillVectorFromSet(outReadSet, inReadSet, read);
		FillVectorFromSet(outWriteSet, inWriteSet, write);
		FillVectorFromSet(outExceptSet, inExceptSet, except);
	}

	return toRet;
}

template<typename SocketType>
inline int SocketUtil::GetSocketAddress(const SocketType* socket, SocketAddressPtr& outAddress)
{
	sockaddr_in addr;
	socklen_t len = sizeof(sockaddr_in);
	if (0 != getpeername(socket->mSocket, (sockaddr*)&addr, &len))
	{
		ReportError(L"SocketUtil::GetSocketAddress");
		return -GetLastError();
	}

	outAddress = std::make_shared<SocketAddress>(reinterpret_cast<const sockaddr&>(addr));
	return 0;
}
