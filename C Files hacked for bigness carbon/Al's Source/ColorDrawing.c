/*-------------------------------------------------------------------------------*\
|
|	File:	ColorDrawing.c
|
|	Description:
|
|	This file contains routines to open the commodore window, initialize our
|	offscreen GWorld, as well as set up the parameters for Color Blasting, 
|	screen updates, and making sure that the commodore screen is not dragged off
|	the main monitor.
|
|	Copyright �	1994, Alan Steremberg and Ed Wynne
|
\*-------------------------------------------------------------------------------*/



//
//	Problems - be sure to check to see that window is on one monitor
//			 - call swapmmu mode if on a 68k machine and not in 32bit mode
//			 - fix problems with double sizing

//
//	QD speed tricks - be sure ctseeds are the same from src to dest when using copybits
//
#include "DebugWindow.h"
#include <SDL2/SDL.h>

#include "Drawing.h"
#include "Window.h"

#include "ColorDrawing.h"


//
//	From DrawingGlobals.h
//

extern	UInt32	fourBitTable[16];
//
//	These constants need to be moved to the header
//

#define	kMonitorWindow	128
#define	kCommodoreClut	128


//
//	Current Drawing Paramters, used to toggle between offscreen and onscreen
//

unsigned char 			*theActiveBits;
UInt32					theActiveRowBytes;
unsigned char 			*originaltheActiveBits;

unsigned	char		*gTheActiveBitsArray[400];

unsigned	long		gPixelDepth;

Boolean					gDoubleSize=FALSE;

//
//	Global Variables needed to Draw Directly to the Screen
//

static	unsigned char 	*theOnScreenBits; 
static	UInt32			theOnScreenRowBytes;


//
//	Global Variables needed to Draw into offscreen PixMap
//
#if	DRAWOFFSCREEN

GWorldPtr			gOffScreenWorld=NULL;
PixMapHandle		gOffScreenPixMap=NULL;

unsigned char 		*theOffScreenBits; 
UInt32				theOffScreenRowBytes;


#endif
Rect				theOffScreenRect;

SDL_Window			*theMonitorWindow=NULL;
SDL_Renderer        *sdlRenderer= NULL;
SDL_Surface         *theSurface=NULL;



//
//	This should be called before drawing directly to the screen, as well as when
//	the user moves the window.
//
void	UpdateBlastingParameters ()
{
    theActiveBits = theOnScreenBits;
    theActiveRowBytes = theOnScreenRowBytes;

    gPixelDepth=8;
    
    for (int i=0;i<400;i++)
        gTheActiveBitsArray[i]=theActiveBits+(theActiveRowBytes*i);
    
    //
    //	Be sure we are in the right video mode..
    //
    VICChangeVideoModes();
  
}
#if 0

