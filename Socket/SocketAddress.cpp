#include "SocketAddress.h"

#include "WS2tcpip.h"

std::string SocketAddress::ToString() const
{
	char destinationBuffer[INET_ADDRSTRLEN];

	// TODO: Make cached.
	inet_ntop(AF_INET, &GetAsSockAddrIn()->sin_addr, destinationBuffer, INET_ADDRSTRLEN);
	return std::string(destinationBuffer) + ":" + std::to_string(ntohs(GetAsSockAddrIn()->sin_port));
}

bool SocketAddress::operator==(const SocketAddress& other) const
{
	return !ToString().compare(other.ToString());
}
