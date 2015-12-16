/*-------------------------------------------------------------------------------*\
|
|	File:	Main.c
|
|	Description:
|
|
|
|
|
|	Copyright �	1994, Alan Steremberg and Ed Wynne
|
\*-------------------------------------------------------------------------------*/

#include <SDL2/SDL.h>

#include "ColorDrawing.h"
#include "Drawing.h"
#include "DebugWindow.h"
#include "Preference.h"
#include "Joystick.h"
//#include "Profiler.h"
#include "Event.h"
#include "Menu.h"
#include "Debug.h"
#include "Emulator.h"
#include "CPU.h"
#include "DeviceMenu.h"
#include "Combug.h"
#include "SID.h"
#include "Main.h"

#include <Assert.h>

void ASMCPU(void);


int main(int argc,char **argv)
{

	#if __profile__
	if (!ProfilerInit(collectSummary, microsecondsTimeBase, 500, 100))
	{
	#endif

	InitApplication();
        
        
    //Main loop flag
    bool quit = false;
    
    //Event handler
    SDL_Event e;
        
    //While application is running
    while( !quit )
    {
            //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
        }
        
#if	1
        EmulatorRun();
#else
        fudge_drawing_vals();
#endif
        
        CopyOffScreenToWindow();
    }


    CleanUpApplication();
	
	#if __profile__
		ProfilerDump("\pCFiles.prof");
		ProfilerTerm();
	}
	#endif

	return noErr;
}





void InitApplication(void)
{
//	short	err;
	
	
//	InitGraf(&qd.thePort);
//	InitFonts();
//	InitWindows();
//	InitMenus();
//	TEInit();
	//InitDialogs(nil);
	//InitCursor();
	//InitEvents();
	//InitAppMenus();

	//SetUpMontorDepths();
	//AboutItemProc(0,0);

	//InitPreferences();
	InitDebug();
	OpenCommodoreMonitorWindow();
	BlastColorTestPattern();
	InitEmulator();
	InitDevices ();
	InitJoystick();
	InitCombug();
//	if (SoundInitialize()!=0)
//		DebugStr("\pMemory was not allocated");


}





void CleanUpApplication(void)
{
	CleanUpCombug();
	CleanUpCommodoreWindow();
	CleanUpEmulator();
	CleanUpDebug();
	//CleanUpPreferences();
	CleanupDevices();
//	#ifdef powerc
	DisposeSounds();
//	CleanUpMonitorDepths();
//	#endif
}