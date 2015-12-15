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
#include <Carbon/Carbon.h>
#include "ColorDrawing.h"
#include	"Drawing.h"
#include "DebugWindow.h"
#include "Preference.h"
#include	"Joystick.h"
//#include "Profiler.h"
#include "Event.h"
#include "Menu.h"
#include "Debug.h"
#include "Emulator.h"
#include "CPU.h"
#include	"DeviceMenu.h"
#include "Combug.h"
#include	"SID.h"
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
	
	

	while(!doEventLoop(0))
		CheckAndFixPixelDepth();	//Eric says this can just be after the update event..
	
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
	InitCursor();
	InitEvents();
	InitAppMenus();

	SetUpMontorDepths();
	AboutItemProc(0,0);

	InitPreferences();
	InitDebug();
	OpenCommodoreMonitorWindow();
	BlastColorTestPattern();
	InitEmulator();
	InitDevices ();
	InitJoystick();
	InitCombug();
	if (SoundInitialize()!=0)
		DebugStr("\pMemory was not allocated");


}





void CleanUpApplication(void)
{
	CleanUpCombug();
	CleanUpCommodoreWindow();
	CleanUpEmulator();
	CleanUpDebug();
	CleanUpPreferences();
	CleanupDevices();
//	#ifdef powerc
	DisposeSounds();
	CleanUpMonitorDepths();
//	#endif
}