/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/


#include "stdafx.h"
#include "ATD_System.h"

//--------------------------------------------------------------------------------------------
BOOL WINAPI DllMain(
  _In_  HINSTANCE hinstDLL,
  _In_  DWORD fdwReason,
  _In_  LPVOID lpvReserved
)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// init global value
		if(!_OnProcessAttached()) return FALSE;
		break;

	case DLL_PROCESS_DETACH:
		_OnProcessDetached();
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
