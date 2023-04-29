#pragma once

/*
Compilation macros:
	
	STR_DEBUG: intern all hashed strings. Allows use of the following functions.
		- DebugStringLookup(StringID) 
			- returns a const char* looked up using a StringID
		- DebugInternString(const char*)  
			- manually hashes and stores a string
			- automatically called by ToStringID(const char*) when STR_DEBUG is defined
		- DebugStringDictionary()
			- returns reference to the global string dictionary where all strings are interned

	MEM_DEBUG: sets the default allocator to a "noisy" wrapper around new_delete_resource,
		and calls _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF) to track
		memory leaks.
*/


/*
* Basic engine loop. Might have to abstract out certain functions outside.
* Returns: int -  0 if exited without issue.
*/
int EngineMain(int argc, char* args[], bool run_prototypes=false);

bool EngineInit();

void EngineCleanup();