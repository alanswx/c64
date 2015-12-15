/*-------------------------------------------------------------------------------*\
|
|	File:	TapeHack.c
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

#include	"Accessors.h"
#include	"Tape.h"
#include	"Traps.h"
#include	"Memory.h"
#include	"CPU.h"
#include "DebugWindow.h"

#define	FLAGS					gCPUFlags
#define	CYCLES				gCPUCycles
#define	POPBYTE()			MEMREADBYTE(STACK+257);STACK=(unsigned char)STACK+1
#define	POPWORD()			MEMREADWORD(STACK+257);STACK=(unsigned char)STACK+2
#define	PUSHBYTE(x)			MEMWRITEBYTE((STACK+256),x);STACK=(unsigned char)STACK-1
#define	PUSHWORD(x)			STACK=(unsigned char)STACK-1;MEMWRITEWORD((STACK+256),x);STACK=(unsigned char)STACK-1
#define	PC						gCPUPC


#define	BITN					0x80
#define	BITV					0x40
#define	BITB					0x10
#define	BITD					0x08
#define	BITI					0x04
#define	BITZ					0x02
#define	BITC					0x01

#define	CLEARFLAGD()		FLAGS &= ~BITD
#define	CLEARFLAGC()		FLAGS &= ~BITC
#define	CLEARFLAGI()		FLAGS &= ~BITI
#define	CLEARFLAGZ()		FLAGS &= ~(BITZ)
#define	CLEARFLAGNZ()		FLAGS &= ~(BITN+BITZ)
#define	CLEARFLAGNVZ()		FLAGS &= ~(BITN+BITV+BITZ)
#define	CLEARFLAGNZC()		FLAGS	&= ~(BITN+BITZ+BITC)
#define	CLEARFLAGNVZC()	FLAGS &= ~(BITN+BITV+BITZ+BITC)
#define	SETFLAGN()			FLAGS |= BITN
#define	SETFLAGI()			FLAGS |= BITI
#define	SETFLAGZ()			FLAGS |= BITZ
#define	SETFLAGC()			FLAGS |= BITC
#define	SETFLAGV()			FLAGS |= BITV
#define	TESTFLAGN()			(FLAGS & BITN)
#define	TESTFLAGZ()			(FLAGS & BITZ)
#define	TESTFLAGC()			(FLAGS & BITC)
#define	TESTFLAGD()			(FLAGS & BITD)
#define	TESTFLAGI()			(FLAGS & BITI)
#define	TESTFLAGV()			(FLAGS & BITV)

extern	unsigned	char	gCPUFlags;
extern	unsigned	char	gCPUAccumulator;
extern	unsigned short	PC;

/*	external globals	*/
extern void i60(void);

extern ReadHandler			*gMemRead;							//	Memory.c
extern WriteHandler			*gMemWrite;							//	Memory.c


static trap tapeTrap[] =
{
{ 0xF539, {0x20, 0xD0, 0xF7}, LoadTapeMixedMode }
};


/*
Al's hack, yes this chews memory, but who cares, it is just faster that way.

*/

#define	DMA	1
#if DMA
//
//	From Memory.c
//
extern	unsigned	char	*gRAMBlock;

#endif

void LoadTapeFS(FSSpec *spec)
{
	short fNum;
	long 	len;
	unsigned	char 	data;
	unsigned	short 	addr;
	long	curEOF;
	Ptr	buf;
#if	!DMA
	long	i;
#endif	
	FSpOpenDF(spec, fsRdPerm, &fNum);
	SetFPos(fNum, fsFromStart, 0x42);
	len=1; FSRead(fNum, &len, &data); addr=data;
	len=1; FSRead(fNum, &len, &data); addr+=((unsigned short)data)<<8;
	
	SetFPos(fNum, fsFromStart, 0x0400);
	GetEOF(fNum,&curEOF);
	if (curEOF-0x0400 > 65535)
		debug_window_printf("Tape Error");
	else
		{
		buf=NewPtr(curEOF-0x0400);
		
		if (buf)
			{
			len=curEOF-0x400;
	
	#if	DMA
			FSRead(fNum,&len,&gRAMBlock[addr]);
			FSClose(fNum);
	#else
			FSRead(fNum,&len,buf);
			FSClose(fNum);
			for (i=0;i<curEOF-0x0400;i++,addr++)
				MEMWRITEBYTE(addr,buf[i]);
	#endif
		
			DisposePtr(buf);
			}
		else
			debug_window_printf("Couldn't allocate memory for tape");
		}

}


void LoadTape(void)
{
//	StandardFileReply reply;
	FSSpec theFileSpec;

//	SFTypeList			types;
		OSErr err=SimpleNavGetFile(&theFileSpec);

//	StandardGetFile(nil, (short)-1, types, &reply);
	if (err==noErr)
	{
		LoadTapeFS(&theFileSpec);
	}
	
}

void LoadTapeMixedMode(void)
{
//	StandardFileReply reply;
	EventRecord			pullKey;
//	SFTypeList			types;

	//pull key out of event queue
	WaitNextEvent(keyDownMask,&pullKey,0,NULL);
	FSSpec theFileSpec;

//	SFTypeList			types;
	OSErr err=SimpleNavGetFile(&theFileSpec);
			
//	StandardGetFile(nil, (short)-1, types, &reply);

	if (err==noErr)
//	if (reply.sfGood)
	{
		LoadTapeFS(&theFileSpec);

		SETFLAGZ();
		CLEARFLAGC();
		gCPUAccumulator=0;

	}
	else
	{
		CLEARFLAGZ();
		gCPUAccumulator=4;
	}

 	CPUOP_60_RTS();

	
}

void	TapeInitialize(void)
{
	AddTrap(tapeTrap[0]);

}