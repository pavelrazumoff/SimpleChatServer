#pragma once

#include "ChatSocketHandler.h"
#include <shared_mutex>

class ChatClient : public ChatSocketHandler
{
public:
	ChatClient(TCPSocketPtr _socket) : ChatSocketHandler(_socket) {}

	void StartClientLoop(std::ostream& logOut);

protected:
	void ClientReceiveMessagesThread(std::ostream& logOut);

	void HandleReceivedSegment(const struct ChatSyncData& segmentData, int bytesReceived, std::ostream& logOut);

private:
	std::atomic<bool> bIsClientConnected = false;
	std::shared_mutex sharedMtx;
};
