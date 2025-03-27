#ifndef NETWORK
#define NETWORK // header guards

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Windows.h"		// Entire Win32 API...
// #include "winsock2.h"	// ...or Winsock alone
#include "ws2tcpip.h"		// getaddrinfo()

// Tell the Visual Studio linker to include the following library in linking.
// Alternatively, we could add this file to the linker command-line parameters,
// but including it in the source code simplifies the configuration.
#pragma comment(lib, "ws2_32.lib")

#include <iostream>			// cout, cerr
#include <string>			// string
#include <optional>			// optional
#include <filesystem>		// file system
#include <fstream>			// file stream
#include <unordered_map>	// unordered_map
#include <mutex>			// mutex

#undef WINSOCK_VERSION		// fix for macro redefinition
#define WINSOCK_VERSION     2
#define WINSOCK_SUBVERSION  2
#define MAX_STR_LEN         1000
#define RETURN_CODE_1       1
#define RETURN_CODE_2       2
#define RETURN_CODE_3       3
#define RETURN_CODE_4       4

#define MAX_WORKER_COUNT	10
#define MAX_QUEUE_SLOTS		20

#define DEFAULT_BUFLEN		4096

#define TIMEOUT_MS		    1000

enum class NetworkType 
{
    UNINITIALISED,
    CLIENT,
    SERVER
};

extern NetworkType networkType;

int InitialiseNetwork();

int StartServer();

int ConnectToServer();

#endif