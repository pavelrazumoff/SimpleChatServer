#pragma once

#include "Socket/TCPSocket.h"

class ChatSocketHandler
{
public:
	ChatSocketHandler(TCPSocketPtr _socket) : mSocket(_socket) {}
	virtual ~ChatSocketHandler() {}

	int GetSocketAddress(const TCPSocket* Socket, SocketAddressPtr& outAddress);
	int GetSocketAddress(const TCPSocket* Socket, SocketAddressPtr& outAddress, std::ostream& logOut);

protected:
	TCPSocketPtr mSocket;
};
