/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_System.h"

//Axtrace system
AT3::System  g_theSystem;

//WinMain Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int nCmdShow)
{
	// Init system
	if(!g_theSystem.init(hInstance, lpCmdLine)) return 1;

	// Run system
	g_theSystem.run();

	// Release system
	g_theSystem.release();

	return 0;
}

