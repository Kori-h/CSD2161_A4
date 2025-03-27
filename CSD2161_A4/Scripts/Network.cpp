#include "Network.h"

// Define
NetworkType networkType = NetworkType::UNINITIALISED;
sockaddr_in serverAddress;
uint16_t port;
SOCKET udpSocket;

void AttachConsoleWindow()
{
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONIN$", "r", stdin);
}

void FreeConsoleWindow()
{
	FreeConsole();
}

int InitialiseNetwork()
{
	std::string networkTypeString;
	std::cout << "Network Type (S for Server | C for Client | Default for Single Player): ";
	std::getline(std::cin, networkTypeString);

	if (networkTypeString == "S")
	{
		networkType = NetworkType::SERVER;
		std::cout << "Initialising as Server..." << std::endl;
		return StartServer();
	}
	else if (networkTypeString == "C") 
	{
		networkType = NetworkType::CLIENT;
		std::cout << "Initialising as Client..." << std::endl;
		return ConnectToServer();
	}
	else
	{
		networkType = NetworkType::UNINITIALISED;
		std::cout << "Initialising as Single Player..." << std::endl;
		return 0;
	}
	
	return ERROR_CODE;
}

int StartServer()
{
	// Get UDP Port Number
	std::string portString;
	std::cout << "Server UDP Port Number: ";
	std::getline(std::cin, portString);
	port = static_cast<uint16_t>(std::stoi(portString));

	// Setup WSA data
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(WINSOCK_VERSION, WINSOCK_SUBVERSION), &wsaData) != 0)
	{
		std::cerr << "WSAStartup() failed." << std::endl;
		return ERROR_CODE;
	}

	// Create a UDP Socket
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET)
	{
		std::cerr << "socket() failed." << std::endl;
		WSACleanup();
		return ERROR_CODE;
	}

	// Initialise server address structure
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;  // Bind to all available interfaces

	// Bind the Socket to the port
	if (bind(udpSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		std::cerr << "bind() failed with error code " << WSAGetLastError() << std::endl;
		closesocket(udpSocket);
		WSACleanup();
		return ERROR_CODE;
	}

	// Get local IP address
	char ipAddressBuffer[INET_ADDRSTRLEN];
	struct sockaddr_in localAddress;
	socklen_t addrLength = sizeof(localAddress);
	if (getsockname(udpSocket, (sockaddr*)&localAddress, &addrLength) == SOCKET_ERROR)
	{
		std::cerr << "getsockname() failed with error code " << WSAGetLastError() << std::endl;
		closesocket(udpSocket);
		WSACleanup();
		return ERROR_CODE;
	}

	if (inet_ntop(AF_INET, &localAddress.sin_addr, ipAddressBuffer, INET_ADDRSTRLEN) == nullptr)
	{
		std::cerr << "inet_ntop() failed." << std::endl;
		closesocket(udpSocket);
		WSACleanup();
		return ERROR_CODE;
	}

	std::cout << "Server established at " << ipAddressBuffer << ":" << port << std::endl;

	return 0;
}

int ConnectToServer()
{
	// Get IP Address
	std::string serverIPAddress{};
	std::cout << "Server IP Address: ";
	std::getline(std::cin, serverIPAddress);

	// Get UDP Server Port Number
	std::string serverPortString;
	std::cout << "Server UDP Port Number: ";
	std::getline(std::cin, serverPortString);
	uint16_t serverPort = static_cast<uint16_t>(std::stoi(serverPortString));

	// Get UDP Client Port Number
	std::string portString;
	std::cout << "Client UDP Port Number: ";
	std::getline(std::cin, portString);
	port = static_cast<uint16_t>(std::stoi(portString));

	// Setup WSA data
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(WINSOCK_VERSION, WINSOCK_SUBVERSION), &wsaData) != 0) 
	{
		std::cerr << "WSAStartup() failed." << std::endl;
		return ERROR_CODE;
	}

	// Resolve server address using getaddrinfo
	addrinfo addressHints{};
	SecureZeroMemory(&addressHints, sizeof(addressHints));
	addressHints.ai_family = AF_INET;        // IPv4
	addressHints.ai_socktype = SOCK_DGRAM;   // Datagram (unreliable)
	addressHints.ai_protocol = IPPROTO_UDP;  // UDP

	addrinfo* addressInfo = nullptr;
	if (getaddrinfo(serverIPAddress.c_str(), serverPortString.c_str(), &addressHints, &addressInfo) != 0)
	{
		std::cerr << "getaddrinfo() failed." << std::endl;
		WSACleanup();
		return ERROR_CODE;
	}

	// Create a UDP Socket
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET)
	{
		std::cerr << "socket() failed." << std::endl;
		WSACleanup();
		return ERROR_CODE;
	}

	// Initialise client address structure
	sockaddr_in clientAddress;
	memset(&clientAddress, 0, sizeof(clientAddress));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_port = htons(port);
	clientAddress.sin_addr.s_addr = INADDR_ANY;

	// Bind the Socket to the port
	if (bind(udpSocket, (sockaddr*)&clientAddress, sizeof(clientAddress)) == SOCKET_ERROR)
	{
		std::cerr << "bind() failed with error code " << WSAGetLastError() << std::endl;
		closesocket(udpSocket);
		WSACleanup();
		return ERROR_CODE;
	}

	// Initialize server address structure
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	serverAddress.sin_addr.s_addr = ((struct sockaddr_in*)addressInfo->ai_addr)->sin_addr.s_addr;

	// Connection successful
	std::cout << "Connected to server at " << serverIPAddress << ":" << serverPort << std::endl;
	freeaddrinfo(addressInfo);

	return 0;
}

