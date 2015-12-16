/*-------------------------------------------------------------------------------*\
|
|	File:	Memory.c
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


#include <stdio.h>
#include "Drawing.h"
#include "DebugWindow.h"
#include "PageTables.h"
#include "Accessors.h"
#include "Memory.h"



#if	DEBUG
extern	Boolean	gVerbose;
#endif


/*	local constants	*/
#define	kROMResType			'ROM '
#define	kBasicROMResID		128
#define	kCharROMResID		129
#define	kKernalROMResID	130
#define	LOCALDEBUG			0


/*	local globals	*/
ReadHandler		*gMemRead;
WriteHandler	*gMemWrite;
unsigned char	*gRAMBlock = NULL;
unsigned char	*gBasicROM = NULL;
unsigned char	*gCharROM = NULL;
unsigned char	*gKernalROM = NULL;





void InitMemory(void)
{
	Handle	data;
	long		*temp;
	//long		index;
	
	
	//	allocate pointers for RAM storage and ROM copies
	gRAMBlock = (unsigned char*)calloc(65536,1);
	assert (gRAMBlock!=NULL);
	gBasicROM = (unsigned char*)calloc(8192,1);
	assert (gBasicROM!=NULL);
	gCharROM = (unsigned char*)calloc(4096,1);
	assert (gCharROM!=NULL);
	gKernalROM = (unsigned char*)calloc(8192,1);
	assert (gKernalROM!=NULL);
	//	load the BASIC ROMs
    fprintf(stderr,"AJS = LOAD ROMS HERE\n");
    fprintf(stderr,"AJS = AFTER ROMS HERE  %c\n",gBasicROM[0]);
#if 1
	data = Get1Resource(kROMResType,kBasicROMResID);
//	BlockMove(*data,gBasicROM,8192);
    memcpy(gBasicROM,*data,8192);
	ReleaseResource(data);
	
	//	load the CHAR ROMs
	data = Get1Resource(kROMResType,kCharROMResID);
//    BlockMove(*data,gCharROM,4096);
    memcpy(gCharROM,*data,4096);
	ReleaseResource(data);
	
	//	load the KERNAL ROMs
	data = Get1Resource(kROMResType,kKernalROMResID);
   // BlockMove(*data,gKernalROM,8192);
    memcpy(gKernalROM,*data,8192);
	ReleaseResource(data);
#endif
    fprintf(stderr,"AJS = AFTER ROMS HERE  %c\n",gBasicROM[0]);

	#if 0
	
	temp = (long*)READTable;
	temp[0] = (long)gRAMBlock;
	temp[1] = (long)gRAMBlock;
	temp[2] = (long)gRAMBlock;
	temp[3] = (long)gRAMBlock;
	temp[4] = (long)gRAMBlock;
	temp[5] = (long)gRAMBlock;
	temp[6] = (long)gRAMBlock;
	temp[7] = (long)gRAMBlock;
	temp[8] = (long)gRAMBlock;
	temp[9] = (long)gRAMBlock;
	temp[10] = (long)gBasicROM-0xA000;
	temp[11] = (long)gBasicROM-0xA000;
	temp[12] = (long)gRAMBlock;
	temp[13] = (long)0;
	temp[14] = (long)gKernalROM-0xE000;
	temp[15] = (long)gKernalROM-0xE000;
	
	temp = (long*)WRITETable;
	temp[0] = (long)gRAMBlock;
	temp[1] = (long)gRAMBlock;
	temp[2] = (long)gRAMBlock;
	temp[3] = (long)gRAMBlock;
	temp[4] = (long)gRAMBlock;
	temp[5] = (long)gRAMBlock;
	temp[6] = (long)gRAMBlock;
	temp[7] = (long)gRAMBlock;
	temp[8] = (long)gRAMBlock;
	temp[9] = (long)gRAMBlock;
	temp[10] = (long)gRAMBlock;
	temp[11] = (long)gRAMBlock;
	temp[12] = (long)gRAMBlock;
	temp[13] = (long)0;
	temp[14] = (long)gRAMBlock;
	temp[15] = (long)gRAMBlock;

	#endif	
	
	//	initialize DDR, PDR and memory page tables
	gRAMBlock[0] = 0x2F;
	gRAMBlock[1] = 0x37;
	gMemRead = Read111;
	gMemWrite = Write111;
	
	//DumpMemory();
}





