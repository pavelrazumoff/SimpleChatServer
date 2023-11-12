#include "ChatSocketHandler.h"

#include "Socket/SocketUtil.h"

#include <ostream>

int ChatSocketHandler::GetSocketAddress(const TCPSocket* Socket, SocketAddressPtr& outAddress)
{
	return SocketUtil::GetSocketAddress<TCPSocket>(Socket, outAddress);
}

int ChatSocketHandler::GetSocketAddress(const TCPSocket* Socket, SocketAddressPtr& outAddress, std::ostream& logOut)
{
	int retRes = SocketUtil::GetSocketAddress<TCPSocket>(Socket, outAddress);
	if (retRes != 0)
	{
		logOut << "\nError " << retRes << " getting client address...\n";
		return retRes;
	}

	return 0;
}
