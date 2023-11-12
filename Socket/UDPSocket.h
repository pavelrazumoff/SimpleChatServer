#pragma once

#include "SocketBase.h"

class UDPSocket : public SocketBase
{
public:
	int SendTo(const void* inData, int inLen, const SocketAddress& inTo);
	int ReceiveFrom(void* inBuffer, int inLen, SocketAddress& outFrom);

private:
	friend class SocketUtil;

	UDPSocket(SOCKET inSocket) : SocketBase(inSocket) {}
};

typedef std::shared_ptr<UDPSocket> UDPSocketPtr;
