#include "ChatClient.h"

#include "ChatUtility.h"

#include <iostream>

void ChatClient::StartClientLoop(std::ostream& logOut)
{
	if (!mSocket) return;

	if (NO_ERROR != mSocket->SetNonBlockingMode(true))
	{
		logOut << "\nError setting client socket to non-blocking mode...\n";
		return;
	}

	logOut << "Enter message to send (empty line to exit): \n";

	bIsClientConnected = true;
	std::thread handleMessagesThread(&ChatClient::ClientReceiveMessagesThread, this, std::ref(logOut));

	do
	{
		std::unique_lock<std::shared_mutex> lock(sharedMtx);
		{
			logOut << ">: ";
			lock.unlock();
		}

		std::string message;
		std::getline(std::cin, message);

		if (message.empty() || !bIsClientConnected) break;

		lock.lock();
		{
			int retResult = mSocket->Send(message.c_str(), (int)message.size() + 1);
			if (retResult < 0)
			{
				logOut << "\nError " << retResult << " sending message...\n";
				continue;
			}
		}

	} while (bIsClientConnected);

	if (bIsClientConnected) bIsClientConnected = false;

	handleMessagesThread.join();
	mSocket->Shutdown();
}

void ChatClient::ClientReceiveMessagesThread(std::ostream& logOut)
{
	bool bWaitingForFinalMessage = false;

	while (bIsClientConnected)
	{
		std::unique_lock<std::shared_mutex> lock(sharedMtx);

		ChatSyncData segmentData;
		int bytesReceived = mSocket->Receive(reinterpret_cast<void*>(&segmentData), sizeof(segmentData));
		if (bytesReceived > 0)
		{
			if (!bWaitingForFinalMessage) logOut << "\n";

			HandleReceivedSegment(segmentData, bytesReceived, logOut);
			if (segmentData.bFinalMessageInQueue)
			{
				bWaitingForFinalMessage = false;
				logOut << ">: ";
			}
			else bWaitingForFinalMessage = true;
		}
		else if (bytesReceived == 0)
		{
			// TODO: Somehow stop the main thread from waiting for input.
			logOut << "\nServer disconnected...\n"
					"Enter empty line to exit.\n"
					">: ";
			bIsClientConnected = false;
			break;
		}
		else
		{
			if (-bytesReceived == WSAEWOULDBLOCK) continue;

			logOut << "\nError " << -bytesReceived << " receiving message...\n";
		}
	}
}

void ChatClient::HandleReceivedSegment(const ChatSyncData& segmentData, int bytesReceived, std::ostream& logOut)
{
	size_t expectSegmentSize = sizeof(segmentData);
	if (expectSegmentSize <= bytesReceived)
	{
		logOut << segmentData.message << "\n";
	}
	else
	{
		logOut << "Received " << bytesReceived << " bytes, but expected " << sizeof(segmentData) << " bytes...\n";
	}
}
