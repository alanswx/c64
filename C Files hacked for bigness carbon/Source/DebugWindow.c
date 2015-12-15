/*-------------------------------------------------------------------------------*\
|
|	File:	DebugWindow.c
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
#include "Window.h"
#include "Preference.h"
#include "DebugWindow.h"

#include <SDL2/SDL.h>




/*	local constants	*/
#define	kScrollBackBufferSize		1200
#define	kDebugWindowLocPref			'DbgL'
#define	kDebugWindowLocPrefID		128
#define	kDebugWindowVisiblePref		'DbgV'
#define	kDebugWindowVisiblePrefID	128

/*	local globals */
#if		DEBUG
	WindowPtr			gDebugWindow = NULL;
	ControlHandle		gDebugScrollBar = NULL;
	char					*gTextBuffer = NULL;
	long					gTotalLines;
	ControlActionUPP	gDebugWindowClickUPP=NULL;
#endif	DEBUG





void InitDebugWindow(void)
{	
	#if	DEBUG
		Handle	data;
		Rect		crect;
		Point		where;
		short		visible;
		Rect		bounds;
		

		gDebugWindowClickUPP=NewControlActionUPP(DebugScrollBarActionProc);
		assert(gDebugWindowClickUPP!=NULL);
		
		gDebugWindow = GetNewWindow(1000,NULL,(WindowPtr)-1L);
		assert(gDebugWindow!=NULL);

		data = GetPreference(kDebugWindowLocPref,kDebugWindowLocPrefID);
		if (data)
		{
			BlockMove(*data,&where,sizeof(Point));
			DisposeHandle(data);
			
			MoveWindow(gDebugWindow,where.h,where.v,false);
		}
		
		NewWindowInfo(gDebugWindow);
		SetWindowCloseProc(gDebugWindow,DebugWindowCloseProc);
		SetWindowUpdateProc(gDebugWindow,DebugWindowUpdateProc);
		SetWindowClickProc(gDebugWindow,DebugWindowClickProc);
		
		GetWindowPortBounds(gDebugWindow, &bounds);

		
		SetRect(&crect,-15,-1,1,bounds.bottom+1);
		OffsetRect(&crect,bounds.right-bounds.left,0);
		gDebugScrollBar = NewControl(gDebugWindow,&crect,"\p",true,0,0,0,scrollBarProc,0L);
		assert(gDebugScrollBar!=NULL);
		
		gTextBuffer = (char*)NewPtrClear(62*kScrollBackBufferSize);
		assert(gTextBuffer!=NULL);
		gTotalLines = 0;
		
		data = GetPreference(kDebugWindowVisiblePref,kDebugWindowVisiblePrefID);
		if (data)
		{
			BlockMove(*data,&visible,sizeof(short));
			DisposeHandle(data);
			
			if (visible)
				ShowDebugWindow();
		}
	#endif	DEBUG
}





void CleanUpDebugWindow(void)
{
	#if	DEBUG
		GrafPtr	savedPort;
		Handle	data;
		Point		where;
		short		visible;
		Rect		bounds;
		
		
		if (gTextBuffer)
			DisposePtr((char*)gTextBuffer);
		
		if (gDebugScrollBar)
			DisposeControl(gDebugScrollBar);
		
		if (gDebugWindow)
		{
			GetWindowPortBounds(gDebugWindow, &bounds);

			data = NewHandle(sizeof(Point));
			assert (data!=NULL);
			SetPt(&where,bounds.left,bounds.top);
			
			GetPort(&savedPort);
			SetPortWindowPort(gDebugWindow);
			LocalToGlobal(&where);
			SetPort(savedPort);
			BlockMove(&where,*data,sizeof(Point));
			SetPreference(kDebugWindowLocPref,kDebugWindowLocPrefID,data);
			DisposeHandle(data);
			
			visible = IsWindowVisible(gDebugWindow);//  ((WindowPeek)gDebugWindow)->visible;
			data = NewHandle(sizeof(short));
			assert (data!=NULL);
			BlockMove(&visible,*data,sizeof(short));
			SetPreference(kDebugWindowVisiblePref,kDebugWindowVisiblePrefID,data);
			DisposeHandle(data);
			
			HideWindow(gDebugWindow);
			DisposeWindowInfo(gDebugWindow);
			DisposeWindow(gDebugWindow);
		}
		
		if (gDebugWindowClickUPP)
			DisposeRoutineDescriptor(gDebugWindowClickUPP);
	#endif	DEBUG
}





