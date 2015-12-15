/*-------------------------------------------------------------------------------*\
|
|	File:	Menu.c
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

#include "Menu.h"





/*	Extern globals	*/
extern short			gQuitFlag;

/*	Local globals	*/
MenuItemProcHandle	gItemProcs;





void InitAppMenus(void)
{
	gItemProcs = NULL;
	
	InsertAppleMenu(128,0);
	InsertPlainMenu(129,0);
	InsertPlainMenu(130,0);
	
	AddItemProc(AboutItemProc,128,1);
	AddItemProc(AppleItemsProc,128,kAllItems);
	AddItemProc(FileQuitItemProc,129,3);
	
	DrawMenuBar();
}





void InsertAppleMenu(short menuID,short beforeID)
{
//	MenuHandle	menu;
	
	
//	menu = GetMenu(menuID);
//	AddResMenu(menu,'DRVR');
//	InsertMenu(menu,beforeID);
}





void InsertPlainMenu(short menuID,short beforeID)
{
	MenuHandle	menu;
	
	
	menu = GetMenu(menuID);
	InsertMenu(menu,beforeID);
}





void AddItemProc(ItemProc proc,short menu,short item)
{
	unsigned long	len;
	
	
	if (!gItemProcs)
	{
		gItemProcs = (MenuItemProcHandle)NewHandle(0L);
		assert (gItemProcs!=NULL);
	}
	SetHandleSize((Handle)gItemProcs,GetHandleSize((Handle)gItemProcs)+sizeof(MenuItemProc));
	len = GetHandleSize((Handle)gItemProcs)/sizeof(MenuItemProc)-1;
	HLock((Handle)gItemProcs);
	
	(*gItemProcs)[len].menu = menu;
	(*gItemProcs)[len].item = item;
	(*gItemProcs)[len].proc = proc;
	
	HUnlock((Handle)gItemProcs);
}





void doMenuSelect(short menu,short item)
{
	unsigned long	len;
	
	
	if (!gItemProcs)
		return;
	
	HLock((Handle)gItemProcs);
	len = GetHandleSize((Handle)gItemProcs)/sizeof(MenuItemProc);
	while(len--)
		if (((*gItemProcs)[len].menu == menu) && ((*gItemProcs)[len].item == item))
		{
			(*gItemProcs)[len].proc(menu,item);
			HUnlock((Handle)gItemProcs);
			return;
		}
	
	len = GetHandleSize((Handle)gItemProcs)/sizeof(MenuItemProc);
	while(len--)
		if (((*gItemProcs)[len].menu == menu) && ((*gItemProcs)[len].item == kAllItems))
		{
			(*gItemProcs)[len].proc(menu,item);
			HUnlock((Handle)gItemProcs);
			return;
		}
	
	HUnlock((Handle)gItemProcs);
}





void AboutItemProc(short menu,short item)
{
DialogPtr	theDlog;
short			itemHit;

	theDlog=GetNewDialog(129,NULL,(WindowPtr)-1);
	ModalDialog(NULL,&itemHit);
	DisposeDialog(theDlog);

}





void AppleItemsProc(short menu,short item)
{
	Str255		name;
	WindowPtr	window;
	MenuHandle	theMenu;
	
#if 0	
	GetPort(&window);
	theMenu = GetMHandle(menu);
	GetMenuItemText(theMenu,item,name);
	OpenDeskAcc(name);
	SetPort(window);
#endif
}





void FileQuitItemProc(short menu,short item)
{
	gQuitFlag = 1;
}