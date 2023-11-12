#pragma once

#include "SocketAddress.h"

class SocketBase
{
public:
	virtual ~SocketBase();

	int Bind(const SocketAddress& inBindAddress);
	int Shutdown(int how = SD_BOTH);

	int SetNonBlockingMode(bool bShouldBeNonBlocking);

protected:
	SocketBase(SOCKET inSocket) : mSocket(inSocket) {}

protected:
	SOCKET mSocket;
};
