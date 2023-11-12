#include "ChatClient.h"

#include "ChatUtility.h"

#include <iostream>

void ChatClient::StartClientLoop(std::ostream& logOut)
{
	if (!mSocket) return;

	// Set the socket to non-blocking mode.
	// It means that the socket won't wait for data to be received while calling the Receive() function.
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

		// Got some data to send to a server.
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

	handleMessagesThread.join(); // Just waiting for the thread to finish.
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
		if (bytesReceived > 0) // Received some data.
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
		else if (bytesReceived == 0) // Server sent a FIN flag. So the connection is closed.
		{
			logOut << "\nServer disconnected...\n"
					"Enter empty line to exit.\n"
					">: ";
			bIsClientConnected = false;
			break;
		}
		else
		{
			// Most of the time we will come here getting this error. It's normal, just ignore it.
			// The reason of it is that we set the socket to non-blocking mode and after getting no data in the incoming buffer we just stopped waiting for it.
			if (-bytesReceived == WSAEWOULDBLOCK) continue;

			// Okay, there was some error for sure.
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
