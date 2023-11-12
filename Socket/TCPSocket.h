#pragma once

#include "SocketBase.h"

class TCPSocket : public SocketBase
{
public:
	int Connect(const SocketAddress& inAddress);

	int Listen(int inBackLog = 32);
	std::shared_ptr<TCPSocket> Accept(SocketAddress& inFromAddress);

	int Send(const void* inData, int inLen);
	int Receive(void* inData, int inLen);

private:
	friend class SocketUtil;

	TCPSocket(SOCKET inSocket) : SocketBase(inSocket) {}
};

typedef std::shared_ptr<TCPSocket> TCPSocketPtr;
