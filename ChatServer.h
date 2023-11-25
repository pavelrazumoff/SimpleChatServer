#pragma once

#include "ChatSocketHandler.h"

#include <vector>
#include <shared_mutex>

struct ClientMessage
{
public:
	ClientMessage(const char* _message, SocketAddressPtr clientAddress)
		: message(_message), senderAddress(clientAddress) {}

public:
	std::string message;
	SocketAddressPtr senderAddress;
};

struct ClientInfo
{
public:
	ClientInfo(TCPSocketPtr socket) : clientSocket(socket) {}

public:
	TCPSocketPtr clientSocket;
	int lastSyncMessageIndex = -1;
};

class ChatServer : public ChatSocketHandler
{
public:
	ChatServer(TCPSocketPtr _socket) : ChatSocketHandler(_socket) {}

	void StartServerLoop(std::ostream& logOut);

protected:
	void ServerReadMessagesThread(std::ostream& logOut);

	void ProcessNewClient(TCPSocketPtr newClientSocket, const SocketAddress& newClientAddress, std::ostream& logOut);
	void ProcessClientMessage(TCPSocketPtr ClientSocket, const char* data, int dataLen,
		const std::vector<TCPSocketPtr>& availableClients, std::ostream& logOut);

	void ProcessClientDisconnected(TCPSocketPtr ClientSocket, std::ostream& logOut);
	bool ProcessClientError(int errorCode, TCPSocketPtr ClientSocket, std::ostream& logOut);

	int SyncChatMessagesWithClient(TCPSocketPtr ClientSocket, std::ostream& logOut);

	void CollectSocketsFromConnectedClients(std::vector<TCPSocketPtr>& clientSockets);

	std::vector<ClientInfo>::iterator FindClientInfoBySocket(TCPSocketPtr socket);
	const std::vector<ClientInfo>::const_iterator FindClientInfoBySocket(TCPSocketPtr socket) const;

	bool IsClientSynced(TCPSocketPtr ClientSocket) const;

protected:
	std::vector<ClientInfo> ConnectedClients;
	std::vector<ClientMessage> ChatMessages;

private:
	std::atomic<bool> bIsServerRunning = false;
	std::shared_mutex sharedMtx;

	HANDLE readInterruptPipe = 0;
	HANDLE writeInterruptPipe = 0;
};
