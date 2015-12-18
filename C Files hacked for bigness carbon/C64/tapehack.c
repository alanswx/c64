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
#include "nfd.h"

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

void LoadTapeFS(nfdchar_t  *spec)
{
//	short fNum;
    FILE *theFile;
	long 	len;
	unsigned	char 	data;
	unsigned	short 	addr;
	long	curEOF;
	Ptr	buf;
#if	!DMA
	long	i;
#endif	
    theFile = fopen(spec,"r");
//	FSpOpenDF(spec, fsRdPerm, &fNum);
//	SetFPos(fNum, fsFromStart, 0x42);
    fseek(theFile,0x42,SEEK_SET);

    len=1; len=fread(&data,len,1,theFile);  addr=data;
    len=1; len=fread(&data,len,1,theFile); addr+=((unsigned short)data)<<8;

	//len=1; FSRead(fNum, &len, &data); addr=data;
	//len=1; FSRead(fNum, &len, &data); addr+=((unsigned short)data)<<8;
	
    fseek(theFile,0x0400,SEEK_SET);
//	SetFPos(fNum, fsFromStart, 0x0400);

    fseek(theFile,0,SEEK_END);
    curEOF = ftell(theFile);
    
    fseek(theFile,0x0400,SEEK_SET);

    //GetEOF(fNum,&curEOF);
	if (curEOF-0x0400 > 65535)
		debug_window_printf("Tape Error");
	else
		{
		buf=malloc(curEOF-0x0400);
		
		if (buf)
			{
			len=curEOF-0x400;
	
	#if	DMA
           //     FSRead(fNum,&len,&gRAMBlock[addr]);
           //     FSClose(fNum);
                fread(&gRAMBlock[addr],len,1,theFile);
                fclose(theFile);
                
	#else
			FSRead(fNum,&len,buf);
			FSClose(fNum);
			for (i=0;i<curEOF-0x0400;i++,addr++)
				MEMWRITEBYTE(addr,buf[i]);
	#endif
		
			free(buf);
			}
		else
			debug_window_printf("Couldn't allocate memory for tape");
		}

}


void LoadTape(void)
{
//	StandardFileReply reply;
//	FSSpec theFileSpec;

//	SFTypeList			types;
//		OSErr err=SimpleNavGetFile(&theFileSpec);
//
//	StandardGetFile(nil, (short)-1, types, &reply);
//	if (err==noErr)
    
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog( "T64", NULL, &outPath );
    
    if ( result == NFD_OKAY )
	{
		LoadTapeFS(outPath);
	}
    if (outPath) free(outPath);
}

void LoadTapeMixedMode(void)
{
//	StandardFileReply reply;
//	EventRecord			pullKey;
//	SFTypeList			types;

	//pull key out of event queue
//	WaitNextEvent(keyDownMask,&pullKey,0,NULL);
//	FSSpec theFileSpec;

//	SFTypeList			types;
//	OSErr err=SimpleNavGetFile(&theFileSpec);
			
//	StandardGetFile(nil, (short)-1, types, &reply);

//	if (err==noErr)
//	if (reply.sfGood)
    
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog( "T64", NULL, &outPath );
    
    if ( result == NFD_OKAY )
    {
		LoadTapeFS(outPath);

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

    if (outPath) free(outPath);

}

void	TapeInitialize(void)
{
	AddTrap(tapeTrap[0]);

}