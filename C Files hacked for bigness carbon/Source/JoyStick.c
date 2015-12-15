/*-------------------------------------------------------------------------------*\
|
|	File:	JoyStick.c
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
#include	"DebugWindow.h"
#include "JoyStick.h"



/*	local globals	*/
Boolean	gEnableJoyA, gEnableJoyB;





void InitJoystick(void)
{
		InsertPlainMenu(132,0);
		AddItemProc(ToggleJoyProc,132,1);
		AddItemProc(ToggleJoyProc,132,2);
		DrawMenuBar();
		gEnableJoyA=FALSE;
		gEnableJoyB=FALSE;
}





unsigned long ReadJoyStick1(void)
{
	unsigned char	keys[16];
	unsigned long	result;
	
	
	GetKeys((long*)&keys[0]);
	
	result = 0;

	if (keys[11]&0x08)	//up
		result |= 0x01;

	if (keys[10]&0x10)	//down
		result |= 0x02;

	if (keys[10]&0x40)	//left
		result |= 0x04;

	if (keys[11]&0x02)
		result |= 0x05;	//up and left

	if (keys[10]&0x08)
		result |= 0x06;	//down and left

	if (keys[11]&0x01)	//right
		result |= 0x08;

	if (keys[11]&0x10)
		result |= 0x09;	//up and right
		
	if (keys[10]&0x20)
		result |= 0x0A;	//down and right


	if (keys[10]&0x04)	//fire button
		result |= 0x10;
	
	return ~result;
}






void ToggleJoyProc	(short menu,short item)
{
		MenuHandle	theMenu;
		Str255		itemString;
		
	switch (item)
		{
		case	1:
		theMenu = GetMenu(menu);
		GetMenuItemText(theMenu,item,itemString);
		if (itemString[1] == 'E')
			{
			gEnableJoyA=TRUE;
			SetMenuItemText(theMenu,item,"\pDisable Joy 2");
			}
		else
			{
			gEnableJoyA=FALSE;
			SetMenuItemText(theMenu,item,"\pEnable Joy 2");
			}
		break;
		case	2:
		theMenu = GetMenu(menu);
		GetMenuItemText(theMenu,item,itemString);
		if (itemString[1] == 'E')
			{
			gEnableJoyB=TRUE;
			SetMenuItemText(theMenu,item,"\pDisable Joy 1");
			}
		else
			{
			gEnableJoyB=FALSE;
			SetMenuItemText(theMenu,item,"\pEnable Joy 1");
			}
		break;
		}
}