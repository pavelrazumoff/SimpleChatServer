#include "SocketUtil.h"

UDPSocketPtr SocketUtil::CreateUDPSocket(SocketAddressFamily inFamily)
{
	SOCKET s = socket(inFamily, SOCK_DGRAM, IPPROTO_UDP);
	if (s != INVALID_SOCKET)
		return UDPSocketPtr(new UDPSocket(s));

	ReportError(L"SocketUtil::CreateUDPSocket");
	return nullptr;
}

TCPSocketPtr SocketUtil::CreateTCPSocket(SocketAddressFamily inFamily)
{
	SOCKET s = socket(inFamily, SOCK_STREAM, IPPROTO_TCP);
	if (s != INVALID_SOCKET)
		return TCPSocketPtr(new TCPSocket(s));

	ReportError(L"SocketUtil::CreateTCPSocket");
	return nullptr;
}

void SocketUtil::ReportError(const wchar_t* inOperationDesc)
{
	#if _WIN32
	LPVOID lpMsgBuf;
	DWORD errorNum = GetLastError();
	
	FormatMessage(
				  FORMAT_MESSAGE_ALLOCATE_BUFFER |
				  FORMAT_MESSAGE_FROM_SYSTEM |
				  FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL,
				  errorNum,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				  (LPTSTR) &lpMsgBuf,
				  0, NULL );
	
	
	//LOG( "Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf );
	#else
	//LOG( "Error: %hs", inOperationDesc );
	#endif
}

int SocketUtil::GetLastError()
{
	#if _WIN32
	return WSAGetLastError();
	#else // _WIN32
	return errno;
	#endif // _WIN32
}
