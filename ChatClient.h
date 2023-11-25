#pragma once

#include "ChatSocketHandler.h"
#include <shared_mutex>

struct ChatSyncData;
class InputMemoryStream;

class ChatClient : public ChatSocketHandler
{
public:
	ChatClient(TCPSocketPtr _socket);
	~ChatClient();

	void StartClientLoop(std::ostream& logOut);

protected:
	void ClientReceiveMessagesThread(std::ostream& logOut);

	bool HandleReceivedSegment(ChatSyncData& segmentData, int bytesReceived, std::ostream& logOut);
	bool ProcessServerError(int errorCode, std::ostream& logOut);

protected:
	static const uint32_t MAX_PACKET_SIZE = 1470;

private:
	std::atomic<bool> bIsClientConnected = false;
	std::shared_mutex sharedMtx;

	InputMemoryStream* mInputStream = nullptr;
};