void CleanUpMemory(void)
{
	//	dispose of allocated memory
	if (gRAMBlock)
		DisposePtr((Ptr)gRAMBlock);
	if (gBasicROM)
		DisposePtr((Ptr)gBasicROM);
	if (gCharROM)
		DisposePtr((Ptr)gCharROM);
	if (gKernalROM)
		DisposePtr((Ptr)gKernalROM);
}





void DumpMemory(void)
{
	static long	dumpIndex=0;
	FILE			*file;
	char			name[20];
	long			inner,outter;
	
	
	sprintf(name,"core #%d",(int)dumpIndex++);
	file = fopen(name,"w");
	
	for (outter = 0;outter < 65536;outter+=32)
	{
		fprintf(file,"%4lX: ",(long int)outter);
		for (inner=0;inner<32;inner++)
		{
			unsigned long	temp;
			
			temp = MEMREADBYTE(inner+outter);
			fprintf(file,"%2X ",(int)temp);
		}
		fprintf(file,"\n");
	}
	
	fclose(file);
}





void InitPageTables(void)
{
	long	*temp;
	
	
	//debug_window_printf("Memory mode switch: PDR = %lX",(long int)gRAMBlock[1]&0x07);
	switch(gRAMBlock[1]&0x07)
	{
		case 0x07:	//	111	BASIC:IN, IO:IN, CHAR:OUT, KERNAL:IN
			gMemRead = Read111;
			gMemWrite = Write111;
			
			#if 0 // ifndef powerc
			temp = (long*)READTable;

			temp[10] = temp[11] = (long)gBasicROM-0xA000;
			temp[13] = (long)0;
			temp[14] = temp[15] = (long)gKernalROM-0xE000;
			
			temp = (long*)WRITETable;
			temp[13] = (long)0;
			#endif

			break;

		case 0x06:	//110		BASIC:OUT, IO:IN, CHAR:OUT, KERNAL:IN
			gMemRead = Read110;
			gMemWrite = Write111;
			
			#if 0 // ifndef powerc
			temp = (long*)READTable;
			temp[10] = temp[11] = (long)gRAMBlock;
			temp[13] = (long)0;
			temp[14] = temp[15] = (long)gKernalROM-0xE000;
			
			temp = (long*)WRITETable;
			temp[13] = (long)0;
			#endif
			break;

		case 0x05:	//101		BASIC:OUT, IO:IN, CHAR:OUT, KERNAL:OUT
			gMemRead = Read101;
			gMemWrite = Write111;


			#if 0 // ifndef powerc
			temp = (long*)READTable;
			temp[10] = temp[11] = (long)gRAMBlock;
			temp[13] = (long)0;
			temp[14] = temp[15] = (long)gRAMBlock;
			
			temp = (long*)WRITETable;
			temp[13] = (long)0;
			#endif
			break;

		case 0x04:	//100		BASIC:OUT, IO:OUT, CHAR:OUT, KERNAL:OUT -- CHAREN ignored
			gMemRead = Read000;
			gMemWrite = Write000;


			#if 0 // ifndef powerc
			
			temp = (long*)READTable;
			temp[10] = temp[11] = (long)gRAMBlock;
			temp[13] = (long)gRAMBlock;
			temp[14] = temp[15] = (long)gRAMBlock;
			
			temp = (long*)WRITETable;
			temp[13] = (long)gRAMBlock;
			#endif
			break;
		
		case 0x03:	//011		BASIC:OUT, IO:IN, CHAR:OUT, KERNAL:IN
			gMemRead = Read011;
			gMemWrite = Write000;
			
			#if 0 // ifndef powerc
			temp = (long*)READTable;
			temp[10] = temp[11] = (long)gBasicROM-0xA000;
			temp[13] = (long)gCharROM-0xD000;
			temp[14] = temp[15] = (long)gKernalROM-0xE000;
			
			temp = (long*)WRITETable;
			temp[13] = (long)gRAMBlock;
			#endif
			break;
		
		case	0x02:	//010		BASIC:OUT, IO:OUT, CHAR:IN, KERNAL:IN
			gMemRead = Read010;
			gMemWrite = Write000;
			
			#if 0 // ifndef powerc
			temp = (long*)READTable;
			temp[10] = temp[11] = (long)gRAMBlock;
			temp[13] = (long)gCharROM-0xD000;
			temp[14] = temp[15] = (long)gKernalROM-0xE000;
			
			temp = (long*)WRITETable;
			temp[13] = (long)gRAMBlock;
			#endif
			break;

		case	0x01:	//001		BASIC:OUT, IO:OUT, CHAR:IN, KERNAL:OUT
			gMemRead = Read001;
			gMemWrite = Write000;
			
			#if 0 // ifndef powerc
			temp = (long*)READTable;
			temp[10] = temp[11] = (long)gRAMBlock;
			temp[13] = (long)gCharROM-0xD000;
			temp[14] = temp[15] = (long)gRAMBlock;
			
			temp = (long*)WRITETable;
			temp[13] = (long)gRAMBlock;
			
			#endif
			
			break;
			
		case	0x00:	//000		BASIC:OUT, IO:OUT, CHAR:OUT, KERNAL:OUT -- CHAREN ignored
			gMemRead = Read000;
			gMemWrite = Write000;
			
			#if 0 // ifndef powerc
			temp = (long*)READTable;
			temp[10] = temp[11] = (long)gRAMBlock;
			temp[13] = (long)gRAMBlock;
			temp[14] = temp[15] = (long)gRAMBlock;
			
			temp = (long*)WRITETable;
			temp[13] = (long)gRAMBlock;
			#endif

			break;


			
		default:
			debug_window_printf("REALLY Entered unknown memory mode!  PDR = %2X",(int)gRAMBlock[1]);
			break;
	}
	
	CalculateVideoRamOffsets();
}





