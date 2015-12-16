/*-------------------------------------------------------------------------------*\
|
|	File:	CombugWindow.c
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
#include <string.h>
#include	<stdarg.h>
#include <Carbon/Carbon.h>
#include "DebugWindow.h"
#include "Preference.h"
#include "Window.h"
#include "Menu.h"
#include	"Combug.h"
#include	"CombugConsole.h"
#include "CombugWindow.h"



#if 0

/*	local constants	*/
#define	kVLineOffset						10
#define	kBottomOffset						gHeight-4
#define	kDisAssemLines						8
#define	kNumScrollBackLines				(((kBottomOffset-kVLineOffset*kDisAssemLines-2)/kVLineOffset) -1 )
#define	kScrollBackBufferSize			200
#define	kScrollBackWidth					70


/*	external globals	*/
extern char						gCommandLineBuffer[kCommandLineBufSize]; //CombugEntry.c
extern unsigned	short		gCLInsertionPoint;				//CombugEntry.c
extern WindowPtr				gCombugWindow;
extern GWorldPtr				gCombugWorld;
extern short					gHeight;
extern short					gWidth;


/*	local globals	*/
Ptr						gCombugScrollBackBuffer;
unsigned	long			gCombugTotalScrollBackLines;
unsigned	long			gCombugScrollValue;





void	CombugDrawCommandLine(char	*CommandLine)
{
	GDHandle	gdh;
	CGrafPtr	port;
	
	
	GetGWorld(&port,&gdh);
	SetGWorld(gCombugWorld,NULL);
	
	TextFont(4);
	TextSize(9);
	TextMode(srcCopy);
		
	MoveTo(60,gHeight-4);
	CombugPrintf("%s",CommandLine);
	
//	TextMode(srcOr);
	SetGWorld(port,gdh);
	GetPort((GrafPtr*)&port);
	SetPortWindowPort(gCombugWindow);

// If you want, kill these lines that draw directly to the screen

	TextFont(4);
	TextSize(9);
	TextMode(srcCopy);
	MoveTo(60,gHeight-4);
	CombugPrintf("%s",CommandLine);
//	TextMode(srcOr);

//	CombugWindowUpdateProc(gCombugWindow,GetWindowRefCon(gCombugWindow));
	SetPort((GrafPtr)port);


}

void	CombugDrawInsertionPoint(Boolean	forceerase)
{
	FontInfo	fInfo;
	GrafPtr	oldPort;
	static	long	time=0;

	if (gCombugWindow==NULL)
		return;

	//if (forceerase || (TickCount() - time > LMGetCaretTime()))
	if (forceerase || (TickCount() - time > GetCaretTime()))
	{
		//find the width
		GetPort(&oldPort);
		SetPortWindowPort(gCombugWindow);
	
		TextFont(4);
		TextSize(9);
		GetFontInfo(&fInfo);
		fInfo.widMax;

		if (forceerase)
			{
			Pattern WhitePat;
			GetQDGlobalsWhite(&WhitePat);

			PenPat(&WhitePat);
			PenMode(srcCopy);
			}
		else
			{
			Pattern BlackPat;
			GetQDGlobalsBlack(&BlackPat);

//			PenMode(srcXor);
			PenPat(&BlackPat);
			}
		MoveTo(gCLInsertionPoint*fInfo.widMax+59,gHeight-kVLineOffset-1);
		LineTo(gCLInsertionPoint*fInfo.widMax+59,gHeight-3);
		SetPort(oldPort);
		if (!forceerase)
			time=TickCount();
	}
}

void	CombugClearCommandLine(void)
{
	GDHandle	gdh;
	CGrafPtr	port;
	Rect	erRect;
	
	SetRect(&erRect,60,gHeight-kVLineOffset-3,gWidth,gHeight);
	
	GetGWorld(&port,&gdh);
	SetGWorld(gCombugWorld,NULL);
	
	EraseRect(&erRect);
	
//	TextMode(srcOr);
	SetGWorld(port,gdh);
	GetPort((GrafPtr*)&port);
	SetPortWindowPort(gCombugWindow);

// If you want, kill these lines that draw directly to the screen

	EraseRect(&erRect);

//	CombugWindowUpdateProc(gCombugWindow,GetWindowRefCon(gCombugWindow));
	SetPort((GrafPtr)port);

	
}

void CombugScrollPrintf(char *format,...)
{
		char		string[512];
		va_list	argptr;
		long		len;
//		GrafPtr	savedPort;
		
		
		va_start(argptr,format);
		len = (long)vsprintf(string,format,argptr);
		va_end (argptr);
		
		if (len > kScrollBackWidth-2)
			len = kScrollBackWidth-2;
		
		gCombugTotalScrollBackLines++;

		BlockMoveData(gCombugScrollBackBuffer,gCombugScrollBackBuffer+kScrollBackWidth,kScrollBackWidth*(kScrollBackBufferSize-1));
		
		if (gCombugTotalScrollBackLines > kScrollBackBufferSize)
			gCombugTotalScrollBackLines=kScrollBackBufferSize;
		
		strcpy(gCombugScrollBackBuffer+1,string);
		memset(gCombugScrollBackBuffer+1+len,' ',kScrollBackWidth-1-len);
		*(gCombugScrollBackBuffer) = kScrollBackWidth-2;
				
		gCombugScrollValue=0;
		
		CombugDrawScrollBackRegion();

}


void	CombugClearScrollBackRegion(void)
{
	Rect	erRect;
	SetRect(&erRect,60,0,gWidth,gHeight-kVLineOffset*(kDisAssemLines+1)-8);
	EraseRect(&erRect);
}

void	CombugDrawScrollBackRegion(void)
{
	long			offset;
	short			i,j;
	GDHandle		gdh;
	CGrafPtr		port;
	short			scrollbackbottom;
	short			top;

	GetGWorld(&port,&gdh);
	SetGWorld(gCombugWorld,NULL);
	
	// AJS 
	//TextFont(monaco);
	TextSize(9);
	TextMode(srcCopy);
		
	scrollbackbottom=gHeight-kVLineOffset*(kDisAssemLines+1)-12;

	i=gCombugScrollValue;

	top=gCombugScrollValue+kNumScrollBackLines;
	if (top > gCombugTotalScrollBackLines)
		top=gCombugTotalScrollBackLines;

	for (j=0;i<top;i++,j++)
		{
			offset = (kScrollBackWidth*(i));

			MoveTo(60, scrollbackbottom- (j*(kVLineOffset)));
			DrawString((StringPtr)gCombugScrollBackBuffer+offset);
		}


	SetGWorld(port,gdh);


	GetPort((GrafPtr*)&port);
	SetPortWindowPort(gCombugWindow);
	CombugWindowUpdateProc(gCombugWindow,GetWindowRefCon(gCombugWindow));
	SetPort((GrafPtr)port);

}

void CombugMoveScrollBackUp(void)
{
if (gCombugTotalScrollBackLines > kNumScrollBackLines)
	if (gCombugScrollValue < (1+gCombugTotalScrollBackLines-kNumScrollBackLines))
		{
		gCombugScrollValue++;
		DrawCombugWindow();
		}

}
void CombugMoveScrollBackDown(void)
{
			if (gCombugScrollValue)
			{
				gCombugScrollValue--;
				DrawCombugWindow();
			}
}

#endif
