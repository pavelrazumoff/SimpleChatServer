#include "ChatServer.h"

#include "Socket/SocketUtil.h"
#include "ChatUtility.h"

#include <iostream>
#include <string>

void ChatServer::StartServerLoop(std::ostream& logOut)
{
	if (!mSocket) return;

	ConnectedClients.clear();

	bIsServerRunning = true;

	std::thread handleMessagesThread(&ChatServer::ServerReadMessagesThread, this, std::ref(logOut));

	do
	{
		std::unique_lock<std::shared_mutex> lock(sharedMtx);
		{
			logOut << "\nEnter \'stop\' to terminate the Server.\n"
				">: ";

			lock.unlock();
		}

		std::string input;
		std::getline(std::cin, input);

		if (input == "stop")
		{
			logOut << "\nServer is shutting down...\n";
			break;
		}

	} while (true);

	bIsServerRunning = false;

	handleMessagesThread.join();
	mSocket->Shutdown();
}

void ChatServer::ServerReadMessagesThread(std::ostream& logOut)
{
	using namespace std;

	vector<TCPSocketPtr> readBlockSockets;
	readBlockSockets.push_back(mSocket);

	vector<TCPSocketPtr> readableSockets;

	vector<TCPSocketPtr> writeBlockSockets;
	vector<TCPSocketPtr> writableSockets;

	timeval timeout;
	timeout.tv_sec = 5;  // 5 seconds
	timeout.tv_usec = 0;

	while (bIsServerRunning)
	{
		if (SocketUtil::Select<TCPSocketPtr>(&readBlockSockets, &readableSockets,
			&writeBlockSockets, &writableSockets,
			nullptr, nullptr,
			&timeout))
		{
			std::unique_lock<std::shared_mutex> lock(sharedMtx);
			bool bPrintInvitationSign = false;

			for (const auto& socket : readableSockets)
			{
				if (socket == mSocket)
				{
					SocketAddress newClientAddress;
					auto newSocket = socket->Accept(newClientAddress);
					readBlockSockets.push_back(newSocket);

					ProcessNewClient(newSocket, newClientAddress, logOut);
					CollectSocketsFromConnectedClients(writeBlockSockets);

					bPrintInvitationSign = true;
				}
				else
				{
					bPrintInvitationSign = true;

					char segment[1024];
					int dataReceived = socket->Receive(segment, 1024);
					if (dataReceived > 0)
					{
						// TODO: Manage to detect clients, that are not writable for now to send them this message later.
						ProcessClientMessage(socket, segment, dataReceived, writableSockets, logOut);
					}
					else if (dataReceived == 0)
					{
						ProcessClientDisconnected(socket, logOut);
						CollectSocketsFromConnectedClients(writeBlockSockets);

						readBlockSockets.erase(find(readBlockSockets.begin(), readBlockSockets.end(), socket));
					}
					else
					{
						logOut << "\nError " << dataReceived << " receiving message...\n";
					}
				}
			}

			for (const auto& socket : writableSockets)
			{
				int res = SyncChatMessagesWithClient(socket, logOut);
				if (res != 0) bPrintInvitationSign = true;
			}

			if (bPrintInvitationSign) logOut << ">: ";
		}
	}
}

void ChatServer::ProcessNewClient(TCPSocketPtr newClientSocket, const SocketAddress& newClientAddress, std::ostream& logOut)
{
	ConnectedClients.push_back(ClientInfo(newClientSocket));

	logOut << "\nNew client connected from " << newClientAddress.ToString() << "\n\n";
}

void ChatServer::ProcessClientMessage(TCPSocketPtr ClientSocket, const char* data, int dataLen,
	const std::vector<TCPSocketPtr>& availableClients, std::ostream& logOut)
{
	SocketAddressPtr clientAddress;
	if (0 != GetSocketAddress(ClientSocket.get(), clientAddress, logOut)) return;

	if (IsClientSynced(ClientSocket))
	{
		auto foundClient = FindClientInfoBySocket(ClientSocket);
		if (foundClient != ConnectedClients.end())
		{
			foundClient->lastSyncMessageIndex = (int)ChatMessages.size();
		}
	}

	ChatMessages.push_back(ClientMessage(std::string(data, dataLen).c_str(), clientAddress));

	std::string clientAddressStr = clientAddress->ToString();
	logOut << "\nMessage received from client (" << clientAddressStr << "): " << data << "\n";
}

