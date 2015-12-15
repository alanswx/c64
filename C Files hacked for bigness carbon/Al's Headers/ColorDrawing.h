/*-------------------------------------------------------------------------------*\
|
|	File:	ColorDrawing.h
|
|	Description:
|
|	This file contains prototypes for the routines that set up and dispose of the
|	commodore's color world.
|
|
|	Copyright �	1994, Alan Steremberg and Ed Wynne
|
\*-------------------------------------------------------------------------------*/


#ifndef		_COLORDRAWING_
#define		_COLORDRAWING_



#ifndef __PALETTES__
//#include <Palettes.h>
#endif
typedef void * PaletteHandle;

#define	DRAWOFFSCREEN	0

void	OpenCommodoreMonitorWindow (void);
void	CleanUpCommodoreWindow		(void);
void	UpdateBlastingParameters 	(void);
void	BlastColorTestPattern 		(void);
void	WriteDispatch					(unsigned short	addr, char	val);
void 	DragCommodoreWindow			(WindowPtr	window,long refcon,Point start);
void	CopyOffScreenToWindow		(void);
void 	UpdateCommodoreWindow		(WindowPtr window,long refcon);
void 	DoBlastTest						(long value);
void	CheckAndFixPixelDepth		(void);
void 	ToggleCommodoreWindowSize 	(void);

void	SetUpMontorDepths(void);
void	CleanUpMonitorDepths(void);

typedef	struct commodoreWindowStruct {
	CTabHandle 			comColors;
	PaletteHandle		comPalette;
	}	commodoreWindowInfo, *commodoreWindowInfoPtr;
	
#endif


//
//	We can't seem to allocate black as the first color, so we need to offset the 
//	commodore colors by one (add) to get to the mac colors
//

#define	kComToMacClrOffset	1

