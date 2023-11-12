#include "UDPSocket.h"
#include "SocketUtil.h"

int UDPSocket::SendTo(const void* inData, int inLen, const SocketAddress& inTo)
{
	int byteSentCount = sendto(mSocket, static_cast<const char*>(inData),
		inLen, 0, &inTo.mSockAddr, (int)inTo.GetSize());

	if (byteSentCount >= 0) return byteSentCount;
	
	SocketUtil::ReportError(L"UDPSocket::SendTo");
	return -SocketUtil::GetLastError();
}

int UDPSocket::ReceiveFrom(void* inBuffer, int inLen, SocketAddress& outFrom)
{
	int fromLength = (int)outFrom.GetSize();
	int readByteCount = recvfrom(mSocket, static_cast<char*>(inBuffer),
		inLen, 0, &outFrom.mSockAddr, &fromLength);

	if (readByteCount >= 0) return readByteCount;

	SocketUtil::ReportError(L"UDPSocket::ReceiveFrom");
	return -SocketUtil::GetLastError();
}
