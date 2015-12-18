/*-------------------------------------------------------------------------------*\
|
|	File:	DrawingSetup.c
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


#include	"DebugWindow.h"
#include "ColorTables.h"
#include	"Drawing.h"
#include "DoubleDrawing.h"
#include "ColorTab4bit.h"
//#include "ColorLookupTab.h" // ajs
#if	DEBUG
extern	Boolean	gVerbose;
#endif

//
//	From ColorDrawing.c
//

extern	unsigned	long		gPixelDepth;
extern	unsigned char		gVICReg[64];
extern	Boolean	gDoubleSize;
//
//	The border color is already extended to be a long, and be a correct 'Mac' color index
//
UInt32	gBorderColor;

//
//	The array of background colors is already extended and each is a correct 'Mac' color index
//
UInt32	gAndBackColor[4];
unsigned	char	gBackColorsUnconverted[4];

UInt32	**Normal_ColorTable;
UInt32	*gRasterBasedOffset;

//
//	This is a table to convert a commodore color into a correct mac color index. This is because
//	our color table is not necessarily layed out in the same order as the Commodore's was. There is 
//	probably a better way of doing this, but I am not that good..
//

//	This table is the 4 bit conversion color table
//
UInt32 fourBitTable[16] =
	{
	0xFFFFFFFF,
	0x00000000,
	0x11111111,
	0x22222222,
	0x33333333,
	0x44444444,
	0x55555555,
	0x66666666,
	0x77777777,
	0x88888888,
	0x99999999,
	0xAAAAAAAA,
	0xBBBBBBBB,
	0xCCCCCCCC,
	0xDDDDDDDD,
	0xEEEEEEEE
	};

//	This table is the 8 bit conversion color table
//
UInt32 eightBitTable[16] =
	{
	0x01010101,
	0x02020202,
	0x03030303,
	0x04040404,
	0x05050505,
	0x06060606,
	0x07070707,
	0x08080808,
	0x09090909,
	0x0A0A0A0A,
	0X0B0B0B0B,
	0X0C0C0C0C,
	0X0D0D0D0D,
	0X0E0E0E0E,
	0X0F0F0F0F,
	0X10101010,
	};




UInt32	Normal_FourBitColorTable[256];


//
//	Thse are the tables used for MultiColor Mode.  The first 8 entries are used for the normal colors
//	The other 8 are the multicolor colors/patterns.
//

UInt32	Multi_Color0[16];
UInt32	Multi_Color1[16];
UInt32	Multi_Color2[16];
UInt32	Multi_Color3[16];
UInt32	Multi_Color4[16];
UInt32	Multi_Color5[16];
UInt32	Multi_Color6[16];
UInt32	Multi_Color7[16];
UInt32	*Multi_ColorTable[16] =
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, Multi_Color0, Multi_Color1, Multi_Color2, Multi_Color3, Multi_Color4, Multi_Color5, Multi_Color6, Multi_Color7 };

UInt32			**gNastyColorTable;




					//00 00  b0 b0 b0 b0
					//00 01	b0 b0 b1 b1
					//00 10	b0 b0 b2 b2
					//00 11	b0 b0 fg fg

					//01 00  b1 b1 b0 b0
					//01 01	b1 b1 b1 b1
					//01 10	b1 b1 b2 b2
					//01 11	b1 b1 fg fg
					
					//10 00  b2 b2 b0 b0
					//10 01	b2 b2 b1 b1
					//10 10	b2 b2 b2 b2
					//10 11	b2 b2 fg fg

					//11 00  fg fg b0 b0
					//11 01	fg fg b1 b1
					//11 10	fg fg b2 b2
					//11 11	fg fg bg fg


//
//	These routines build 8 bit and  tables that can be used in multicolor mode.
//
void	BuildMultiColorTable8Bit()
{
register	short	h,i,k,j;

UInt32	oldcolor=gAndBackColor[3];

	for (k=8;k<16;k++)
		{
		j=0;
		gAndBackColor[3]=eightBitTable[k-8];
		for (h=0;h<4;h++)
			for (i=0;i<4;i++)
				{
				(Multi_ColorTable[k])[j++]=(gAndBackColor[h] & 0xFFFF0000) | (gAndBackColor[i] & 0x0000FFFF);
				}
		}
gAndBackColor[3]=oldcolor;

//debug_window_printf("rebuilding 8 bit multicolor table");
}


//
//	Build Multi Color Table 4 bit.. This table shows the beginnings of the logic used in this function.
//	b=background fg=foreground color..
//

	//00 00 00 00  b0 b0 b0 b0
	//00 00 00 01	b0 b0 b0 b1
	//00 00 00 10	b0 b0 b0 b2
	//00 00 00 11	b0 b0 b0 fg

	//00 00 01 00  b0 b0 b1 b0
	//00 00 01 01	b0 b0 b1 b1
	//00 00 01 10	b0 b0 b1 b2
	//00 00 01 11	b0 b0 b1 fg
	
	//00 00 10 00  b0 b0 b2 b0
	//00 00 10 01	b0 b0 b2 b1
	//00 00 10 10	b0 b0 b2 b2
	//00 00 10 11	b0 b0 b2 fg

	//...
	//...
	//...




void	BuildMultiColorTable()
{
if (gDoubleSize)
	DoubleBuildMultiColorTable8Bit();
else

switch (gPixelDepth)
	{
	case	8:
		BuildMultiColorTable8Bit();
		break;
	}
}

//
//	This function needs to be called for:
//			RasterLine_MultiColorText_40Char
//			RasterLine_NormalText_40Char
//
//	This stamps the background color, and FF's into a huge color table for normal text mode. This is where we get our
// speed from!

void	UpdateBackgroundColor_NormalText(void)
{
register	short	i,j;
static	UInt32	lastbackcolor=0;
static	short	emptycache=0;
Boolean	NormalMode;

if (gDoubleSize==TRUE)
{
	DoubleUpdateBackgroundColor_NormalText();
	return;
}
	
if (lastbackcolor!=gAndBackColor[0])
	switch (gPixelDepth)
		{
		case	8:
			
			Normal_ColorTable=ColorTables[gVICReg[0x21]];
			if (!(gVICReg[0x16]&0x10))
				{				
				gNastyColorTable=Normal_ColorTable;
				}

			for (i=0;i<8;i++)
				Multi_ColorTable[i]=Normal_ColorTable[i];
			
							
			break;
		}

lastbackcolor=gAndBackColor[0];
}



void	InitializeDrawingTables (void)
{
	short	i=0;
	
	debug_window_printf("Allocating Drawing Tables");
	
	//
	//	Allocate Raster Offset Table
	//
	
	gRasterBasedOffset = (UInt32 *)malloc(sizeof(UInt32)*200);
	assert (gRasterBasedOffset!=NULL);
	
	for (i=0;i<200;i++)
		gRasterBasedOffset[i]=(i/8)*40;
		
		
	//
	// Only on a PowerMac?
	//
	
	DoubleBuildColorTable();

	
}

void	DisposeDrawingTables(void)
{
	debug_window_printf("Disposing Drawing Tables");
	
	free(gRasterBasedOffset);
}
