#include "EngineMain.h"

int main(int argc, char* args[]) {
	bool status = EngineInit();
	if (!status)
		return -1;

	//Specify third argument as true because we want to run the prototypes.
	int exit_code = EngineMain(argc, args, true);
	
	EngineCleanup();
	
	return exit_code;
}