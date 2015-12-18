/*-------------------------------------------------------------------------------*\||	File:	VICSprites.c||	Description:||		||||	Copyright �	1994, Alan Steremberg and Ed Wynne|\*-------------------------------------------------------------------------------*/#include	"DebugWindow.h"#include	"Drawing.h"#include	"DrawingGlobals.h"#include	"VICInterrupts.h"#include	"VICSprites.h"#if	DEBUGextern	Boolean	gVerbose;#endifextern	unsigned char	gVICReg[64];////	From ColorDrawing.c//extern	unsigned long		gPixelDepth;unsigned	char	oneSpriteRasterColl[700];////	From Memory.c//extern	unsigned	char	*gRAMBlock;extern	unsigned	char	*gVideoScreenRamBase;extern	unsigned	char 	*gVideoBankRamBase;unsigned	char	gSpriteToSpriteCollisions=0;SpriteData	gSpriteInfo[8]={0};////	The ninth position keeps track of whether there are any sprites to be drawn////SpritePtr	spriteRasters[256][8]={0};//short			gSpriteOnRaster[256];SpritePtr	spriteRasters[256][8]={0};short			gSpriteOnRaster[256];void	SpriteSetYPosition (short	sprNum){	register	SpritePtr	cachegspriteinfo;	unsigned	char	i;	UInt32	j;	UInt32	spriteNum=sprNum;	unsigned	char	temp;	//	//	cache this to make it faster..	//			cachegspriteinfo=&gSpriteInfo[spriteNum];		//	//	Clear Old Position From Raster Structure	//		if (cachegspriteinfo->expandVert)		{			for (i=cachegspriteinfo->bounds.top, j=0;j < 42; j++, i++)			{				if (spriteRasters[i][spriteNum]!=NULL)				{					gSpriteOnRaster[i]--;					spriteRasters[i][spriteNum]=NULL;							}			}		}		else		{			for (i=cachegspriteinfo->bounds.top, j=0;j < 21; j++, i++)			{				if (spriteRasters[i][spriteNum]!=NULL)				{					gSpriteOnRaster[i]--;					spriteRasters[i][spriteNum]=NULL;							}			}		}	//	//	Set New Position	//	if (gVICReg[0x17]&(1<<spriteNum))		gSpriteInfo[spriteNum].expandVert=TRUE;	else			gSpriteInfo[spriteNum].expandVert=FALSE;	cachegspriteinfo->bounds.top=(unsigned char)gVICReg[(spriteNum+spriteNum)+1];	temp=cachegspriteinfo->bounds.top+21+(cachegspriteinfo->expandVert?21:0);	cachegspriteinfo->bounds.bottom=temp;		//	//	Set New Position in Raster Structure	// if it is already enabled..	if (gVICReg[0x15]&(1<<spriteNum))	{		if (cachegspriteinfo->expandVert)		{			for (i=cachegspriteinfo->bounds.top, j=0;j < 42; j++, i++)			{				gSpriteOnRaster[i]++;				spriteRasters[i][spriteNum]=cachegspriteinfo;			}		}		else		{			for (i=cachegspriteinfo->bounds.top, j=0;j < 21; j++, i++)			{				gSpriteOnRaster[i]++;				spriteRasters[i][spriteNum]=cachegspriteinfo;			}		}				}}void	SpriteSetXPosition (short	sprNum){	register	SpritePtr	cachegspriteinfo;	SInt32	spriteNum=sprNum;	register	unsigned	short	temp;	if (spriteNum > 7 || spriteNum < 0)		return;		//	//	cache this to make it faster..	//			cachegspriteinfo=&gSpriteInfo[spriteNum];	//	//	Set New Position	//		temp=cachegspriteinfo->bounds.left=(gVICReg[(spriteNum)<<1] |  ((gVICReg[0x10]<<(8-spriteNum))&0x0100));	cachegspriteinfo->bounds.right=temp+24+(cachegspriteinfo->expandHoriz?24:0);}void	SpriteEnableDisable (){	UInt32 spriteNum;	unsigned	char	i;	UInt32	j;	register	SpritePtr	cachegspriteinfo;//	short ack=0;		for (spriteNum=0;spriteNum<8;spriteNum++)		{		//		//	If our sprite is enabled, be sure it is added to Raster Structure		//		cachegspriteinfo=&gSpriteInfo[spriteNum];		if (gVICReg[0x15]&(1<<spriteNum))			{			//			//	Set  Position in Raster Structure			//			if (cachegspriteinfo->expandVert)			{				for (i=cachegspriteinfo->bounds.top, j=0;j < 42; j++, i++)				{					gSpriteOnRaster[i]++;					spriteRasters[i][spriteNum]=cachegspriteinfo;				}			}			else			{				for (i=cachegspriteinfo->bounds.top, j=0;j < 21; j++, i++)				{					gSpriteOnRaster[i]++;					spriteRasters[i][spriteNum]=cachegspriteinfo;				}			}/*			for (i=cachegspriteinfo->bounds.top; i <cachegspriteinfo->bounds.bottom && i<255;i++)				{				if (spriteRasters[i][spriteNum]==NULL)					{					spriteRasters[i][spriteNum]=&gSpriteInfo[spriteNum];					gSpriteOnRaster[i]++;					}				}*/						}		//		//	if our sprite is disabled, be sure it is removed from Raster Structure		//		else			{			//			//	Clear Position From Raster Structure			//			if (cachegspriteinfo->expandVert)			{				for (i=cachegspriteinfo->bounds.top, j=0;j < 42; j++, i++)				{					if (spriteRasters[i][spriteNum]!=NULL)					{						gSpriteOnRaster[i]--;						spriteRasters[i][spriteNum]=NULL;								}				}			}			else			{				for (i=cachegspriteinfo->bounds.top, j=0;j < 21; j++, i++)				{					if (spriteRasters[i][spriteNum]!=NULL)					{						gSpriteOnRaster[i]--;						spriteRasters[i][spriteNum]=NULL;								}				}			}/*			for (i=cachegspriteinfo->bounds.top; i <cachegspriteinfo->bounds.bottom&& i<255;i++)				{				if (spriteRasters[i][spriteNum]!=NULL)					{					gSpriteOnRaster[i]--;					spriteRasters[i][spriteNum]=NULL;								}							}*/			}		}//if (gVerbose)	//debug_window_printf("enabled %d sprites",ack);}void	SpriteSetBkgDisplayPriority (){	UInt32 spriteNum;		for (spriteNum=0;spriteNum<8;spriteNum++)		gSpriteInfo[spriteNum].sprOverBkgPriority=gVICReg[0x1B]&(1<<spriteNum);	}void	SpriteCollideSpriteToSprite (unsigned char	sprites){//	if (gSpriteToSpriteCollisions)//		debug_window_printf("unnecessary collision");	VICPullIntLine(0x04);		//cause sprite-sprite interrupt	gSpriteToSpriteCollisions|=sprites;	}////	will this depend on bit depth????//#define	COLLISIONS	1void	SpriteDrawOneRaster(short	rasterline, unsigned char *dstImagePosition){	UInt32	spriteNum;	UInt32	multicolorarr[4];	unsigned	char	collidedSprites=0;			//		//	Build up our multicolor tables, they are filled with longs so we can pull		//	tricks to do pixel replication horizontally.. 		//		multicolorarr[0]=0x00000000;		multicolorarr[1]=eightBitTable[gVICReg[0x25]];//		multicolorarr[2]=eightBitTable[gVICReg[0x27+spriteNum]];	//This needs to be filled out based on the sprite..		multicolorarr[3]=eightBitTable[gVICReg[0x26]];				{register	UInt32	i;register	UInt32	*sprlongcollarr=(UInt32*)oneSpriteRasterColl;for (i=0;i<8;i++)	{	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	*(sprlongcollarr++)=0x00000000;	}}				//SpritePtr	spriteRasters[256][8]={0};	//	//	Plot each sprite in reverse order of precedence	//		{	SpritePtr	*cachespr=spriteRasters[rasterline];		for (spriteNum=7;spriteNum>=0;--spriteNum)		{		register	SpritePtr	cachespriter=cachespr[spriteNum];		//		//	check to see if this sprite exists on this rasterline		//		if (cachespriter)			{			short					left;			short					linenum;			unsigned	char		*data;			Boolean				Double;			UInt32		bkgPriority;			unsigned char		backColor=(unsigned char)gAndBackColor[0];			//			// %%% add sprite over background priority later.. && background collisions						left=cachespriter->bounds.left;			bkgPriority=cachespriter->sprOverBkgPriority;////	----------------------	----------------------// |                   	|	|							|//	| --cur raster line 40	|	|	--top	of sprite	|// |     					|	|	--cur raster		|//	| --bottom or sprite 41 |	|	--bottom of sprite|//	|							|	|							|//	|							|	|							|//	----------------------	----------------------////	--top of sprite	FF			if (cachespriter->expandVert)			{				linenum=(rasterline-cachespriter->bounds.top) >> 1;//faster equivalent		//		if (linenum < 0)		//			DebugStr("\pYuck");		//		if (linenum > 41)		//			DebugStr("\pYuck");			}			else			{				linenum=rasterline-cachespriter->bounds.top;		//		if (linenum < 0)		//			DebugStr("\pYuck");		//		if (linenum > 20)			//		DebugStr("\pYuck");			}				data=gVideoBankRamBase+( gVideoScreenRamBase[1016+spriteNum]<<6)+(linenum+linenum+linenum);			if (cachespriter->expandHoriz)				Double=TRUE;			else				Double=FALSE;			if (cachespriter->multiColor)				{				UInt32	i;				multicolorarr[2]=eightBitTable[gVICReg[0x27+spriteNum]];	//The rest of the table is filled out once at the top of the function				//	A sprite is 3 data bytes wide									if (Double)	//check to see if we should double size the sprite//----------------------------------------------////		MultiColor Doubled Sprite Drawing Routines////----------------------------------------------						{						register	UInt32	*sprdataptr;									UInt32	*leftdataptr;									UInt32	*rightdataptr;						register	unsigned	char	*cachesprcollarr=&oneSpriteRasterColl[left];										sprdataptr=(UInt32 *)&dstImagePosition[left-24];						leftdataptr=(UInt32 *)dstImagePosition;						rightdataptr=(UInt32 *)dstImagePosition+80;												for (i=0;i<3;i++)							{							register	UInt32	data_src=(long)*data++;							register	UInt32	temp;												temp=data_src>>6;							if (temp)								{								UInt32	k;								if (sprdataptr >= leftdataptr && sprdataptr <= rightdataptr)									if (!bkgPriority)										*sprdataptr=multicolorarr[temp];									else									{										if (backColor==*sprdataptr)											*sprdataptr=multicolorarr[temp];									}										// we need to calculate collision detection for each pixel								// this isn't efficient, there is probably a better way to do it#if	COLLISIONS																for (k=0;k<4;k++)									{									if (*cachesprcollarr)										{										*cachesprcollarr|=1<<spriteNum;										if (sprdataptr >= leftdataptr && sprdataptr <= rightdataptr)											*sprdataptr=multicolorarr[temp];										collidedSprites|=*cachesprcollarr;										}									else												*cachesprcollarr|=1<<spriteNum;									}#endif								}														sprdataptr++;							cachesprcollarr++;														temp=data_src>>4 & 0x03;							if (temp)								{								UInt32	k;								if (sprdataptr >= leftdataptr && sprdataptr <= rightdataptr)									if (!bkgPriority)                                    {										*sprdataptr=multicolorarr[temp];                                    }									else									{										if (backColor==*sprdataptr)											*sprdataptr=multicolorarr[temp];									}									// we need to calculate collision detection for each pixel								// this isn't efficient, there is probably a better way to do it#if	COLLISIONS																for (k=0;k<4;k++)									{									if (*cachesprcollarr)										{										if (sprdataptr >= leftdataptr && sprdataptr <= rightdataptr)											*sprdataptr=multicolorarr[temp];										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										}									else												*cachesprcollarr|=1<<spriteNum;									}#endif								}														sprdataptr++;							cachesprcollarr++;							temp=data_src>>2 & 0x03;													if (temp)								{								UInt32	k;								if (sprdataptr >= leftdataptr && sprdataptr <= rightdataptr)									if (!bkgPriority)										*sprdataptr=multicolorarr[temp];									else									{										if (backColor==*sprdataptr)											*sprdataptr=multicolorarr[temp];									}									// we need to calculate collision detection for each pixel								// this isn't efficient, there is probably a better way to do it#if	COLLISIONS																for (k=0;k<4;k++)									{									if (*cachesprcollarr)										{										if (sprdataptr >= leftdataptr && sprdataptr <= rightdataptr)											*sprdataptr=multicolorarr[temp];										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										}									else												*cachesprcollarr|=1<<spriteNum;									}#endif								}														sprdataptr++;							cachesprcollarr++;							temp=data_src&0x03;													if (temp)								{								UInt32	k;								if (sprdataptr >= leftdataptr && sprdataptr <= rightdataptr)									if (!bkgPriority)										*sprdataptr=multicolorarr[temp];									else									{										if (backColor==*sprdataptr)											*sprdataptr=multicolorarr[temp];									}									// we need to calculate collision detection for each pixel								// this isn't efficient, there is probably a better way to do it#if	COLLISIONS																for (k=0;k<4;k++)									{									if (*cachesprcollarr)										{										if (sprdataptr >= leftdataptr && sprdataptr <= rightdataptr)											*sprdataptr=multicolorarr[temp];										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										}									else												*cachesprcollarr|=1<<spriteNum;									}#endif								}														sprdataptr++;							cachesprcollarr++;							}						}					else//--------------------------------------------------////		MultiColor Non Doubled Sprite Drawing Routines////--------------------------------------------------						{						register	unsigned	short	*sprshortdataptr;						register	unsigned	char	*cachesprcollarr=&oneSpriteRasterColl[left];									unsigned	short	*leftdataptr;									unsigned	short	*rightdataptr;															sprshortdataptr=(unsigned	short *)&dstImagePosition[left-24];						leftdataptr=(unsigned short *)dstImagePosition;						rightdataptr=(unsigned short *)dstImagePosition+160;						for (i=0;i<3;i++)							{							register	UInt32	data_src=(UInt32)data[i];							register	UInt32	temp;//---	1							temp=data_src>>6;							if (temp)								{								if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)									if (!bkgPriority)										*sprshortdataptr=multicolorarr[temp];									else									{										if (backColor==*sprshortdataptr)											*sprshortdataptr=multicolorarr[temp];									}										// we need to calculate collision detection for each pixel									// this isn't efficient, there is probably a better way to do it#if	COLLISIONS									if (*cachesprcollarr)										{										if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)											*sprshortdataptr=multicolorarr[temp];										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										}									else											*cachesprcollarr|=1<<spriteNum;									cachesprcollarr++;									if (*cachesprcollarr)										{										if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)											*sprshortdataptr=multicolorarr[temp];										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										}										else										*cachesprcollarr|=1<<spriteNum;									cachesprcollarr++;									#endif								}							else								{								cachesprcollarr+=2;								}							sprshortdataptr++;//---	2							temp=data_src>>4 & 0x03;							if (temp)								{								if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)									if (!bkgPriority)										*sprshortdataptr=multicolorarr[temp];									else									{										if (backColor==*sprshortdataptr)											*sprshortdataptr=multicolorarr[temp];									}										// we need to calculate collision detection for each pixel									// this isn't efficient, there is probably a better way to do it#if	COLLISIONS									if (*cachesprcollarr)										{										if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)											*sprshortdataptr=multicolorarr[temp];										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										}									else											*cachesprcollarr|=1<<spriteNum;									cachesprcollarr++;									if (*cachesprcollarr)										{										if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)											*sprshortdataptr=multicolorarr[temp];										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										}										else										*cachesprcollarr|=1<<spriteNum;									cachesprcollarr++;#endif								}							else								{								cachesprcollarr+=2;								}							sprshortdataptr++;//---	3							temp=data_src>>2 & 0x03;							if (temp)								{								if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)									if (!bkgPriority)										*sprshortdataptr=multicolorarr[temp];									else									{										if (backColor==*sprshortdataptr)											*sprshortdataptr=multicolorarr[temp];									}											// we need to calculate collision detection for each pixel									// this isn't efficient, there is probably a better way to do it#if	COLLISIONS									if (*cachesprcollarr)										{										if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)											*sprshortdataptr=multicolorarr[temp];										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										}									else											*cachesprcollarr|=1<<spriteNum;									cachesprcollarr++;									if (*cachesprcollarr)										{										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)											*sprshortdataptr=multicolorarr[temp];										}										else										*cachesprcollarr|=1<<spriteNum;									cachesprcollarr++;#endif								}							else								{								cachesprcollarr+=2;								}							sprshortdataptr++;//---	4							temp=data_src&0x03;							if (temp)								{									if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)										if (!bkgPriority)											*sprshortdataptr=multicolorarr[temp];										else										{											if (backColor==*sprshortdataptr)												*sprshortdataptr=multicolorarr[temp];										}										// we need to calculate collision detection for each pixel									// this isn't efficient, there is probably a better way to do it#if	COLLISIONS									if (*cachesprcollarr)										{										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)											*sprshortdataptr=multicolorarr[temp];										}									else											*cachesprcollarr|=1<<spriteNum;									cachesprcollarr++;									if (*cachesprcollarr)										{										*cachesprcollarr|=1<<spriteNum;										collidedSprites|=*cachesprcollarr;										if (sprshortdataptr >= leftdataptr && sprshortdataptr <= rightdataptr)											*sprshortdataptr=multicolorarr[temp];										}										else										*cachesprcollarr|=1<<spriteNum;									cachesprcollarr++;#endif								}							else								{								cachesprcollarr+=2;								}							sprshortdataptr++;							}							}					}			else			{				if (Double)//--------------------------------------------------////		Regular Doubled Sprite Drawing Routines////--------------------------------------------------					{					register	UInt32	i;					register	unsigned	char	*sprchardataptr;					register	unsigned	char	*cachesprcollarr=&oneSpriteRasterColl[left];					register	unsigned	char	color=eightBitTable[gVICReg[0x27+spriteNum]];								unsigned	char	*leftdataptr;								unsigned	char	*rightdataptr;					sprchardataptr=&dstImagePosition[left-24];					leftdataptr=dstImagePosition;					rightdataptr=dstImagePosition+320;					for (i=0;i<3;i++)							{											register	UInt32	data_src=(UInt32)*data++;						if (data_src&0x80)						{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}							else									*cachesprcollarr|=1<<spriteNum;															cachesprcollarr++;							sprchardataptr++;														if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}								else								*cachesprcollarr|=1<<spriteNum;							cachesprcollarr++;							sprchardataptr++;						}						else							{							cachesprcollarr+=2;							sprchardataptr+=2;							}										if (data_src&0x40)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}							else									*cachesprcollarr|=1<<spriteNum;															cachesprcollarr++;							sprchardataptr++;														if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}								else								*cachesprcollarr|=1<<spriteNum;							cachesprcollarr++;							sprchardataptr++;						}						else							{							sprchardataptr+=2;							cachesprcollarr+=2;							}													if (data_src&0x20)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}							else									*cachesprcollarr|=1<<spriteNum;															cachesprcollarr++;							sprchardataptr++;														if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}								else								*cachesprcollarr|=1<<spriteNum;							cachesprcollarr++;							sprchardataptr++;						}						else							{							sprchardataptr+=2;							cachesprcollarr+=2;							}							if (data_src&0x10)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}							else									*cachesprcollarr|=1<<spriteNum;															cachesprcollarr++;							sprchardataptr++;														if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}								else								*cachesprcollarr|=1<<spriteNum;							cachesprcollarr++;							sprchardataptr++;						}						else							{							sprchardataptr+=2;							cachesprcollarr+=2;							}													if (data_src&0x08)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}							else									*cachesprcollarr|=1<<spriteNum;															cachesprcollarr++;							sprchardataptr++;														if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}								else								*cachesprcollarr|=1<<spriteNum;							cachesprcollarr++;							sprchardataptr++;						}						else							{							sprchardataptr+=2;							cachesprcollarr+=2;							}														if (data_src&0x04)						{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}							else									*cachesprcollarr|=1<<spriteNum;															cachesprcollarr++;							sprchardataptr++;														if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}								else								*cachesprcollarr|=1<<spriteNum;							cachesprcollarr++;							sprchardataptr++;						}						else							{							sprchardataptr+=2;							cachesprcollarr+=2;							}													if (data_src&0x02)						{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}							else									*cachesprcollarr|=1<<spriteNum;															cachesprcollarr++;							sprchardataptr++;														if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}								else								*cachesprcollarr|=1<<spriteNum;							cachesprcollarr++;							sprchardataptr++;						}						else							{							sprchardataptr+=2;							cachesprcollarr+=2;							}							if (data_src&0x01)						{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}							else									*cachesprcollarr|=1<<spriteNum;															cachesprcollarr++;							sprchardataptr++;														if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)							{								if (!bkgPriority)									*(sprchardataptr)=color;								else if (backColor==*sprchardataptr)										*(sprchardataptr)=color;							}							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								collidedSprites|=*cachesprcollarr;								}								else								*cachesprcollarr|=1<<spriteNum;							cachesprcollarr++;							sprchardataptr++;						}						else							{							sprchardataptr+=2;							cachesprcollarr+=2;							}												}				}				else//--------------------------------------------------////		Regular NonDoubled Sprite Drawing Routines////--------------------------------------------------					{					register	UInt32	i;					register	unsigned	char	*sprchardataptr;					register	unsigned	char	*cachesprcollarr=&oneSpriteRasterColl[left];					register	unsigned	char	color=eightBitTable[gVICReg[0x27+spriteNum]];								unsigned	char	*leftdataptr;								unsigned	char	*rightdataptr;//Debugger();					sprchardataptr=&dstImagePosition[left-24];					leftdataptr=dstImagePosition;					rightdataptr=dstImagePosition+320;					for (i=0;i<3;i++)							{											register	UInt32	data_src=(UInt32)*data++;						if (data_src&0x80)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)								if (!bkgPriority)								{									*(sprchardataptr)=color;								}									else								{									if (backColor==*sprchardataptr)									{										*(sprchardataptr)=color;									}								}#if	COLLISIONS							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								collidedSprites|=*cachesprcollarr;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								}							else									*cachesprcollarr|=1<<spriteNum;#endif						}						cachesprcollarr++;						sprchardataptr++;						if (data_src&0x40)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)								if (!bkgPriority)								{									*(sprchardataptr)=color;								}									else								{									if (backColor==*sprchardataptr)									{										*(sprchardataptr)=color;									}								}#if	COLLISIONS							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								collidedSprites|=*cachesprcollarr;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								}								else								*cachesprcollarr|=1<<spriteNum;#endif						}						sprchardataptr++;						cachesprcollarr++;												if (data_src&0x20)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)								if (!bkgPriority)								{									*(sprchardataptr)=color;								}									else								{									if (backColor==*sprchardataptr)									{										*(sprchardataptr)=color;									}								}#if	COLLISIONS							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								collidedSprites|=*cachesprcollarr;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								}								else								*cachesprcollarr|=1<<spriteNum;#endif						}												sprchardataptr++;						cachesprcollarr++;						if (data_src&0x10)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)								if (!bkgPriority)								{									*(sprchardataptr)=color;								}									else								{									if (backColor==*sprchardataptr)									{										*(sprchardataptr)=color;									}								}#if	COLLISIONS							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								collidedSprites|=*cachesprcollarr;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								}								else								*cachesprcollarr|=1<<spriteNum;#endif						}						sprchardataptr++;						cachesprcollarr++;												if (data_src&0x08)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)								if (!bkgPriority)								{									*(sprchardataptr)=color;								}									else								{									if (backColor==*sprchardataptr)									{										*(sprchardataptr)=color;									}								}#if	COLLISIONS							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								collidedSprites|=*cachesprcollarr;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								}								else								*cachesprcollarr|=1<<spriteNum;#endif						}												sprchardataptr++;						cachesprcollarr++;													if (data_src&0x04)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)								if (!bkgPriority)								{									*(sprchardataptr)=color;								}									else								{									if (backColor==*sprchardataptr)									{										*(sprchardataptr)=color;									}								}#if	COLLISIONS							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								collidedSprites|=*cachesprcollarr;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								}							else									*cachesprcollarr|=1<<spriteNum;#endif						}						sprchardataptr++;						cachesprcollarr++;												if (data_src&0x02)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)								if (!bkgPriority)								{									*(sprchardataptr)=color;								}									else								{									if (backColor==*sprchardataptr)									{										*(sprchardataptr)=color;									}								}#if	COLLISIONS							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								collidedSprites|=*cachesprcollarr;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								}								else								*cachesprcollarr|=1<<spriteNum;#endif						}																				sprchardataptr++;						cachesprcollarr++;						if (data_src&0x01)							{							if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)								if (!bkgPriority)								{									*(sprchardataptr)=color;								}									else								{									if (backColor==*sprchardataptr)									{										*(sprchardataptr)=color;									}								}#if	COLLISIONS							if (*cachesprcollarr)								{								*cachesprcollarr|=1<<spriteNum;								collidedSprites|=*cachesprcollarr;								if (sprchardataptr >= leftdataptr && sprchardataptr <= rightdataptr)									*(sprchardataptr)=color;								}								else								*cachesprcollarr|=1<<spriteNum;#endif						}													sprchardataptr++;						cachesprcollarr++;											}				}			}		}		}	}	////	Pull Interrupt Line If we had a collision//	if (collidedSprites)	SpriteCollideSpriteToSprite(collidedSprites);	}