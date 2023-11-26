#include "ChatClient.h"

#include "ChatUtility.h"
#include "Serialization/MemoryStream.h"

#include <iostream>

ChatClient::ChatClient(TCPSocketPtr _socket)
	: ChatSocketHandler(_socket)
	, mInputStream(new InputMemoryStream(MAX_PACKET_SIZE))
{
}

ChatClient::~ChatClient()
{
	if (mInputStream) delete mInputStream;
}

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

		mInputStream->Reset();

		int bytesReceived = mSocket->Receive(mInputStream->GetBufferPtr(), mInputStream->GetCapacity());
		if (bytesReceived > 0) // Received some data.
		{
			if (!bWaitingForFinalMessage) logOut << "\n";

			ChatSyncData segmentData;
			bool bFailed = false;
			while (mInputStream->GetHandledDataSize() < (uint32_t)bytesReceived) // We could receive more than one segment (chat message) at once.
			{
				if (!HandleReceivedSegment(segmentData, logOut))
				{
					bFailed = true;
					break;
				}
			}

			if (!bFailed)
			{
				if (segmentData.bFinalMessageInQueue) // Here we have the last read message to check if it's the last one in the queue.
				{
					bWaitingForFinalMessage = false;
					logOut << ">: ";
				}
				else bWaitingForFinalMessage = true;
			}
			else
			{
				logOut << ">: ";
				bWaitingForFinalMessage = false; // Don't know what to do here. It could be the last or not message in queue. But it'd be better to just not wait for new messages in queue.
			}
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
			int errorCode = -bytesReceived;
			if(!ProcessServerError(errorCode, logOut))
			{
				bIsClientConnected = false;
				break;
			}
		}
	}
}

bool ChatClient::HandleReceivedSegment(ChatSyncData& segmentData, std::ostream& logOut)
{
	if (!segmentData.Serialize(mInputStream))
	{
		logOut << "Error reading segment data...\n";
		return false;
	}

	logOut << segmentData.message << "\n";
	return true;
}

bool ChatClient::ProcessServerError(int errorCode, std::ostream& logOut)
{
	logOut << "\nError " << errorCode << " receiving message...\n";

	if (errorCode == WSAECONNRESET)
	{
		logOut << "Server disconnected...\n"
				"Enter empty line to exit.\n"
				">: ";
		return false;
	}

	return true;
}
