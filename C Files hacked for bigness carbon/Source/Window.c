/*-------------------------------------------------------------------------------*\
|
|	File:	Window.c
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


#include "Window.h"





void DoWindowKey(char c,unsigned char k,short modifiers)
{
	WindowInfo	*info;
	WindowPtr	window;
	GrafPtr		savedPort;
	
		if (window==NULL) return;

	if ((window = FrontWindow()) != NULL)
	{
		GetPort(&savedPort);
		SetPortWindowPort(window);
		
		info = (WindowInfo*)GetWRefCon(window);
		if (info && info->WindowKeyProc)
			info->WindowKeyProc(window,info->refcon,c,k,modifiers);
		
		SetPort(savedPort);
	}
}





void DoWindowClick(WindowPtr window,Point where)
{
	WindowInfo	*info;
	GrafPtr		savedPort;
	
		if (window==NULL) return;

	GetPort(&savedPort);
	SetPortWindowPort(window);
	
	GlobalToLocal(&where);
	info = (WindowInfo*)GetWRefCon(window);
	if (info && info->WindowClickProc)
		info->WindowClickProc(window,info->refcon,where);
	
	SetPort(savedPort);
}





void DoWindowItem(WindowPtr window,short item)
{
	WindowInfo	*info;
	GrafPtr		savedPort;
	
		if (window==NULL) return;

	GetPort(&savedPort);
	SetPortWindowPort(window);
	
	info = (WindowInfo*)GetWRefCon(window);
	if (info && info->WindowItemProc)
		info->WindowItemProc(window,info->refcon,item);
	
	SetPort(savedPort);
}





void DoWindowUpdate(WindowPtr window)
{
	WindowInfo	*info;
	GrafPtr		savedPort;
	
		if (window==NULL) return;

	GetPort(&savedPort);
	SetPortWindowPort(window);
	BeginUpdate(window);
	
	info = (WindowInfo*)GetWRefCon(window);
	if (info && info->WindowUpdateProc)
		info->WindowUpdateProc(window,info->refcon);
	
	EndUpdate(window);
	SetPort(savedPort);
}





void DoWindowClose(WindowPtr window)
{
	WindowInfo	*info;
	GrafPtr		savedPort;
		if (window==NULL) return;

	
	GetPort(&savedPort);
	SetPortWindowPort(window);
	
	info = (WindowInfo*)GetWRefCon(window);
	if (info && info->WindowCloseProc)
		info->WindowCloseProc(window,info->refcon);
	
	SetPort(savedPort);
}





void DoWindowDrag(WindowPtr window,Point start)
{
	WindowInfo	*info;
	GrafPtr		savedPort;
		if (window==NULL) return;

	
	GetPort(&savedPort);
	SetPortWindowPort(window);
	
	info = (WindowInfo*)GetWRefCon(window);
	if (info && info->WindowDragProc)
		info->WindowDragProc(window,info->refcon,start);
	else
	{
	   Rect bounds;
		BitMap bmap;
		GetQDGlobalsScreenBits(&bmap);
		bounds = bmap.bounds;
		DragWindow(window, start, &bounds);
		//DragWindow(window,start,&qd.screenBits.bounds);

	}
	
	SetPort(savedPort);
}





void DoWindowGrow(WindowPtr window,Point start)
{
	WindowInfo	*info;
	GrafPtr		savedPort;
	Rect			sizeRect;
	long			newSize;
		if (window==NULL) return;

	
	GetPort(&savedPort);
	SetPortWindowPort(window);
	
	info = (WindowInfo*)GetWRefCon(window);
	if (info && info->WindowGrowProc)
		info->WindowGrowProc(window,info->refcon,start);
	else
	{
	
		Rect bounds;
		BitMap bmap;
		GetQDGlobalsScreenBits(&bmap);
		bounds = bmap.bounds;
	
	
	
		SetRect(&sizeRect,40,40,bounds.right-bounds.left-10,
			bounds.bottom-bounds.top-GetMBarHeight()-30);
		newSize = GrowWindow(window,start,&sizeRect);
		SizeWindow(window,newSize&0xFFFF,(newSize>>16)&0xFFFF,true);
	}
	
	SetPort(savedPort);
}





void DoWindowZoomIn(WindowPtr window)
{
	WindowInfo	*info;
	GrafPtr		savedPort;
	
		if (window==NULL) return;

	GetPort(&savedPort);
	SetPortWindowPort(window);
	
	info = (WindowInfo*)GetWRefCon(window);
	if (info && info->WindowZoomInProc)
		info->WindowZoomInProc(window,info->refcon);
	
	SetPort(savedPort);
}





void DoWindowZoomOut(WindowPtr window)
{
	WindowInfo	*info;
	GrafPtr		savedPort;
		if (window==NULL) return;

	
	GetPort(&savedPort);
	SetPortWindowPort(window);
	
	info = (WindowInfo*)GetWRefCon(window);
	if (info && info->WindowZoomOutProc)
		info->WindowZoomOutProc(window,info->refcon);
	
	SetPort(savedPort);
}





void NewWindowInfo(WindowPtr window)
{
	WindowInfo	*info;
	
		if (window==NULL) return;

	info = (WindowInfo*)NewPtrClear(sizeof(WindowInfo));
	SetWRefCon(window,(long)info);
}





void DisposeWindowInfo(WindowPtr window)
{
	WindowInfo	*info;
		if (window==NULL) return;

	
	info = (WindowInfo*)GetWRefCon(window);
	if (info)
		DisposePtr((Ptr)info);
	SetWRefCon(window,0L);
}





long GetWindowRefCon(WindowPtr window)
{
	WindowInfo	*info;
	
		if (window==NULL) return NULL;

	info = (WindowInfo*)GetWRefCon(window);
	if (info)
		return info->refcon;
	
	return NULL;
}





void SetWindowRefCon(WindowPtr window,long refcon)
{
	WindowInfo	*info;
		if (window==NULL) return;

	
	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->refcon = refcon;
}





void SetWindowKeyProc(WindowPtr window,WindowKeyProc key)
{
	WindowInfo	*info;
	
		if (window==NULL) return;

	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->WindowKeyProc = key;
}





void SetWindowClickProc(WindowPtr window,WindowClickProc click)
{
	WindowInfo	*info;
	
		if (window==NULL) return;

	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->WindowClickProc = click;
}





void SetWindowItemProc(WindowPtr window,WindowItemProc item)
{
	WindowInfo	*info;
	
	if (window==NULL) return;
	
	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->WindowItemProc = item;
}





void SetWindowUpdateProc(WindowPtr window,WindowUpdateProc update)
{
	WindowInfo	*info;
	
	if (window==NULL) return;
	
	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->WindowUpdateProc = update;
}





void SetWindowCloseProc(WindowPtr window,WindowCloseProc close)
{
	WindowInfo	*info;
	
	if (window==NULL) return;
	
	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->WindowCloseProc = close;
}





void SetWindowDragProc(WindowPtr window,WindowDragProc drag)
{
	WindowInfo	*info;
	
	if (window==NULL) return;
	
	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->WindowDragProc = drag;
}





void SetWindowGrowProc(WindowPtr window,WindowGrowProc grow)
{
	WindowInfo	*info;
	if (window==NULL) return;
	
	
	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->WindowGrowProc = grow;
}





void SetWindowZoomInProc(WindowPtr window,WindowZoomInProc zoomIn)
{
	WindowInfo	*info;
	
	if (window==NULL) return;
	
	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->WindowZoomInProc = zoomIn;
}





void SetWindowZoomOutProc(WindowPtr window,WindowZoomOutProc zoomOut)
{
	WindowInfo	*info;
	if (window==NULL) return;
	
	
	info = (WindowInfo*)GetWRefCon(window);
	if (!info)
		NewWindowInfo(window);
	
	info->WindowZoomOutProc = zoomOut;
}