void ShowDebugWindow(void)
{
	#if	DEBUG
		MenuHandle	theMenu;
		
		
		theMenu = GetMenu(1000);
		SetMenuItemText(theMenu,2,"\pHide Debug Window");
		ShowWindow(gDebugWindow);
	#endif	DEBUG
}





void HideDebugWindow(void)
{
	#if	DEBUG
		MenuHandle	theMenu;
		
		
		theMenu = GetMenu(1000);
		SetMenuItemText(theMenu,2,"\pShow Debug Window");
		HideWindow(gDebugWindow);
	#endif	DEBUG
}





void DebugWindowClickProc(WindowPtr window,long refcon,Point where)
{
	#if	DEBUG
		ControlHandle	control;
		short				partCode;
		
		
		partCode = FindControl(where,window,&control);
		if (partCode)
			switch(partCode)
			{
				case kControlUpButtonPart:
				case kControlDownButtonPart:
				case kControlPageUpPart:
				case kControlPageDownPart:
					partCode = TrackControl(control,where,gDebugWindowClickUPP);
					break;
				case kControlIndicatorPart:
					partCode = TrackControl(control,where,NULL);
					DrawDebugWindowText();
					break;
			}
	#endif	DEBUG
}





pascal void DebugScrollBarActionProc(ControlHandle control,short part)
{
	#if	DEBUG
		short	value,originalValue;
		
		
		value = GetControl32BitValue(control);
		originalValue = value;
		
		switch(part)
		{
			case kControlUpButtonPart:
				value = (value == 0) ? value : value-1;
				break;
			case kControlDownButtonPart:
				value = (value == (gTotalLines-20)) ? value : value+1;
				break;
			case kControlPageUpPart:
				value -= 19;
				if (value < 0)
					value = 0;
				break;
			case kControlPageDownPart:
				value += 19;
				if (value > (gTotalLines-20))
					value = (gTotalLines-20);
				break;
		}
		
		if (value != originalValue)
		{
			SetControl32BitValue(control,value);
			DrawDebugWindowText();
		}
	#endif	DEBUG
}





void DebugWindowCloseProc(WindowPtr window,long refcon)
{
	#if	DEBUG
		HideDebugWindow();
	#endif	DEBUG
}





void DebugWindowUpdateProc(WindowPtr window,long refcon)
{
	#if	DEBUG
		RgnHandle	savedClip;
		Rect			newClip;
		Str255		wah;
		long			index,offset;
		
		
		DrawControls(window);
		
		savedClip = NewRgn();
		GetClip(savedClip);
		
		GetWindowPortBounds(gDebugWindow, &newClip);

		//newClip = window->portRect;
		newClip.right -= 15;
		ClipRect(&newClip);
		
		DrawDebugWindowText();
		
		SetClip(savedClip);
		DisposeRgn(savedClip);
	#endif	DEBUG
}





void DrawDebugWindowText(void)
{
	#if	DEBUG
		long	offset,negoffset;
		short	index,top,value;
		
		
		//TextFont(monaco);
		TextSize(9);
		TextMode(srcCopy);
		
		value = GetControl32BitValue(gDebugScrollBar);
		negoffset = (value > 0) ? value*11 : 0;
		
		index = value;
		top = ((index+20) > gTotalLines) ? gTotalLines : index+20;
		
		for (;index<top;index++)
		{
			offset = index*62;
			MoveTo(2,index*11+9-negoffset);
			DrawString((StringPtr)gTextBuffer+offset);
		}
	#endif	DEBUG
}





void debug_window_printf(char *format,...)
{
	#if	DEBUG
		char		string[512];
		va_list	argptr;
		long		len,offset;
		GrafPtr	savedPort;
		
		
		va_start(argptr,format);
		len = (long)vsprintf(string,format,argptr);
		va_end (argptr);
		
		if (len > 60)
			len = 60;
		
		gTotalLines++;
		if (gTotalLines == kScrollBackBufferSize)
		{
			BlockMoveData(gTextBuffer+62,gTextBuffer,62*(kScrollBackBufferSize-1));
			gTotalLines = kScrollBackBufferSize-1;
		}
		
		offset = 62*(gTotalLines-1);
		strcpy(gTextBuffer+1+offset,string);
		memset(gTextBuffer+offset+1+len,' ',61-len);
		*(gTextBuffer+offset) = 60;
		
		if (gTotalLines > 20)
		{
			SetControl32BitMaximum(gDebugScrollBar,gTotalLines-20);
			SetControl32BitValue(gDebugScrollBar,gTotalLines-20);
		}
		
		
		GetPort(&savedPort);
		SetPortWindowPort(gDebugWindow);
		
		
		DrawDebugWindowText();
		
		SetPort(savedPort);
	#endif	DEBUG
}