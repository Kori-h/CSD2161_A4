#include "Network.h"

NetworkType networkType = NetworkType::UNINITIALISED;
uint16_t udpPort;
SOCKET udpSocket;

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
		std::cout << "Initialising Single Player..." << std::endl;
		return 0;
	}
	
	return -1;
}

int StartServer()
{
	// Get UDP Port Number
	std::string udpPortNumber;
	std::cout << "Server UDP Port Number: ";
	std::getline(std::cin, udpPortNumber);
	std::cout << std::endl;

	// -------------------------------------------------------------------------
	// Start up Winsock, asking for version 2.2.
	//
	// WSAStartup()
	// -------------------------------------------------------------------------

	// This object holds the information about the version of Winsock that we
	// are using, which is not necessarily the version that we requested.
	WSADATA wsaData{};

	// Initialize Winsock. You must call WSACleanup when you are finished.
	// As this function uses a reference counter, for each call to WSAStartup,
	// you must call WSACleanup or suffer memory issues.
	int errorCode = WSAStartup(MAKEWORD(WINSOCK_VERSION, WINSOCK_SUBVERSION), &wsaData);
	if (NO_ERROR != errorCode)
	{
		std::cerr << "WSAStartup() failed." << std::endl;
		return errorCode;
	}

	// -------------------------------------------------------------------------
	// Setup UDP socket
	// -------------------------------------------------------------------------

	addrinfo udpHints{};
	SecureZeroMemory(&udpHints, sizeof(udpHints));
	udpHints.ai_family = AF_INET;        // IPv4
	udpHints.ai_socktype = SOCK_DGRAM;   // Datagram (unreliable)
	udpHints.ai_protocol = IPPROTO_UDP;  // UDP

	addrinfo* udpInfo = nullptr;
	errorCode = getaddrinfo(nullptr, udpPortNumber.c_str(), &udpHints, &udpInfo);
	if ((NO_ERROR != errorCode) || (nullptr == udpInfo))
	{
		std::cerr << "getaddrinfo() failed." << std::endl;
		WSACleanup();
		return errorCode;
	}

	// -------------------------------------------------------------------------
	// Create a UDP socket
	// -------------------------------------------------------------------------

	// socket is globally visible
	udpSocket = socket(udpInfo->ai_family, udpInfo->ai_socktype, udpInfo->ai_protocol);
	if (INVALID_SOCKET == udpSocket)
	{
		std::cerr << "socket() failed." << std::endl;
		freeaddrinfo(udpInfo);
		WSACleanup();
		return RETURN_CODE_2;
	}

	// -------------------------------------------------------------------------
	// Bind the UDP socket
	// -------------------------------------------------------------------------

	// Create a sockaddr_in for binding
	sockaddr_in bindAddr{};
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(udpPort);
	bindAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(udpSocket, (sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
	{
		std::cerr << "bind() failed." << std::endl;
		closesocket(udpSocket);
		freeaddrinfo(udpInfo);
		WSACleanup();
		return RETURN_CODE_3;
	}

	// -------------------------------------------------------------------------
	// Print details
	// -------------------------------------------------------------------------

	/* PRINT SERVER IP ADDRESS AND PORT NUMBER */
	char serverIPAddr[MAX_STR_LEN];
	struct sockaddr_in* serverAddress = reinterpret_cast<struct sockaddr_in*> (udpInfo->ai_addr);
	inet_ntop(AF_INET, &(serverAddress->sin_addr), serverIPAddr, INET_ADDRSTRLEN);
	getnameinfo(udpInfo->ai_addr, static_cast <socklen_t> (udpInfo->ai_addrlen), 
		serverIPAddr, sizeof(serverIPAddr), nullptr, 0, NI_NUMERICHOST);

	std::cout << "Server IP Address: " << serverIPAddr << std::endl;
	std::cout << "Server UDP Port Number: " << udpPortNumber << std::endl;

	// -------------------------------------------------------------------------
	// Server successfully initialised
	// -------------------------------------------------------------------------

	freeaddrinfo(udpInfo);

	return 0;
}

int ConnectToServer()
{
	// Get IP Address
	std::string host{};
	std::cout << "Server IP Address: ";
	std::getline(std::cin, host);
	std::cout << std::endl;

	// Get UDP Server Port Number
	std::string udpServerPortNumber;
	std::cout << "Server UDP Port Number: ";
	std::getline(std::cin, udpServerPortNumber);
	std::cout << std::endl;

	// Get UDP Client Port Number
	std::string udpClientPortNumber;
	std::cout << "Client UDP Port Number: ";
	std::getline(std::cin, udpClientPortNumber);
	std::cout << std::endl;

	// -------------------------------------------------------------------------
	// Start up Winsock, asking for version 2.2
	// -------------------------------------------------------------------------

	// This object holds the information about the version of Winsock that we
	// are using, which is not necessarily the version that we requested.
	WSADATA wsaData{};
	SecureZeroMemory(&wsaData, sizeof(wsaData));

	// Initialize Winsock. You must call WSACleanup when you are finished.
	// As this function uses a reference counter, for each call to WSAStartup,
	// you must call WSACleanup or suffer memory issues.
	int errorCode = WSAStartup(MAKEWORD(WINSOCK_VERSION, WINSOCK_SUBVERSION), &wsaData);
	if (NO_ERROR != errorCode)
	{
		std::cerr << "WSAStartup() failed." << std::endl;
		return errorCode;
	}

	// -------------------------------------------------------------------------
	// Setup UDP socket
	// -------------------------------------------------------------------------

	// Object hints indicate which protocols to use to fill in the info.
	addrinfo udpHints{};
	SecureZeroMemory(&udpHints, sizeof(udpHints));
	udpHints.ai_family = AF_INET;        // IPv4
	udpHints.ai_socktype = SOCK_DGRAM;   // Datagram (unreliable)
	udpHints.ai_protocol = IPPROTO_UDP;  // UDP

	addrinfo* udpInfo = nullptr;
	errorCode = getaddrinfo(host.c_str(), udpClientPortNumber.c_str(), &udpHints, &udpInfo);
	if ((NO_ERROR != errorCode) || (nullptr == udpInfo))
	{
		std::cerr << "getaddrinfo() failed." << std::endl;
		WSACleanup();
		return errorCode;
	}

	// -------------------------------------------------------------------------
	// Create a UDP socket
	// -------------------------------------------------------------------------

	// socket is globally visible
	udpSocket = socket(udpInfo->ai_family, udpInfo->ai_socktype, udpInfo->ai_protocol);
	if (INVALID_SOCKET == udpSocket)
	{
		std::cerr << "socket() failed." << std::endl;
		freeaddrinfo(udpInfo);
		WSACleanup();
		return RETURN_CODE_2;
	}

	// -------------------------------------------------------------------------
	// Bind the UDP socket
	// -------------------------------------------------------------------------

	// Create a sockaddr_in for binding
	sockaddr_in bindAddr{};
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(udpPort);
	bindAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(udpSocket, (sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
	{
		std::cerr << "bind() failed with error code " << WSAGetLastError() << std::endl;
		closesocket(udpSocket);
		freeaddrinfo(udpInfo);
		WSACleanup();
		return RETURN_CODE_3;
	}

	// -------------------------------------------------------------------------
	// Print details
	// -------------------------------------------------------------------------

	/* PRINT CLIENT IP ADDRESS AND PORT NUMBER */
	char clientIPAddr[MAX_STR_LEN];
	struct sockaddr_in* serverAddress = reinterpret_cast<struct sockaddr_in*> (udpInfo->ai_addr);
	inet_ntop(AF_INET, &(serverAddress->sin_addr), clientIPAddr, INET_ADDRSTRLEN);
	getnameinfo(udpInfo->ai_addr, static_cast <socklen_t> (udpInfo->ai_addrlen), 
		clientIPAddr, sizeof(clientIPAddr), nullptr, 0, NI_NUMERICHOST);

	std::cout << "Client IP Address: " << clientIPAddr << std::endl;
	std::cout << "Client UDP Port Number: " << udpClientPortNumber << std::endl;

	// -------------------------------------------------------------------------
	// Client successfully connected with the server
	// -------------------------------------------------------------------------

	freeaddrinfo(udpInfo);

	return 0;
}

void Disconnect()
{
	// -------------------------------------------------------------------------
	// Shutdown
	// -------------------------------------------------------------------------

	// close UDP socket
	int errorCode = shutdown(udpSocket, SD_SEND);
	if (SOCKET_ERROR == errorCode)
	{
		std::cerr << "shutdown() failed." << std::endl;
	}
	closesocket(udpSocket);

	// -------------------------------------------------------------------------
	// Clean-up after Winsock.
	// -------------------------------------------------------------------------

	WSACleanup();
}