void	UpdateBlastingParameters ()
{
	Point				topleft;
	GrafPtr				oldPort;
	GDHandle			theScreen=NULL;
	short				i;
	OSErr				iErr;

	//
	//	Be sure the window is valid
	//
	if (theMonitorWindow==NULL)
		debug_window_printf("UpdateBlastingParameters(): theMonitorWindow is NULL");
	else
		{
		   Rect thePortRect;

		//
		//	Grab the handle to the main screen device
		//	%%% Should change this to find screen window resides on...
		//
		theScreen=GetMainDevice();
		if ((**theScreen).gdType!=0)
			debug_window_printf("UpdateBlastingParameters(): gdType is not CLUT");
		//
		//	Do the calculations to find the top left of the window
		//
	
		theOnScreenBits=(unsigned char *)(**(**theScreen).gdPMap).baseAddr; 
		theOnScreenRowBytes=(UInt32)( (**(**theScreen).gdPMap).rowBytes & 0x0007FFF);

		//gPixelDepth=(**(**theScreen).gdPMap).pixelSize;
		gPixelDepth=8; // AJS
	
		
	   GetWindowPortBounds(theMonitorWindow, &thePortRect);

		//topleft.h=theMonitorWindow->portRect.left;
		//topleft.v=theMonitorWindow->portRect.top;
		
		topleft.h=thePortRect.left;
		topleft.v=thePortRect.top;
	
		GetPort(&oldPort);
		SetPortWindowPort((WindowPtr)theMonitorWindow);
		LocalToGlobal(&topleft);
		switch (gPixelDepth)
			{
			case	8:
				if (gDoubleSize)
				{
					short swidth=((**theScreen).gdRect.right-(**theScreen).gdRect.left);
					//short wwidth=(theMonitorWindow->portRect.right-theMonitorWindow->portRect.left);
					short wwidth=(thePortRect.right-thePortRect.left);
					

					short sheight=((**theScreen).gdRect.bottom-(**theScreen).gdRect.top);
					//short wheight=(theMonitorWindow->portRect.bottom-theMonitorWindow->portRect.top);
					short wheight=(thePortRect.bottom-thePortRect.top);

					topleft.h=(swidth/2)-(wwidth/2);
					topleft.v=(sheight/2)-(wheight/2)+15;
					topleft.h-=topleft.h%2;
					
				}
				else
					topleft.h -= topleft.h%4;
			break;
			case	4:
				topleft.h -= topleft.h%8;
			break;
			case	1:
				topleft.h -= topleft.h%32;
			break;
			}


		MoveWindow((WindowPtr)theMonitorWindow,topleft.h,topleft.v,true);
		SetPort(oldPort);
	
		//
		//	Move to the top left of the window
		//
		switch (gPixelDepth)
			{
			case	8:
				theOnScreenBits+=(topleft.h)+(topleft.v*theOnScreenRowBytes);
			break;
			case	4:
				theOnScreenBits+=(topleft.h/2)+((topleft.v)*theOnScreenRowBytes);
			break;
			case	1:
				theOnScreenBits+=(topleft.h/8)+((topleft.v)*theOnScreenRowBytes);
			break;
			}
 		}

	//
	//	When we update our blasting parameters, set the active drawing mode to blasting, we probably want 
	//	to check something before doing this for real
	//

#if	DRAWOFFSCREEN
	originaltheActiveBits=theActiveBits=theOffScreenBits;
	theActiveRowBytes=theOffScreenRowBytes;
#else
	originaltheActiveBits=theActiveBits=theOnScreenBits;
	theActiveRowBytes=theOnScreenRowBytes;
#endif
	for (i=0;i<400;i++)
		gTheActiveBitsArray[i]=theActiveBits+(theActiveRowBytes*i);

	//
	//	Be sure we are in the right video mode..
	//
	VICChangeVideoModes();

}
#endif

#if 0
void DragCommodoreWindow(WindowPtr	window,long refcon,Point start)
{
	GDHandle			theScreen=NULL;
	Rect				dragablearea;
	Point				localstart;
	
	Rect				theWindowRect;
		   
	GetWindowPortBounds(window, &theWindowRect);

	theScreen=GetMainDevice();

	localstart=start;
	GlobalToLocal(&localstart);

#if	DRAWOFFSCREEN
	{
	BitMap bmap;
	GetQDGlobalsScreenBits(&bmap);
	dragablearea=bmap.bounds;
	}
#else
	dragablearea=((**theScreen).gdRect);	

	//
	//	Screw with the dragable rectangle so it never leaves the main window
	//

	dragablearea.left=localstart.h-theWindowRect.left;	
	dragablearea.right-=theWindowRect.right-localstart.h;
	dragablearea.bottom-=theWindowRect.bottom-localstart.v;
#endif	
	DragWindow(window,start,&dragablearea);

	GetWindowPortBounds(window, &theWindowRect);

	localstart.h = theWindowRect.left;
	localstart.v = theWindowRect.top;
	LocalToGlobal(&localstart);
	localstart.h -= localstart.h%4;
	MoveWindow(window,localstart.h,localstart.v,true);

	UpdateBlastingParameters();
	BlastColorTestPattern();
}

#endif

//
//	Copy the offscren world to the window - used during updates, safe mode, etc.
//
void CopyOffScreenToWindow(void)
{
   // fprintf(stderr,"CopyOffScreenToWindow %p\n",theActiveBits);
if (theActiveBits!=NULL)
{
    //SDL_FillRect(theSurface, NULL, SDL_MapRGB(theSurface->format, 255, 0, 0));
    SDL_LockSurface(theSurface);
    int pixel=0; for (int i=0;i<16;i++) for (int j=0;j<16;j++) theActiveBits[pixel++]=i;
    memcpy(theSurface->pixels,theActiveBits,theSurface->w*theSurface->h);

    SDL_Texture *sdlTexture = SDL_CreateTextureFromSurface(sdlRenderer,theSurface);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
    SDL_DestroyTexture(sdlTexture);
    SDL_UnlockSurface(theSurface);
}
}

