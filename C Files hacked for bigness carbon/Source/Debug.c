/*-------------------------------------------------------------------------------*\
|
|	File:	Debug.c
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
#include "Menu.h"
#include "DebugWindow.h"
#include "Debug.h"

//temp
#include "SID.h"
#include "ColorDrawing.h"

Boolean		gVerbose = FALSE;

#undef DEBUG


/*	local globals */
#if		DEBUG
	WindowPtr	gSpeedWindow = NULL;
	Boolean		gVerbose = FALSE;
#endif	DEBUG





void InitDebug(void)
{
	#ifdef	DEBUG
		InsertPlainMenu(1000,0);
		AddItemProc(ShowHideDebugWindowProc,1000,1);
		AddItemProc(ShowHideDebugWindowProc,1000,2);
		AddItemProc(ShowHideDebugWindowProc,1000,3);
		AddItemProc(ShowHideDebugWindowProc,1000,4);
		AddItemProc(ShowHideDebugWindowProc,1000,5);
		DrawMenuBar();
		
		InitDebugWindow();
		
		//gSpeedWindow = GetNewWindow(1001,NULL,(WindowPtr)(-1L));
		//OSErr err=CreateWindowFromResource(1001,&gSpeedWindow);
		
		Rect bounds;
		bounds.top=100;
		bounds.left=100;
		bounds.right=500;
		bounds.bottom=500;
		OSStatus err=CreateNewWindow ( kDocumentWindowClass ,kWindowStandardDocumentAttributes , &bounds, &gSpeedWindow);
		
		ShowWindow(gSpeedWindow);
		
	//	assert(gSpeedWindow!=NULL);
		if (gSpeedWindow==NULL)
			DebugStr("\p Could not allocate speed window");
	#endif	DEBUG
}





void CleanUpDebug(void)
{
	#ifdef	DEBUG
		CleanUpDebugWindow();
		if (gSpeedWindow)
			DisposeWindow(gSpeedWindow);
	#endif	DEBUG
}





void ShowHideDebugWindowProc(short menu,short item)
{
	#ifdef	DEBUG
		MenuHandle	theMenu;
		Str255		itemString;
			
		switch (item)
		{
			case	1:
				theMenu = GetMenu(menu);
				GetMenuItemText(theMenu,item,itemString);
				if (itemString[1] != 'S')
				{
					HideWindow(gSpeedWindow);
					SetMenuItemText(theMenu,item,"\pShow Speed Window");
				}
				else
				{
					ShowWindow(gSpeedWindow);
					SetMenuItemText(theMenu,item,"\pHide Speed Window");
				}
				break;
			case	2:
				theMenu = GetMenu(menu);
				GetMenuItemText(theMenu,item,itemString);
				if (itemString[1] != 'S')
					HideDebugWindow();
				else
					ShowDebugWindow();
				break;

				case 3:
				theMenu = GetMenu(menu);
				GetMenuItemText(theMenu,item,itemString);
				if (itemString[1] != 'V')
				{
					gVerbose=TRUE;
					SetMenuItemText(theMenu,item,"\pVerbose");
				}
				else
				{
					gVerbose=FALSE;
					SetMenuItemText(theMenu,item,"\pQuiet");
				}
				break;

			case	4:
				theMenu = GetMenu(menu);
				GetMenuItemText(theMenu,item,itemString);
				ToggleSound();
				if (itemString[8] != 'F')
				{

					SetMenuItemText(theMenu,item,"\pSound OFF");
				}
				else
				{

					SetMenuItemText(theMenu,item,"\pSound ON");
				}
				break;
				case 5:
				theMenu = GetMenu(menu);
				GetMenuItemText(theMenu,item,itemString);
				ToggleCommodoreWindowSize();
//				if (itemString[1] != '')
//				{
//					SetMenuItemText(theMenu,item,"\pVerbose");
//				}
//				else
//				{
//					SetMenuItemText(theMenu,item,"\pQuiet");
//				}
				break;

		}
	#endif	DEBUG
}





#define	COUNT_OPS	0

#if	COUNT_OPS
extern unsigned long	one;
extern unsigned long	two;
extern unsigned long	thr;
#endif




void UpdateSpeedWindow(float speed)
{
	GrafPtr	oldPort;
	char		s[20];
	
	return; // AJS
#if 0
	sprintf(s,"%f MHz  ",speed);
	CtoPstr(s);
	
	GetPort(&oldPort);
	SetPort(gSpeedWindow);
	
	TextMode(srcCopy);
	TextFont(0);
	
	MoveTo(12,14);
	DrawString((StringPtr)s);
	
	SetPort(oldPort);
	
	#if	COUNT_OPS
	debug_window_printf("");
	debug_window_printf("one: %ld",one);one=0;
	debug_window_printf("two: %ld",two);two=0;
	debug_window_printf("thr: %ld",thr);thr=0;
	#endif
#endif
}