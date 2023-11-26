#pragma once

#include "ChatSocketHandler.h"
#include "ChatUtility.h"

#include <shared_mutex>

class InputMemoryStream;

class ChatClient : public ChatSocketHandler
{
public:
	ChatClient(TCPSocketPtr _socket);
	~ChatClient();

	void StartClientLoop(std::ostream& logOut);

protected:
	void ClientReceiveMessagesThread(std::ostream& logOut);

	#if USE_REFLECTION_SYSTEM_DATA()
	bool HandleReceivedSegment(ChatObject& segmentData, std::ostream& logOut);
	#else // USE_REFLECTION_SYSTEM_DATA()
	bool HandleReceivedSegment(ChatSyncData& segmentData, std::ostream& logOut);
	#endif // USE_REFLECTION_SYSTEM_DATA()
	bool ProcessServerError(int errorCode, std::ostream& logOut);

protected:
	static const uint32_t MAX_PACKET_SIZE = 1470;

private:
	std::atomic<bool> bIsClientConnected = false;
	std::shared_mutex sharedMtx;

	InputMemoryStream* mInputStream = nullptr;
};