#if 0

void	CopyOffScreenToWindow(void)
{
GrafPtr	oldPort;

#if	0
if (theMonitorWindow==NULL)
	debug_window_printf("\p window is null, problem here..");
if (gOffScreenPixMap==NULL)
	debug_window_printf("\p pixmap is null, problem here..");
return;
#endif

#if	DRAWOFFSCREEN
	GetPort(&oldPort);
	SetPortWindowPort((WindowPtr)theMonitorWindow);

//if (!gDoubleSize)
//	CopyBits ((BitMapPtr)*(theOffscreenGraf->portPixMap), &((GrafPtr)theMonitorWindow)->portBits, &theMonitorWindow->portRect, &theMonitorWindow->portRect, srcCopy, nil);
//else
//	CopyBits ((BitMapPtr)*(theOffscreenGraf->portPixMap), &((GrafPtr)theMonitorWindow)->portBits, &theOffScreenRect, &theMonitorWindow->portRect, srcCopy, nil);

	Rect thePortRect;
	GetWindowPortBounds(theMonitorWindow, &thePortRect);

#if 1
if (!gDoubleSize)
	{

	CopyBits ((BitMapPtr)*(gOffScreenPixMap), GetPortBitMapForCopyBits(GetWindowPort(theMonitorWindow)), &thePortRect, &thePortRect, srcCopy, nil);
	}
else
	CopyBits ((BitMapPtr)*(gOffScreenPixMap), GetPortBitMapForCopyBits(GetWindowPort(theMonitorWindow)), &theOffScreenRect, &thePortRect, srcCopy, nil);
#endif

	SetPort(oldPort);

#endif
}
#endif

//
//
//

void UpdateCommodoreWindow(WindowPtr window,long refcon)
{
	CopyOffScreenToWindow();

}

void ToggleCommodoreWindowSize ()
{
#if 0
//#if	DRAWOFFSCREEN
if (gDoubleSize)
	{
	gDoubleSize=false;
	SizeWindow((WindowPtr)theMonitorWindow,theOffScreenRect.right-theOffScreenRect.left,theOffScreenRect.bottom-theOffScreenRect.top,TRUE);
	
	}
else
	{
	SizeWindow((WindowPtr)theMonitorWindow,(theOffScreenRect.right-theOffScreenRect.left)*2,(theOffScreenRect.bottom-theOffScreenRect.top)*2,TRUE);
	gDoubleSize=true;
	}
//#endif
#endif
UpdateBlastingParameters();
}
//
//	This function is will setup the emulator window, and setup the color palette and color
//	table to work with direct screen drawing.
//



SDL_Color palette2[16] = {
    {0x00 ,0x00 ,0x00},
    {0xFF, 0xFF, 0xFF},
    {0x68, 0x37, 0x2B},
    {0x70, 0xA4, 0xB2},
    {0x6F, 0x3D, 0x86},
    {0x58, 0x8D, 0x43},
    {0x35, 0x28, 0x79},
    {0xB8, 0xC7, 0x6F},
    {0x6F, 0x4F, 0x25},
    {0x43, 0x39, 0x00},
    {0x9A, 0x67, 0x59},
    {0x44, 0x44, 0x44},
    {0x6C, 0x6C, 0x6C},
    {0x9A, 0xD2, 0x84},
    {0x6C, 0x5E, 0xB5},
    {0x95, 0x95, 0x95}
};
SDL_Color palette[16] = {
    {0x00 ,0x00 ,0x00}, //0
    {0x00 ,0x00 ,0x00}, //0
    {0xFF, 0xFF, 0xFF},
    {0xFF, 0x00, 0x00}, //2
    {0x00, 0xFF, 0xFF},
    {0xFF, 0x00, 0xFF},  //4
    {0x00, 0xFF, 0x00},
    {0x00, 0x00, 0xFF},  //6
    {0xFF, 0xFF, 0x00},
    {0xff, 0x7F, 0x00},  //8
    {0x7F, 0x2F, 0x00},
    {0xFF, 0x80, 0x80},  //10 (A)
    {0x40, 0x40, 0x40},
    {0x7f, 0x7f, 0x7f},  //12 (C)
    {0x7f, 0xff, 0x7f},
    {0x7f, 0xbf, 0xff},  //14 (E)
    {0xbf, 0xbf, 0xbf},
};