void ChatServer::ProcessClientDisconnected(TCPSocketPtr ClientSocket, std::ostream& logOut)
{
	SocketAddressPtr clientAddress;
	if (0 == GetSocketAddress(ClientSocket.get(), clientAddress, logOut))
	{
		logOut << "\nClient " << clientAddress->ToString() << " disconnected.\n\n";
	}

	auto foundClient = FindClientInfoBySocket(ClientSocket);
	if (foundClient != ConnectedClients.end())
	{
		ConnectedClients.erase(foundClient);
	}

	ClientSocket->Shutdown();
}

int ChatServer::SyncChatMessagesWithClient(TCPSocketPtr ClientSocket, std::ostream& logOut)
{
	auto foundClient = FindClientInfoBySocket(ClientSocket);
	if (foundClient == ConnectedClients.end())
	{
		logOut << "\nFailed to find client's socket...\n";
		return -1;
	}

	int& lastSyncMessageIndex = foundClient->lastSyncMessageIndex;
	const int actualMessageIndex = (int)ChatMessages.size() - 1;

	if (lastSyncMessageIndex == actualMessageIndex) return 0; // Already up to date.

	SocketAddressPtr clientAddress;
	if (0 != GetSocketAddress(ClientSocket.get(), clientAddress))
	{
		logOut << "\nFailed to sync client...\n";
		return -1;
	}

	int nextMessageIndex = lastSyncMessageIndex + 1;
	
	// Same sender. Skip this message.
	const auto& messageInfo = ChatMessages[nextMessageIndex];
	if (clientAddress == messageInfo.senderAddress) return 0;

	const std::string senderAddressStr = messageInfo.senderAddress->ToString();
	const std::string clientAddressStr = clientAddress->ToString();

	std::string message;
	message.append(senderAddressStr); message.append(": ");
	message.append(messageInfo.message);

	ChatSyncData syncData;
	strcpy_s(syncData.message, message.c_str());
	syncData.bFinalMessageInQueue = (nextMessageIndex == actualMessageIndex);

	int retResult = ClientSocket->Send(reinterpret_cast<const void*>(&syncData), sizeof(syncData));
	if (retResult < 0)
	{
		logOut << "\nError " << retResult << " sending message...\n";
		return -1;
	}
	else logOut << retResult << " bytes sent to client " << clientAddressStr << "\n";

	if (++lastSyncMessageIndex == actualMessageIndex)
		logOut << "Made sync of all messages for client " << clientAddressStr << "\n";

	return 1;
}

void ChatServer::CollectSocketsFromConnectedClients(std::vector<TCPSocketPtr>& clientSockets)
{
	clientSockets.clear();

	clientSockets.reserve(ConnectedClients.size());
	for (const auto& client : ConnectedClients)
		clientSockets.push_back(client.clientSocket);
}

std::vector<ClientInfo>::iterator ChatServer::FindClientInfoBySocket(TCPSocketPtr socket)
{
	return std::find_if(ConnectedClients.begin(), ConnectedClients.end(), [&](const ClientInfo& clientInfo)
		{
			return clientInfo.clientSocket.get() == socket.get();
		});
}

const std::vector<ClientInfo>::const_iterator ChatServer::FindClientInfoBySocket(TCPSocketPtr socket) const
{
	return std::find_if(ConnectedClients.begin(), ConnectedClients.end(), [&](const ClientInfo& clientInfo)
		{
			return clientInfo.clientSocket.get() == socket.get();
		});
}

bool ChatServer::IsClientSynced(TCPSocketPtr ClientSocket) const
{
	auto foundClient = FindClientInfoBySocket(ClientSocket);
	if (foundClient != ConnectedClients.end())
		return foundClient->lastSyncMessageIndex == (int)ChatMessages.size() - 1;
	
	return false;
}