void Disconnect()
{
	shutdown(udpSocket, SD_SEND);
	closesocket(udpSocket);
	WSACleanup();
}

void SendPacket(SOCKET socket, sockaddr_in address, NetworkPacket packet)
{
	int sentBytes = sendto(socket, (char*)&packet, sizeof(packet), 0, (sockaddr*)&address, sizeof(address));

	if (sentBytes == SOCKET_ERROR)
	{
		std::cerr << "Failed to send game data. Error: " << WSAGetLastError() << std::endl;
	}
}

NetworkPacket ReceivePacket(SOCKET socket, sockaddr_in address)
{
	NetworkPacket packet;

    int addressSize = sizeof(address);
    int receivedBytes = recvfrom(socket, (char*)&packet, sizeof(packet), 0, (sockaddr*)&address, &addressSize);

	if (receivedBytes == SOCKET_ERROR)
	{
		std::cerr << "Failed to receive data. Error: " << WSAGetLastError() << std::endl;
	}

	return packet;
}

void SendJoinRequest(SOCKET socket, sockaddr_in address) 
{
	NetworkPacket packet;
	packet.packetID = PacketID::JOIN_REQUEST;
	packet.sourcePortNumber = port;
	packet.destinationPortNumber = address.sin_port;
	SendPacket(socket, address, packet);
}

void HandleJoinRequest(SOCKET socket, sockaddr_in address, NetworkPacket packet)
{
	if (packet.packetID == PacketID::JOIN_REQUEST) 
	{
		std::cout << "Player [" << packet.sourcePortNumber << "] is joining the lobby." << std::endl;

		NetworkPacket responsePacket;
		responsePacket.packetID = PacketID::REQUEST_ACCEPTED;
		responsePacket.sourcePortNumber = port;
		responsePacket.destinationPortNumber = packet.sourcePortNumber;
		SendPacket(socket, address, responsePacket);
	}
}

void SendInput(SOCKET socket, sockaddr_in address) 
{
	NetworkPacket packet;
	packet.packetID = PacketID::GAME_INPUT;
	packet.sourcePortNumber = port;
	packet.destinationPortNumber = address.sin_port;
	strcpy_s(packet.data, "[PLAYER_INPUT_DATA]");
	SendPacket(socket, address, packet);
}

void HandlePlayerInput(SOCKET socket, sockaddr_in address, NetworkPacket packet)
{
	if (packet.packetID == PacketID::GAME_INPUT)
	{
		std::cout << "Received input from Player [" << packet.sourcePortNumber << "]: " << packet.data << std::endl;

		NetworkPacket responsePacket;
		responsePacket.packetID = PacketID::GAME_STATE_UPDATE;
		responsePacket.sourcePortNumber = port;
		responsePacket.destinationPortNumber = packet.sourcePortNumber;
		strcpy_s(responsePacket.data, "[GAME_STATE_DATA]");

		SendPacket(socket, address, responsePacket);
	}
}

void ReceiveGameStateStart(SOCKET socket) 
{
	sockaddr_in address{};
	NetworkPacket packet = ReceivePacket(socket, address);

	if (packet.packetID == PacketID::GAME_STATE_START)
	{
		std::cout << "Game started. Initial game state: " << packet.data << std::endl;
	}
}