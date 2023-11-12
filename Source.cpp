#include "Socket/SocketFactory.h"
#include "Socket/SocketUtil.h"

#include "ChatServer.h"
#include "ChatClient.h"

#include <iostream>
#include <thread>

using namespace std;

TCPSocketPtr MakeListenServer(int& errCode);
TCPSocketPtr ConnectToServer(int& errCode);

int main()
{
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err)
	{
		cerr << "WSAStartup failed.\n";
		return err;
	}

	TCPSocketPtr socket;
	bool bIsHost = false;

	bool bPrintMenu = true;

	do {

		if (bPrintMenu)
		{
			cout << "\nWelcome to the Chat Server.\n\n"
				"The options you have:\n"
				"\ta) Create a Host-Server to chat with your friends.\n"
				"\tb) Connect to a remote server and chat with the Host.\n"
				"\tc) Exit.\n\n"
				"\t>: ";
		}

		bPrintMenu = false;

		char c;
		cin >> c;

		cin.clear(); cin.ignore(LLONG_MAX, '\n');

		int errCode = NO_ERROR;
		switch (c)
		{
			case 'a':
				socket = MakeListenServer(errCode);
				if (errCode != NO_ERROR) cout << "Failed to create a Host-Server. Error code: " << errCode << "\n";
				else bIsHost = true;
				break;
			case 'b':
				socket = ConnectToServer(errCode);
				if (errCode != NO_ERROR) cout << "Failed to connect to a remote server. Error code: " << errCode << "\n";
				break;
			case 'c':
				break;
			default:
				cout << "Wrong anwser. Try again...\n";
				continue;
		}

		if (socket)
		{
			if (bIsHost)
			{
				ChatServer server(socket);
				server.StartServerLoop(cout);
			}
			else
			{
				ChatClient client(socket);
				client.StartClientLoop(cout);
			}

			socket.reset();
			bPrintMenu = true;
		}
		else
		{
			if (errCode == NO_ERROR) break;
			else bPrintMenu = true;
		}

	} while (true);

	WSACleanup();

	return 0;
}

TCPSocketPtr MakeListenServer(int& errCode)
{
	TCPSocketPtr listenSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);
	SocketAddress listenAddress = SocketAddress(INADDR_ANY, 48000);

	errCode = listenSocket->Bind(listenAddress);
	if (NO_ERROR != errCode) return nullptr;

	errCode = listenSocket->Listen();
	if (NO_ERROR != errCode) return nullptr;

	return listenSocket;
}

TCPSocketPtr ConnectToServer(int& errCode)
{
	const string defAddress = "127.0.0.1:48000";
	string address;

	cout << "Enter server address to connect to (empty string to use localhost): ";

	getline(cin, address);

	if (address.empty()) address = defAddress;

	SocketAddressPtr serverAddress = SocketAddressFactory::CreateIPv4FromString(address);
	TCPSocketPtr clientSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);

	errCode = clientSocket->Connect(*serverAddress);
	if (NO_ERROR != errCode) return nullptr;

	return clientSocket;
}
