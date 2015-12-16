/*-------------------------------------------------------------------------------*\
|
|	File:	Emulator.c
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


#include <string.h>
#include <Carbon/Carbon.h>
#include "DebugWindow.h"
#include "Menu.h"
#include "Accessors.h"
#include "Memory.h"
#include "VIC.h"
#include "SID.h"
#include "CRAM.h"
#include "CIA1.h"
#include "CIA2.h"
#include "SLOT1.h"
#include "SLOT2.h"
#include "CPU.h"
#include "ASMCPU.h"
#include "CPUState.h"
#include "Combug.h"
#include "Interrupts.h"
#include "CombugWindow.h"
#include "Emulator.h"





/*	external globals	*/
extern ReadHandler	*gMemRead;			//	Memory.c
extern WriteHandler	*gMemWrite;			//	Memory.c
extern unsigned char	*gRAMBlock;			//	Memory.c
extern unsigned long	gTraceMode;			//	CPUState.c
extern GWorldPtr		gCombugWorld;		// Combug.c
extern CombugState	gCombugState;		// Combug.c
extern WindowPtr		gCombugWindow;		// Combug.c
extern unsigned long	gPCRegister;




void InitEmulator(void)
{
	//	make the reset item (in the file menu) work
	//AddItemProc(FileResetItemProc,129,1);
		
	//	initialize the memory system
	InitMemory();
	
	// initialize the CPU emulator
	InitCPU();
	
	//	make sure the emulator is in the initial power on state
	ResetEmulator();
}





void CleanUpEmulator(void)
{
	CleanUpCPU();
}





void FileResetItemProc(short menu,short item)
{
	ResetEmulator();
}





//
//	Reset the state of the emulator to initial power on
//
void ResetEmulator(void)
{
	static char	wah=0;
	
	#if		DEBUG
		debug_window_printf("");
		debug_window_printf("--> RESET ");
		debug_window_printf("");
	#endif	DEBUG
	
//	debug_window_printf("Value of TV flag: %2lX",(long int)MEMREADBYTE(0x02A6));
	
	//	initialize DDR and PDR registers
	MEMWRITEBYTE(0,0x2F);
	MEMWRITEBYTE(1,0x37);
	
	memset(gRAMBlock,wah++,65535);
	
	ResetCPUState();
	if (gCombugWorld)
		DrawCombugWindow();
}



void HaltProcessor (void);
void HaltProcessor (void)
{
fprintf(stderr,"Shit, shouldn't get here..");
}

void EmulatorRun(void)
{
	char	line[256];	
	
	
	if (gCombugState.halted)
	{
		if (gCombugState.gto)
		{
			if (gCombugState.gtoValue==gPCRegister)
				{
				CombugScrollPrintf("Reached Destination Address");
				SelectWindow(gCombugWindow);
				DrawCombugWindow();
				gCombugState.gto=0;
				gCombugState.step=0;
				return;
				}

			gTraceMode = 1;
			CalculateNextInterrupt();
		}
		else
		{
			if (gCombugState.step)
				gCombugState.step--;
			else
				return;
			
			gTraceMode = 1;
			CalculateNextInterrupt();
		
			gPCRegister++;
			DisasmOneOpcode(line,gPCRegister-1);
			gPCRegister--;
			CombugScrollPrintf("%s",line);
		}

	}
	
    
	//#if	powerc
	CPU();
//	#else
//	ASMCPU();
//	#endif

	if (gTraceMode)
	{
		gTraceMode = 0;
		if (!gCombugState.gto)
			DrawCombugWindow();
	}

}