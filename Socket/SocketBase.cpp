#include "SocketBase.h"

#include "SocketUtil.h"

SocketBase::~SocketBase()
{
	closesocket(mSocket);
}

int SocketBase::Bind(const SocketAddress& inBindAddress)
{
	int err = bind(mSocket, &inBindAddress.mSockAddr, (int)inBindAddress.GetSize());
	if (err != 0)
	{
		SocketUtil::ReportError(L"SocketBase::Bind");
		return SocketUtil::GetLastError();
	}

	return NO_ERROR;
}

int SocketBase::Shutdown(int how)
{
	return shutdown(mSocket, how);
}

int SocketBase::SetNonBlockingMode(bool bShouldBeNonBlocking)
{
	#if _WIN32
	u_long arg = bShouldBeNonBlocking ? 1 : 0;
	int result = ioctlsocket(mSocket, FIONBIO, &arg);

	#else // _WIN32
	int flags = fcntl(mSocket, F_GETFL, 0);
	flags bShouldBeNonBlocking ?
		(flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);

	int result = fcntl(mSocket, F_SETFL, flags);
	#endif // _WIN32

	if (result == SOCKET_ERROR)
	{
		SocketUtil::ReportError(L"SocketBase::SetNonBlockingMode");
		return SocketUtil::GetLastError();
	}

	return NO_ERROR;
}
