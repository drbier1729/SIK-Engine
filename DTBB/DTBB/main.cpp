#include"stdafx.h"

#include "Engine/EngineMain.h"

int main(int argc, char* argv[]) {

	//Initialize the engine
	bool status = EngineInit();

	//If intitialization failed return exit code -2
	if (!status){
		return -2;
	}

	//Start the main engine loop.
	int exit_code = EngineMain(argc, argv);

	//Cleanup and shutdown the engine
	EngineCleanup();

	return exit_code;
}