void	OpenCommodoreMonitorWindow ()
{
    // create the window that the C64 will live in
    int width = 320;
    int height = 200;
    
    theMonitorWindow = SDL_CreateWindow( "C64 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN  );
    sdlRenderer= SDL_CreateRenderer(theMonitorWindow, -1, 0);

    
    // create our offscreen bits to draw into
    theOnScreenBits = malloc(width * height);
    theOnScreenRowBytes = width;
    theActiveBits = theOnScreenBits;
    theActiveRowBytes = theOnScreenRowBytes;

    
    // create the texture that we will have the pixels draw into
    theSurface = SDL_CreateRGBSurface(0, width, height, 8,  0, 0, 0, 0);


    
    // set the palette
    SDL_SetPaletteColors(theSurface->format->palette,  palette, 0, 17);
    
#if 0
    SDL_Texture *sdlTexture = SDL_CreateTextureFromSurface(sdlRenderer,theSurface);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
    SDL_DestroyTexture(sdlTexture);
    SDL_UnlockSurface(hbm);
#endif
    
    for (int i=0;i<400;i++)
        gTheActiveBitsArray[i]=theActiveBits+(theActiveRowBytes*i);
    
    //
    //	Be sure we are in the right video mode..
    //
    VICChangeVideoModes();

    InitializeDrawingTables();

    
}

#if 0
void	OpenCommodoreMonitorWindow ()
{
	commodoreWindowInfoPtr		theWinInfo;
	GDHandle					theScreen;
#if	DRAWOFFSCREEN
	OSErr						iErr;
#endif	


	theMonitorWindow = (WindowPtr)GetNewCWindow (kMonitorWindow, NULL, (WindowPtr)-1L);
	if (theMonitorWindow==NULL)
		debug_window_printf("OpenCommodoreMonitorWindow(): GetNewCWindow returned NULL");
	else
		{
		//
		//	Setup Window Handling, so it can be dragged, and so we can update our
		//	locations after it gets dragged
		NewWindowInfo((WindowPtr)theMonitorWindow);
		SetWindowDragProc((WindowPtr)theMonitorWindow,DragCommodoreWindow);
		SetWindowUpdateProc((WindowPtr)theMonitorWindow,UpdateCommodoreWindow);

		//
		//	Make the struct to shove in our window
		//	

		theWinInfo=(commodoreWindowInfoPtr)NewPtr(sizeof(commodoreWindowInfo));
		//assert(theWinInfo!=NULL);
		if (theWinInfo==NULL)
			DebugStr("\pOpenCommodoreMonitorWindow(): NewPtr returned NULL");
		else
			{
			SetWindowRefCon((WindowPtr)theMonitorWindow,(long)theWinInfo);

			InitializeDrawingTables();


			// 
			//	Grab the commodore color table and set everything up so we will be able to
			//	do direct screen drawing with the right colors.
			//
			theScreen=GetMainDevice();
			gPixelDepth=(**(**theScreen).gdPMap).pixelSize;
			gPixelDepth=8;// AJS
			if (gPixelDepth==8)
				theWinInfo->comColors = GetCTable (kCommodoreClut);
			else
				theWinInfo->comColors = GetCTable (kCommodoreClut+4);
				
			if (theMonitorWindow==NULL)
				DebugStr("\pOpenCommodoreMonitorWindow(): GetCTable returned NULL");
			else
				{
				HLock((Handle)theWinInfo->comColors);
				theWinInfo->comPalette=NewPalette(16,theWinInfo->comColors,pmTolerant+pmAnimated+pmExplicit,0);
				if (theWinInfo->comPalette==NULL)
					DebugStr("\pOpenCommodoreMonitorWindow(): NewPalette returned NULL");
				else
					{
					NSetPalette((WindowPtr)theMonitorWindow,theWinInfo->comPalette,pmAllUpdates);
					ActivatePalette((WindowPtr)theMonitorWindow);


					if (gPixelDepth==8)
						AnimatePalette((WindowPtr)theMonitorWindow,theWinInfo->comColors,0,1,16);

		
					GetWindowPortBounds(theMonitorWindow, &theOffScreenRect);

					theOffScreenRect.bottom=theOffScreenRect.bottom-theOffScreenRect.top;
					theOffScreenRect.right=theOffScreenRect.right-theOffScreenRect.left;
					theOffScreenRect.top=0;
					theOffScreenRect.left=0;
					//theOffScreenRect=theMonitorWindow->portRect;
	
#if	DRAWOFFSCREEN



//					iErr=CreateOffScreen(&theOffScreenRect,8,(**((**theScreen).gdPMap)).pmTable/*theWinInfo->comColors*/,
//						 &theOffscreenGraf,&theOffscreenGDevice);

//					(**(**(theOffscreenGraf->portPixMap)).pmTable).ctSeed=(** (**((**theScreen).gdPMap)).pmTable).ctSeed;

//					iErr=NewGWorld(&gOffScreenWorld,8,&theOffScreenRect,(**((**theScreen).gdPMap)).pmTable,NULL, 0);
					iErr=NewGWorld(&gOffScreenWorld,8,&theOffScreenRect,NULL,NULL, 0);
					if (iErr!=noErr)
						DebugStr("\p Oh no! Out of memory while allocating GWorld");	

					gOffScreenPixMap=GetGWorldPixMap(gOffScreenWorld);
					LockPixels(gOffScreenPixMap);
					
					
					theOffScreenBits=(unsigned char *)GetPixBaseAddr(gOffScreenPixMap);
					
//					theOffScreenBits=(unsigned char *)(**(theOffscreenGraf->portPixMap)).baseAddr; 

					theOffScreenRowBytes=(long) ((**(gOffScreenPixMap)).rowBytes & 0x0007FFF);


#endif
					ToggleCommodoreWindowSize();


					UpdateBlastingParameters ();

					}
				}
			}
		}
}
#endif


//
//	Clean Up Everything We Allocated
//
void	CleanUpCommodoreWindow()
{
// TODO - dispose of stuff
    DisposeDrawingTables();

}

#if 0
void	CleanUpCommodoreWindow()
{
	commodoreWindowInfoPtr	theWinInfo;

#if	DRAWOFFSCREEN

	UnlockPixels(gOffScreenPixMap);
	DisposeGWorld(gOffScreenWorld);

#endif
//	DisposeOffScreen( theOffscreenGraf, theOffscreenGDevice);
	
	theWinInfo=(commodoreWindowInfoPtr)GetWindowRefCon((WindowPtr)theMonitorWindow);
	
	DisposePalette(theWinInfo->comPalette);
	ReleaseResource((Handle)theWinInfo->comColors);
	
	DisposeWindowInfo((WindowPtr)theMonitorWindow);
	DisposeWindow((WindowPtr)theMonitorWindow);

	DisposeDrawingTables();


}
#endif

//
//	This will draw our color bar in the top left part of the border. This is useful for
//	debugging, since it shows the color indexes in order..
//

void	BlastColorTestPattern ()
{
	register short		h,v;
	short				color;
	unsigned char 		*dstImagePosition;

//return;

	if (gPixelDepth==8)
		{
			for	(color=0;color<16;color++) 
			{
				for (v=0;v<8;v++)
					{
					dstImagePosition=theActiveBits+(theActiveRowBytes * (v+((color)*8)));
			//		dstOffImagePosition=theOffScreenBits+(theOffScreenRowBytes * (v+((color)*8)));
			
					for	(h=0;h<8;++h)
							dstImagePosition[h]=color+kComToMacClrOffset;
					}
				}
		}
	else if (gPixelDepth==4)
		{
			for	(color=0;color<16;color++) 
			{
				for (v=0;v<8;v++)
					{
					dstImagePosition=theActiveBits+(theActiveRowBytes * (v+((color)*8)));
			
					for	(h=0;h<8;++h)
							{
							dstImagePosition[h]=(unsigned char)fourBitTable[color];
							}
					}
				}
		}
}





