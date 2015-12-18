/*-------------------------------------------------------------------------------*\||	File:	VICInterrupts.c||	Description:||||||	Copyright �	1994, Alan Steremberg and Ed Wynne|\*-------------------------------------------------------------------------------*/#include	"DebugWindow.h"#include	"Drawing.h"#include "CPU.h"#include	"ColorDrawing.h"#include	"Interrupts.h"#include	"VICSprites.h"#include	"VICInterrupts.h"#if	DEBUGextern	Boolean	gVerbose;#endif#define	LOCALDEBUG			0////	For ease, the raster number is set in VIC.c//UInt32	gRasterNumberToInterrupt=0;extern	UInt32	gCurrentRaster;extern	unsigned	char	gVerticalScroll;extern	unsigned	char	gVICReg[];extern	RasteringFunction	gCurrentRasteringFunction;unsigned	char	gVICInterruptRequestLine=0;unsigned	char	gVICIntEnabledMask=0;unsigned	char	gVICIntFlags=0;#define	PAL	1#if	PALUInt32	gNumberofVerticalRasters=311;#elseUInt32	gNumberofVerticalRasters=261;#endifextern	unsigned char 	*		theActiveBits;extern	UInt32						theActiveRowBytes;extern	unsigned char 	*		originaltheActiveBits;extern	SpritePtr	spriteRasters[256][16];void	VICRasterIncrement(){	static	int doIfire=1;		gCurrentRaster ++;		if (gCurrentRaster> gNumberofVerticalRasters)		{			#if	DRAWOFFSCREEN			if (doIfire)				CopyOffScreenToWindow();			#endif						if (doIfire)				doIfire=0;			else				doIfire=1;			gCurrentRaster=0;		}	if (gRasterNumberToInterrupt==gCurrentRaster )	//check to see if our rasters line up	{		VICPullIntLine(0x01);	}	if (doIfire /*(doIfire)^(gCurrentRaster&0x01)*/)	{		if (gCurrentRaster > 50 && gCurrentRaster < 251)			gCurrentRasteringFunction(gCurrentRaster-51);	}}////	This function is called whenever an interrupt condition occurs.//	It worries about checking the vic interrupt flags, and actually//	interrupting the processor//void	VICPullIntLine(unsigned	char	mask){		gVICIntFlags|=mask;	gVICIntFlags|=0x80;	if (gVICIntEnabledMask&mask)		{		//do interrupt thing as defined by ed..			gVICInterruptRequestLine=1;//		debug_window_printf("interrupting processor");		if (IRQInterrupt())				gVICInterruptRequestLine = 0;		}}void	VICWriteD019 (unsigned long val){					/*					VIC Interrupt Flag Register (Bit = 1: IRQ Occured)								A write of one clears the bit						7	Set on Any Enabled VIC IRQ Condition					�						3	Light-Pen Triggered IRQ Flag								�						2	Sprite to Sprite Collision IRQ Flag						�						1	Sprite to Background Collision IRQ Flag				�						0	Raster Compare IRQ Flag										�					*/					if (val&0x80)						gVICIntFlags&=~0x80;					if (val&0x08)						gVICIntFlags&=~0x08;					if (val&0x04)						gVICIntFlags&=~0x04;					if (val&0x02)						gVICIntFlags&=~0x02;					if (val&0x01)						gVICIntFlags&=~0x01;					//we need to clear the interrupt request line..					//does bit 7 effect it differently?  If they clear 7 do we lower the int line?					//if they clear all but 7, do we clear 7?					if (!gVICIntFlags)						gVICInterruptRequestLine=0;					else						gVICIntFlags|=0x80;}void	VICWriteD01A(unsigned long val){				/*						IRQ Mask Register: 1 = Interrupt Enabled						7	no connect	according to Mastering the C64										6	no connect						5	no connect						4	no connect						3	Light-Pen Triggered IRQ Flag								�						2	Sprite to Sprite Collision IRQ Flag						�						1	Sprite to Background Collision IRQ Flag				�						0	Raster Compare IRQ Flag										�					*//*					if (val&0x80)						{	#if	LOCALDEBUG && DEBUG						debug_window_printf("VICInterrupts enabled");	#endif						gVICIntEnabledMask|=0x80;						}*/					if (val&0x08)						gVICIntEnabledMask|=0x08;					if (val&0x04)						{	#if	LOCALDEBUG && DEBUG						debug_window_printf("Sprite Collision Enable");	#endif						gVICIntEnabledMask|=0x04;						}					if (val&0x02)						gVICIntEnabledMask|=0x02;					if (val&0x01)						{	#if	LOCALDEBUG && DEBUG						debug_window_printf("Raster Compare int enable");						debug_window_printf("gRasterNumberToInterrupt=%d",gRasterNumberToInterrupt);	#endif						gVICIntEnabledMask|=0x01;						}}unsigned	long	VICReadD019(){return gVICIntFlags;}unsigned	long	VICReadD01A(){return gVICIntEnabledMask;}