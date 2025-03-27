/******************************************************************************/
/*!
\file		Main.cpp
\author		
\par		
\date		
\brief		This file contains the 'WinMain' function and is the driver to run the application. It
			initialises the systems and game state manager to run the game loop which includes Input
			Handling, Update and Rendering

Copyright (C) 20xx DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "Network.h"	// networking for multiplayer
#include "Main.h"		// main headers

#include <memory>		// for memory leaks

// ---------------------------------------------------------------------------
// Globals
float	 g_dt;
double	 g_appTime;

/******************************************************************************/
/*!
\brief 
    Main driver function, start of the application. Program execution begins and ends here.

\return int
*/
/******************************************************************************/
int WINAPI WinMain(HINSTANCE instanceH, HINSTANCE prevInstanceH, LPSTR command_line, int show)
{
	// ignore these parameters
	UNREFERENCED_PARAMETER(prevInstanceH);
	UNREFERENCED_PARAMETER(command_line);

	// Enable run-time memory check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif

	// Attach a console for input/output
	AttachConsoleWindow();

	// Initialize network
	if (InitialiseNetwork() != 0)
	{
		std::cerr << "Error Connecting to Network" << std::endl;
		return 0;
	}

	// Game loop
	if (networkType == NetworkType::SERVER)
	{
		std::cout << "Processing Server..." << std::endl;
		
		sockaddr_in address{};
		std::map<uint32_t, sockaddr_in> clients;

		while (true)
		{
			NetworkPacket packet = ReceivePacket(udpSocket, address);

			if (packet.packetID == JOIN_REQUEST)
			{
				HandleJoinRequest(udpSocket, address, packet);
				clients[packet.sourcePortNumber] = address;
			}
			else if (packet.packetID == GAME_INPUT)
			{
				HandlePlayerInput(udpSocket, address, packet);
			}
		}

		Disconnect();
	}
	else if(networkType == NetworkType::CLIENT)
	{
		std::cout << "Processing Client..." << std::endl;
		
		SendJoinRequest(udpSocket, serverAddress);

		NetworkPacket responsePacket = ReceivePacket(udpSocket, serverAddress);
		if (responsePacket.destinationPortNumber == port && responsePacket.packetID == REQUEST_ACCEPTED)
		{
			std::cout << "Joined the lobby successfully!" << std::endl;
		}

		ReceiveGameStateStart(udpSocket);

		while (true)
		{
			SendInput(udpSocket, serverAddress);

			NetworkPacket gameState = ReceivePacket(udpSocket, serverAddress);
			if (gameState.packetID == GAME_STATE_UPDATE)
			{
				std::cout << "Game state updated: " << gameState.data << std::endl;
			}
		}

		Disconnect();
	}
	else
	{
		// Initialize the system
		AESysInit(instanceH, show, 800, 600, 1, 60, false, NULL);

		// Changing the window title
		AESysSetWindowTitle("Asteroids!");

		//set background color
		AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

		// set starting game state to asteroid
		GameStateMgrInit(GS_ASTEROIDS);

		// breaks this loop if game state set to quit
		while (gGameStateCurr != GS_QUIT)
		{
			// reset the system modules
			AESysReset();

			// If not restarting, load the gamestate
			if (gGameStateCurr != GS_RESTART)
			{
				GameStateMgrUpdate();
				GameStateLoad();
			}
			else
			{
				gGameStateNext = gGameStateCurr = gGameStatePrev;
			}

			// Initialize the gamestate
			GameStateInit();

			// main game loop
			while (gGameStateCurr == gGameStateNext)
			{
				AESysFrameStart(); // start of frame

				GameStateUpdate(); // update current game state

				GameStateDraw(); // draw current game state

				AESysFrameEnd(); // end of frame

				// check if forcing the application to quit
				if (AESysDoesWindowExist() == false)
				{
					gGameStateNext = GS_QUIT;
				}

				g_dt = static_cast<f32>(AEFrameRateControllerGetFrameTime()); // get delta time
				g_appTime += g_dt; // accumulate application time
			}

			GameStateFree(); // free current game state

			// unload current game state unless set to restart
			if (gGameStateNext != GS_RESTART)
				GameStateUnload();

			// set prev and curr for the next game states
			gGameStatePrev = gGameStateCurr;
			gGameStateCurr = gGameStateNext;
		}
	
		// free the system
		AESysExit();
	}

	// free console
	FreeConsoleWindow();

	return 0;
}