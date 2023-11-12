#pragma once

#include "SocketAddress.h"
#include "ws2tcpip.h"

class SocketAddressFactory
{
public:
	static SocketAddressPtr CreateIPv4FromString(const std::string& inString, SocketAddressFamily family = INET)
	{
		using namespace std;

		auto pos = inString.find_last_of(':');
		string host, service;

		if (pos != string::npos)
		{
			host = inString.substr(0, pos);
			service = inString.substr(pos + 1);
		}
		else
		{
			host = inString;
			service = "0"; // Using the port by default.
		}

		addrinfo hint;
		memset(&hint, 0, sizeof(hint));
		hint.ai_family = family;

		addrinfo* result;
		int error = getaddrinfo(host.c_str(), service.c_str(),
			&hint, &result);

		if (error != 0 && result != nullptr)
		{
			freeaddrinfo(result);
			return nullptr;
		}

		while (!result->ai_addr && result->ai_next)
		{
			result = result->ai_next;
		}

		if (!result->ai_addr)
		{
			freeaddrinfo(result);
			return nullptr;
		}

		auto toRet = make_shared<SocketAddress>(*result->ai_addr);

		freeaddrinfo(result);
		return toRet;
	}
};
