#pragma once

#include "WinSock2.h"
#include <memory>
#include <string>

enum SocketAddressFamily
{
	INET = AF_INET,
	INET6 = AF_INET6
};

class SocketAddress
{
public:
	SocketAddress(uint32_t inAddress, uint16_t inPort, SocketAddressFamily family = INET)
	{
		GetAsSockAddrIn()->sin_family = family;
		GetAsSockAddrIn()->sin_addr.S_un.S_addr = htonl(inAddress);
		GetAsSockAddrIn()->sin_port = htons(inPort);
	}
	SocketAddress(const sockaddr& inSockAddr)
	{
		memcpy(&mSockAddr, &inSockAddr, sizeof(sockaddr));
	}
	SocketAddress() { GetAsSockAddrIn()->sin_family = INET; }

	size_t GetSize() const { return sizeof(sockaddr); }

	std::string ToString() const;

	bool operator==(const SocketAddress& other) const;

private:
	sockaddr_in* GetAsSockAddrIn() { return reinterpret_cast<sockaddr_in*>(&mSockAddr); }
	const sockaddr_in* GetAsSockAddrIn() const { return reinterpret_cast<const sockaddr_in*>(&mSockAddr); }

private:
	friend class SocketBase;
	friend class UDPSocket;
	friend class TCPSocket;

	sockaddr mSockAddr;

	std::string savedAddress;
};

typedef std::shared_ptr<SocketAddress> SocketAddressPtr;
