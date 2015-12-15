/*-------------------------------------------------------------------------------*\||	File:	Event.c||	Description:||||||	Copyright �	1994, Alan Steremberg and Ed Wynne|\*-------------------------------------------------------------------------------*/#include "Menu.h"#include "Emulator.h"#include "CombugWindow.h"#include "Combug.h"#include "Event.h"extern	CWindowPtr			theMonitorWindow;extern	WindowPtr			gCombugWindow;extern	CombugState			gCombugState;/*	Local globals	*/short		gQuitFlag;Boolean	gCommdoreHasFocus=false;void InitEvents(void){	gQuitFlag = 0;}void	fudge_drawing_vals(void);short doEventLoop(long sleepTime){	DialogPtr	dialog;	EventRecord	event;	short			item;	short			gotEvent;			gotEvent = WaitNextEvent(everyEvent,&event,sleepTime,NULL);	if (gotEvent)	{		if (IsDialogEvent(&event) && DialogSelect(&event,&dialog,&item))			DoWindowItem((WindowPtr)dialog,item);		else			switch(event.what)			{				case mouseDown:					doMouseDown(event.where);					break;				case mouseUp:					break;				case autoKey:					if (event.modifiers&cmdKey)						break;				case keyDown:					doKeyDown(event.message,event.modifiers);					break;				case keyUp:					break;				case updateEvt:					DoWindowUpdate((WindowPtr)event.message);					break;				case activateEvt:					if (theMonitorWindow)					{						if (theMonitorWindow==(CWindowPtr)FrontWindow())							gCommdoreHasFocus=true;							else									gCommdoreHasFocus=false;						}					break;				case osEvt:						switch ((event.message >> 24) & 0x00FF)						{						case suspendResumeMessage:							if (event.message & 0x00000001) //resume							{													if (theMonitorWindow)								{								if (theMonitorWindow==(CWindowPtr)FrontWindow())									gCommdoreHasFocus=true;									else											gCommdoreHasFocus=false;									}							}						else							if (theMonitorWindow)							{								if (theMonitorWindow==(CWindowPtr)FrontWindow())									gCommdoreHasFocus=false;								}						}				break;			}	}	else if (IsDialogEvent(&event))		DialogSelect(&event,&dialog,&item);	#if	1	EmulatorRun();#else	fudge_drawing_vals();#endif	if (gCombugState.halted && gCombugWindow==FrontWindow() && (!gCombugState.halted))		CombugDrawInsertionPoint(FALSE);	else		CombugDrawInsertionPoint(TRUE);	return gQuitFlag;}void doMouseDown(Point where){	WindowPtr	window;	Rect			sizeRect;	long			menuSelection;	long			newSize;	short			partCode;			partCode = FindWindow(where,&window);	if (window && (window != FrontWindow()))		SelectWindow(window);	else		switch(partCode)		{			case inDesk:				break;			case inMenuBar:				menuSelection = MenuSelect(where);				if (menuSelection & 0xFFFF0000)				{					doMenuSelect(menuSelection>>16,menuSelection);					HiliteMenu(0);				}				break;			case inSysWindow:				break;			case inContent:				GlobalToLocal(&where);				DoWindowClick(window,where);				break;			case inDrag:				DoWindowDrag(window,where);				break;			case inGrow:				DoWindowGrow(window,where);				break;			case inGoAway:				if (TrackGoAway(window,where))					DoWindowClose(window);				break;			case inZoomIn:				if (TrackBox(window,where,inZoomIn))					DoWindowZoomIn(window);				break;			case inZoomOut:				if (TrackBox(window,where,inZoomOut))					DoWindowZoomIn(window);				break;		}}void doKeyDown(long message,short modifiers){	WindowInfo	*info;	WindowPtr	window;	long			menuSelection=0;	char			theChar;			theChar = charCodeMask & message;	if (modifiers & cmdKey)	{		menuSelection = MenuKey(theChar);		if (menuSelection & 0xFFFF0000)		{			doMenuSelect(menuSelection>>16,menuSelection);			HiliteMenu(0);		}	}		if ((!(menuSelection&0xFFFF)) && (window = FrontWindow()) != NULL)		if (!(modifiers & cmdKey) || (theChar == '.'))			DoWindowKey(theChar,(keyCodeMask&message)>>8,modifiers);}