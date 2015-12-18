/*-------------------------------------------------------------------------------*\||	File:	VICMemory.c||	Description:||||||	Copyright �	1994, Alan Steremberg and Ed Wynne|\*-------------------------------------------------------------------------------*/#include	"Accessors.h"#include "debugwindow.h"#include	"ColorDrawing.h"#include "Drawing.h"#include "DoubleDrawing.h"#include	"DrawingGlobals.h"#include	"Memory.h"#include <SDL2/SDL.h>extern	Boolean	gDoubleSize;#if	DEBUGextern	Boolean	gVerbose;#endif/*	local constants	*/#define	LOCALDEBUG			1//// From DoubleDrawingSetup.c//extern double	**DNormal_ColorTable;extern double	**DMulti_ColorTable;extern double	**gDoubleColorTable;////	From	ColorDrawing.c//extern	unsigned	long		gPixelDepth;////	From Memory.c//extern	unsigned	char	*gRAMBlock;extern	unsigned	char	*gCharROM;////	From VIC.c//extern	unsigned	char	gVICReg[];extern	unsigned	char	gCIA2Reg[];////	From CRAM.h//extern	unsigned char	gCRAMBlock[];////	Temporary///*	external globals	*/extern ReadHandler			*gMemRead;							//	Memory.cextern WriteHandler			*gMemWrite;							//	Memory.c////	Locals..////unsigned	char	gVICRegister[0x2F];unsigned	char	*gVideoCharacterBase;unsigned	char	*gVideoScreenRamBase;unsigned	char	*gVideoColorBase;unsigned	char 	*gVideoBankRamBase;Boolean	gUpdateBkgNormalRequest=FALSE;Boolean	gUpdateMultiColorRequest=FALSE;//RasteringFunction	gCurrentRasteringFunction=RasterLine_NormalText_40Char;RasteringFunction	gCurrentRasteringFunction=RasterLine_DefaultMode;void	UpdateBkgColors(){			switch (gPixelDepth)				{				case	8:					gAndBackColor[0]=eightBitTable[gVICReg[0x21]];					gAndBackColor[1]=eightBitTable[gVICReg[0x22]];					gAndBackColor[2]=eightBitTable[gVICReg[0x23]];					gAndBackColor[3]=eightBitTable[gVICReg[0x24]];				break;				case	4:					gAndBackColor[0]=fourBitTable[gVICReg[0x21]];					gAndBackColor[1]=fourBitTable[gVICReg[0x22]];					gAndBackColor[2]=fourBitTable[gVICReg[0x23]];					gAndBackColor[3]=fourBitTable[gVICReg[0x24]];				break;				case	1:				break;					}}void	RasterLine_DefaultMode (short	rasterline){//DebugStr("\pThis Drawing Mode is not implemented yet");}/*This gets called when the CIA 0xDD00 or the VIC 0xD018 registers get updated.*/void	CalculateVideoRamOffsets (){UInt32				VideoBankNum;UInt32				VideoScreenRamOffset;UInt32				VideoCharacterOffset;UInt32	scratch;//debug_window_printf("Recalculating offsets");/*Video Bank Selection====================CIA Register, 0xDD00, bits 0,1bits |  bank  |  start-------------------------00   |   3    |   0xC00001   |   2    |   0x800010   |   1    |   0x400011   |   0    |   0x0000	(Default)	*/VideoBankNum			=		(gCIA2Reg[0x00]&0x03)^0x3;gVideoBankRamBase		=		gRAMBlock+ VideoBankNum*0x4000;#if	LOCALDEBUG && DEBUG//	if (gVerbose)//		debug_window_printf("Bank: %X",VideoBankNum);#endif//debug_window_printf("Vicreg=%X",gVICReg[0x18]);/*Screen Memory=============VIC Register, 0xD018bits		|	Location--------------------0000XXXX |  0x00000001XXXX |  0x0400	(Default)0010XXXX |  0x08000011XXXX |  0x0C000100XXXX |  0x10000101XXXX |  0x14000110XXXX |  0x18000111XXXX |  0x1C001000XXXX |  0x20001001XXXX |  0x24001010XXXX |  0x28001011XXXX |  0x2c001100XXXX |  0x30001101XXXX |  0x34001110XXXX |  0x38001111XXXX |  0x3c00*/VideoScreenRamOffset	=		((gVICReg[0x18]&0xf0)>>4)*0x400;//double check this...//VideoScreenRamOffset	=		((gVICReg[0x18]&0xf0)>>4)*0x200;gVideoScreenRamBase	=		VideoScreenRamOffset+gVideoBankRamBase;/*Color Memory============Fixed at 0xD800*/gVideoColorBase			=		gCRAMBlock;/*Character Memory================VIC Register, 0xD018bits		|	Location--------------------XXXX000X |  0x0000XXXX001X |  0x0800XXXX010X |  0x1000		ROM Image in Bank 0 & 2 (default)XXXX011X |  0x1800		ROM Image in Bank 0 & 2XXXX100X |  0x2000		XXXX101X |  0x2800XXXX110X |  0x3000XXXX111X |  0x0800*///if bitmap mode is on, then we need to ignore bits 1 and 2scratch=gVICReg[0x18];if (gVICReg[0x11]&0x20)	//check bitmap mode	scratch&=~0x06;VideoCharacterOffset =		((scratch&0x0e)>>1)*0x0800;gVideoCharacterBase	=		VideoCharacterOffset+gVideoBankRamBase;////	If we are in Bank 0 or 2, check to see if we should point into the char rom//if (VideoBankNum==0 || VideoBankNum==2)	{	if (VideoCharacterOffset== 0x1000)		{//		if ((gRAMBlock[1] & 0x07)!=5)	{		gVideoCharacterBase=gCharROM;//			debug_window_printf("using charrom");	}	}	//	//	IS THIS CORRECT?	//	else if (VideoCharacterOffset== 0x1800)		{//		if ((gRAMBlock[1] & 0x07)!=5)	{		gVideoCharacterBase=gCharROM+0x800;//		debug_window_printf("using charrom");	}	}	}/*Uppercase / Lowercase Select (or whatever)==========================================VIC Register, 0xD018bits  |   offset	----------------0	 	|	0x00001		|	0x0800*///if (gVICReg[0x18]&1)//	debug_window_printf("silly bit");;//	gVideoCharacterBase+=0x0800;//gVideoCharacterBase+=(gVICReg[0x18]&1)*0x0800;//debug_window_printf("charram=%X",gVideoCharacterBase-gRAMBlock);}void	VICChangeVideoModes1Bit(){	//	//	Based on 0xD011	//	Boolean	ExtendedColorTextMode	=	gVICReg[0x11]&0x40;	Boolean	BitMapMode					=	gVICReg[0x11]&0x20;	Boolean	BlankScreen					=	!(gVICReg[0x11]&0x10);	Boolean	TwentyFiveRowMode			=	gVICReg[0x11]&0x08;	//	//	Based on 0xD016	//	Boolean	MultiColorMode				=	gVICReg[0x16]&0x10;	Boolean	FourtyColumnMode			=	gVICReg[0x16]&0x08;		UpdateBkgColors();	if (!BlankScreen)		{//			gCurrentRasteringFunction=RasterLine_Border_4Bit;//			debug_window_printf("Switched into RasterLine_Border_4Bit Mode");		}	else		{		gCurrentRasteringFunction=RasterLine_NormalText_40Char1Bit;		debug_window_printf("Switched into RasterLine_NormalText_40Char1Bit Mode");				}}void	VICChangeVideoModes4Bit(){	//	//	Based on 0xD011	//	Boolean	ExtendedColorTextMode	=	gVICReg[0x11]&0x40;	Boolean	BitMapMode					=	gVICReg[0x11]&0x20;	Boolean	BlankScreen					=	!(gVICReg[0x11]&0x10);	Boolean	TwentyFiveRowMode			=	gVICReg[0x11]&0x08;	//	//	Based on 0xD016	//	Boolean	MultiColorMode				=	gVICReg[0x16]&0x10;	Boolean	FourtyColumnMode			=	gVICReg[0x16]&0x08;		UpdateBkgColors();gUpdateBkgNormalRequest=FALSE;gUpdateMultiColorRequest=FALSE;	if (BlankScreen)		{			gCurrentRasteringFunction=RasterLine_Border_4Bit;			debug_window_printf("Switched into RasterLine_Border_4Bit Mode");		}	else		{//if (FourtyColumnMode)		//	{			if (MultiColorMode/* && !BitMapMode*/)				{				gUpdateBkgNormalRequest=TRUE;				UpdateBackgroundColor_NormalText();			//	gCurrentRasteringFunction=RasterLine_MultiColorText_40Char4Bit;			//	debug_window_printf("Switched into RasterLine_MultiColorText_40Char Mode");				}			else				{				gUpdateBkgNormalRequest=TRUE;				UpdateBackgroundColor_NormalText();				gCurrentRasteringFunction=RasterLine_NormalText_40Char4Bit;						debug_window_printf("Switched into RasterLine_NormalText_40Char4Bit Mode");				}		//	else if (ExtendedColorTextMode &&!BitMapMode)		//		{		//		//Be sure to setup the back color, since multicolor stomps it		//		gAndBackColor[3]=gBackColor3;		//		gCurrentRasteringFunction=RasterLine_ExtendedColorText_40Char;		//		debug_window_printf("Switched into RasterLine_ExtendedColorText_40Char Mode");		//		}		//	else 		//	if (BitMapMode && !MultiColorMode)		//		{				//				//	Is there a 40/38 mode?				//		//		gCurrentRasteringFunction=RasterLine_Bitmap_40Char;		//		debug_window_printf("Switched into RasterLine_Bitmap_40Char Mode");		//		}		//	 if (!BitMapMode && !ExtendedColorTextMode && !MultiColorMode)		//		{		//		}		//	}		}}void	VICChangeVideoModes8Bit(){	//	//	Based on 0xD011	//	Boolean	ExtendedColorTextMode	=	gVICReg[0x11]&0x40;	Boolean	BitMapMode					=	gVICReg[0x11]&0x20;	Boolean	BlankScreen					=	!(gVICReg[0x11]&0x10);	Boolean	TwentyFiveRowMode			=	gVICReg[0x11]&0x08;	//	//	Based on 0xD016	//	Boolean	MultiColorMode				=	gVICReg[0x16]&0x10;	Boolean	FourtyColumnMode			=	gVICReg[0x16]&0x08;		UpdateBkgColors();		CalculateVideoRamOffsets();//temporary..	gUpdateBkgNormalRequest=FALSE;	gUpdateMultiColorRequest=FALSE;	if (BlankScreen)		{			gCurrentRasteringFunction=RasterLine_Border_8Bit;			#if	LOCALDEBUG && DEBUG	if (gVerbose)		debug_window_printf("Switched into RasterLine_Border_8Bit Mode");	#endif		}	else		{					if (MultiColorMode && !BitMapMode)			{				gUpdateBkgNormalRequest=TRUE;				gUpdateMultiColorRequest=TRUE;				gNastyColorTable=Multi_ColorTable;				UpdateBackgroundColor_NormalText();				BuildMultiColorTable();								gCurrentRasteringFunction=RasterLine_NormalText_Sprite_40Char;				#if	LOCALDEBUG && DEBUG				if (gVerbose)					debug_window_printf("Switched into RasterLine_NormalText_Sprite_40Char Mode - multicolor");				#endif			}			else if (MultiColorMode &&  BitMapMode)			{				gCurrentRasteringFunction=RasterLine_MultiColorBitmap_Sprite_40Char;				#if	LOCALDEBUG && DEBUG				if (gVerbose)					debug_window_printf("Switched into RasterLine_MultiColorBitmap_Sprite_40Char Mode");				#endif			}			else if (ExtendedColorTextMode &&!BitMapMode)			{					gCurrentRasteringFunction=RasterLine_ExtendedColorText_Sprite_40Char;					#if	LOCALDEBUG && DEBUG					if (gVerbose)						debug_window_printf("Switched into RasterLine_ExtendedColorText_Sprite_40Char Mode");					#endif			}			else if (BitMapMode && !MultiColorMode)				{				//				//	Is there a 40/38 mode?				//					gCurrentRasteringFunction=RasterLine_Bitmap_40Char;					#if	LOCALDEBUG && DEBUG					if (gVerbose)						debug_window_printf("Switched into RasterLine_Bitmap_40Char Mode");					#endif				}			else if (!BitMapMode && !ExtendedColorTextMode && !MultiColorMode)				{				gNastyColorTable=Normal_ColorTable;				gUpdateBkgNormalRequest=TRUE;				UpdateBackgroundColor_NormalText();				gCurrentRasteringFunction=RasterLine_NormalText_Sprite_40Char;				#if	LOCALDEBUG && DEBUG				if (gVerbose)					debug_window_printf("Switched into RasterLine_NormalText_Sprite_40Char Mode");				#endif				}			else 				debug_window_printf("Bogus Video Mode");			}}void	VICChangeVideoModes8BitDouble(){	//	//	Based on 0xD011	//	Boolean	ExtendedColorTextMode	=	gVICReg[0x11]&0x40;	Boolean	BitMapMode					=	gVICReg[0x11]&0x20;	Boolean	BlankScreen					=	!(gVICReg[0x11]&0x10);	Boolean	TwentyFiveRowMode			=	gVICReg[0x11]&0x08;	//	//	Based on 0xD016	//	Boolean	MultiColorMode				=	gVICReg[0x16]&0x10;	Boolean	FourtyColumnMode			=	gVICReg[0x16]&0x08;		UpdateBkgColors();		CalculateVideoRamOffsets();//temporary..	gUpdateBkgNormalRequest=FALSE;	gUpdateMultiColorRequest=FALSE;	if (BlankScreen)		{			gCurrentRasteringFunction=RasterLine_DBorder_8Bit;			#if	LOCALDEBUG && DEBUG	if (gVerbose)		debug_window_printf("Switched into RasterLine_Border_8Bit Mode");	#endif		}	else		{					if (MultiColorMode && !BitMapMode)			{				gUpdateBkgNormalRequest=TRUE;				gUpdateMultiColorRequest=TRUE;//				gNastyColorTable=Multi_ColorTable;				gDoubleColorTable=DMulti_ColorTable;//				gDoubleColorTable=DNormal_ColorTable;				UpdateBackgroundColor_NormalText();				BuildMultiColorTable();								gCurrentRasteringFunction=RasterLine_DNormalText_Sprite_40Char;				#if	LOCALDEBUG && DEBUG				if (gVerbose)					debug_window_printf("Switched into RasterLine_DNormalText_Sprite_40Char Mode - multicolor");				#endif			}			else if (MultiColorMode &&  BitMapMode)			{				gCurrentRasteringFunction=RasterLine_DMultiColorBitmap_Sprite_40Char;				#if	LOCALDEBUG && DEBUG				if (gVerbose)					debug_window_printf("Switched into RasterLine_DMultiColorBitmap_Sprite_40Char Mode");				#endif			}			else if (ExtendedColorTextMode &&!BitMapMode)			{			//			//should be easy to retrofit for double size			//					gCurrentRasteringFunction=RasterLine_DExtendedColorText_Sprite_40Char;					#if	LOCALDEBUG && DEBUG					if (gVerbose)						debug_window_printf("Switched into RasterLine_DExtendedColorText_Sprite_40Char Mode");					#endif			}			else if (BitMapMode && !MultiColorMode)				{									gCurrentRasteringFunction=RasterLine_DBitmap_40Char;					#if	LOCALDEBUG && DEBUG					if (gVerbose)						debug_window_printf("Switched into RasterLine_DBitmap_40Char Mode");					#endif				}			else if (!BitMapMode && !ExtendedColorTextMode && !MultiColorMode)				{				gDoubleColorTable=DNormal_ColorTable;				gUpdateBkgNormalRequest=TRUE;				DoubleUpdateBackgroundColor_NormalText();				gCurrentRasteringFunction=RasterLine_DNormalText_Sprite_40Char;								#if	LOCALDEBUG && DEBUG				if (gVerbose)					debug_window_printf("Switched into RasterLine_DNormalText_Sprite_40Char Mode");				#endif				}			else 				debug_window_printf("Bogus Video Mode");			}}void	VICChangeVideoModes(){    fprintf(stderr,"VICCHangeVideoModes %d\n",gPixelDepth);        fprintf(stderr,"sizeof(unsigned long) %d\n",sizeof(unsigned long));    fprintf(stderr,"sizeof(double) %d\n",sizeof(unsigned long));    fprintf(stderr,"sizeof(Uint32) %d\n",sizeof(Uint32));    fprintf(stderr,"sizeof(char) %d\n",sizeof(char));        gPixelDepth=8;    VICChangeVideoModes8Bit();	if (gDoubleSize)		{			VICChangeVideoModes8BitDouble();            fprintf(stderr,"VICCHangeVideoModes8D\n");		}	else			switch (gPixelDepth)				{				case	8:					VICChangeVideoModes8Bit();                        fprintf(stderr,"VICCHangeVideoModes8\n");				break;				case	4:					VICChangeVideoModes4Bit();				break;				case	1:					VICChangeVideoModes1Bit();				break;					}		}#include	"VIC.h"#include	"CIA2.h"#include "VICInterrupts.h"#define	SPRITES		0#define	USEVICINT	1void	fudge_drawing_vals(void);void	fudge_drawing_vals(){UInt32	i;UInt32 numlines,start;static	int mode=0;//mode++;////	Setup VIC and CIA registers//	MEMWRITEBYTE(0xDD00,0x03);	MEMWRITEBYTE(0xD018,0x14);//CIA2Write(0xDD00,0xFF);//VICWrite(0xd018,0x14);VICWrite(0xd021,0x03);VICWrite(0xd022,0x0C);VICWrite(0xd023,0x08);VICWrite(0xd024,0x12);#if	SPRITESVICWrite(0xd015,0xFF);#endifmode=1;if (mode%5==0)	{	VICWrite(0xd016,0x18);		//multicolor text	VICWrite(0xd011,0x18); 	//  (use for multicolor)	}else if (mode%5==1)	{	VICWrite(0xd016,0x08);	// (use for extended)	VICWrite(0xd011,0x58); //01001000	extended text	}else if (mode%5==2)	{	VICWrite(0xd016,0x08);	// normal text 	VICWrite(0xd011,0x18); // normal text	}else if (mode%5==3)	{	VICWrite(0xd016,0x08);	// bitmap	VICWrite(0xd011,0x38); // bitmap	}	else if (mode%5==4)	{	VICWrite(0xd016,0x18);	// multicolor bitmap	VICWrite(0xd011,0x38); // multicolor bitmap	}//for (i=0;i<1000;i++)//	gVideoColorBase[i]=0x0A;for (i=0;i<1000;i++) 	gVideoColorBase[i]=i%16;	for (i=0;i<1000;i++)	gVideoScreenRamBase[i]=i%256;//for (i=0;i<1000;i++)//	gVideoScreenRamBase[i]=1;////	Blast the characters to the screen..//start=TickCount();numlines=0;do	{#if	USEVICINTfor (i=0;i<312;i++)	VICRasterIncrement();#elsefor (i=0;i<200;i++)	gCurrentRasteringFunction(i);#endif//for (i=0;i<1000;i++)//	gVideoScreenRamBase[i]=((i%256)+(numlines%256))%256;        CopyOffScreenToWindow();	numlines++;			}while (TickCount()-60 < start);debug_window_printf("We got %d frames per second",numlines);}