//	
//	Read memory accessor routines for normal RAM
//		This accessor reads the RAM value unmolested
//

//#ifndef	powerc
//unsigned long RAMRead(unsigned long address:__A0)
//#else
unsigned long RAMRead(unsigned long address)
//#endif
{
	return gRAMBlock[address];
}





//	
//	Write memory accessor routines for normal RAM
//		This accessor writes the RAM value unmolested
//
void RAMWrite(unsigned long address,unsigned long value)
{
	gRAMBlock[address] = value;
}





//
//	Write memory accessor for page $0000-$00FF
//		This accessor catches writes to the PDR register
//		which controls the memory page tables
//
void PageZeroWrite(unsigned long address,unsigned long value)
{
	//	write is for non PDR address, pass it thru
	if (address != 1)
	{
		gRAMBlock[address] = value;
		return;
	}
	
	//	check for change in PDR register value
	if ((gRAMBlock[address]&0x07) != (value&0x07))
	{
		//	change in PDR register will cause a new page map
		gRAMBlock[address] = value;
		InitPageTables();
	}
	else
	{
		//	change in PDR register will not cause a new page map
		gRAMBlock[address] = value;
	}
}





//
//	Read memory accessor for pages $A000-$BFFF
//		This accessor catches reads to the Basic ROMs
//

//#ifndef	powerc
//unsigned long BasicROMRead(unsigned long address:__A0)
//#else
unsigned long BasicROMRead(unsigned long address)
//#endif
{
	return gBasicROM[address-0xA000];
}





//
//	Read memory accessor for page $D000-$DFFF
//		This accessor catches reads to the Char ROMs and color RAM
//
//#ifndef	powerc
//unsigned long CharROMRead(unsigned long address:__A0)
//#else
unsigned long CharROMRead(unsigned long address)
//#endif
{
	return gCharROM[address-0xD000];
}





//
//	Read memory accessor for pages $E000-$FFFF
//		This accessor catches reads to the Kernal ROMs
//
//#ifndef	powerc
//unsigned long KernalROMRead(unsigned long address:__A0)
//#else
unsigned long KernalROMRead(unsigned long address)
//#endif
{
	return gKernalROM[address-0xE000];
}