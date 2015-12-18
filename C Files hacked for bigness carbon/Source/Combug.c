/*-------------------------------------------------------------------------------*\
|
|	File:	Combug.c
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
#include <stdarg.h>
#include <Carbon/Carbon.h>
#include "DebugWindow.h"
#include "Disassembler.h"
#include "Preference.h"
#include "Window.h"
#include "Menu.h"
#include "Accessors.h"
#include "Memory.h"
#include "OpcodeDefs.h"
#include "CombugWindow.h"
#include "CombugConsole.h"
#include "Combug.h"





/*	local constants	*/
#define	kCombugWindowLocPref				'CbgL'
#define	kCombugWindowLocPrefID			128
#define	kCombugWindowVisiblePref		'CbgV'
#define	kCombugWindowVisiblePrefID		128
#define	kCombugWINDRsrcID					256
#define	kCombugMENURsrcID					256
#define	kVLineOffset						10
#define	kBottomOffset						gHeight-4
#define	kDisAssemLines						8
#define	kMaxAddrModeSymbolLength		22
#define	kNumScrollBackLines				(((kBottomOffset-kVLineOffset*kDisAssemLines-2)/kVLineOffset) -1 )
#define	kScrollBackBufferSize			200


/*	external globals	*/
extern unsigned char			*gRAMBlock;							//	Memory.c
extern ReadHandler			*gMemRead;							//	Memory.c
extern WriteHandler			*gMemWrite;							//	Memory.c


extern unsigned long			gACCRegister;						//	CPUState.c
extern unsigned long			gINXRegister;						//	CPUState.c
extern unsigned long			gINYRegister;						//	CPUState.c
extern unsigned long			gSTACKRegister;					//	CPUState.c
extern unsigned long			gPCRegister;						//	CPUState.c
extern unsigned long			gSRRegister;						//	CPUState.c
extern Ptr						gCombugScrollBackBuffer;
extern unsigned	long		gCombugTotalScrollBackLines;
extern unsigned	long		gCombugScrollValue;
extern char						gCommandLineBuffer[];
extern	WindowPtr			theMonitorWindow;



/*	local globals	*/
WindowPtr		gCombugWindow = NULL;
GWorldPtr		gCombugWorld = NULL;
short				gHeight = 0;
short				gWidth;
CombugState		gCombugState;
unsigned short	gNumSymbols = 0;
Ptr				gSymbolTable = NULL;





void InitCombug(void)
{

}
void DrawCombugWindow(void)
{
    
}
#if 0
void InitCombug(void)
{
    Handle	data;
	Point	where;
	short	visible;
	
	Rect	thePortRect;

	//
//	Allocate space for the scrollback
//
	gCombugScrollBackBuffer = (char*)calloc(70,kScrollBackBufferSize);
//	assert (gCombugScrollBackBuffer!=NULL);
	if (gCombugScrollBackBuffer==NULL)
		DebugStr("\p Died in initialization of scrollback buffer");
	gCombugTotalScrollBackLines = 0;
	gCombugScrollValue=0;

	gCombugWindow = GetNewWindow(kCombugWINDRsrcID,NULL,(WindowPtr)(-1L));
	ShowWindow(gCombugWindow); // AJS
	data = GetPreference(kCombugWindowLocPref,kCombugWindowLocPrefID);
	if (data)
	{
		BlockMove(*data,&where,sizeof(Point));
		DisposeHandle(data);
		
		MoveWindow(gCombugWindow,where.h,where.v,false);
	}
	
	NewWindowInfo(gCombugWindow);
	SetWindowCloseProc(gCombugWindow,CombugWindowCloseProc);
	SetWindowUpdateProc(gCombugWindow,CombugWindowUpdateProc);
	SetWindowKeyProc(gCombugWindow,CombugWindowKeyProc);
	
	InsertPlainMenu(kCombugMENURsrcID,1000);
	AddItemProc(CombugMenuProc,kCombugMENURsrcID,kAllItems);
	DrawMenuBar();
	
	data = Get1Resource('CSYM',256);
	if (data)
	{
		gSymbolTable = NewPtrClear(GetHandleSize(data)-2);
	//	assert(gSymbolTable!=NULL);
		if (gSymbolTable==NULL)
			DebugStr("\p Failed to allocate symbol table");
		BlockMove(*data+2,gSymbolTable,GetHandleSize(data)-2);
		gNumSymbols = *(short*)(*data);
		ReleaseResource(data);
	}
		
	GetWindowPortBounds(gCombugWindow, &thePortRect);

	//assert(NewGWorld(&gCombugWorld,1,&gCombugWindow->portRect,NULL,NULL,0)==noErr);
	(NewGWorld(&gCombugWorld,1,&thePortRect,NULL,NULL,0));
	LockPixels(GetGWorldPixMap(gCombugWorld));
	CopyBits((BitMap*)*GetGWorldPixMap(gCombugWorld),(BitMap*)*GetGWorldPixMap(gCombugWorld),
				&thePortRect,&thePortRect,srcBic,NULL);
	
	gHeight = thePortRect.bottom-thePortRect.top;
	gWidth = thePortRect.right-thePortRect.left;
	
	gCombugState.halted = 1;
	gCombugState.step = 0;
	gCombugState.symbols = 1;
	gCombugState.gto=0;
	
	DrawCombugWindow();
	data = GetPreference(kCombugWindowVisiblePref,kCombugWindowVisiblePrefID);
	if (data)
	{
		BlockMove(*data,&visible,sizeof(short));
		DisposeHandle(data);
		
		if (visible)
			ShowWindow(gCombugWindow);
		else
			ToggleCombugWindow();
	}
}

#endif



void CleanUpCombug(void)
{
    
}
#if 0
void CleanUpCombug(void)
{
	GrafPtr	savedPort;
	Handle	data;
	Point	where;
	short	visible;
	Rect	thePortRect;
	
	
	if (gCombugWorld)
		DisposeGWorld(gCombugWorld);
	
	if (gCombugWindow)
	{
		GetWindowPortBounds(gCombugWindow, &thePortRect);

		data = NewHandle(sizeof(Point));
//		assert(data!=NULL);
		if (data==NULL)
			DebugStr("\p Failed to allocate memory in CleanUpCombug");
		SetPt(&where,thePortRect.left,thePortRect.top);
		
		GetPort(&savedPort);
		SetPortWindowPort(gCombugWindow);
		LocalToGlobal(&where);
		SetPort(savedPort);
		BlockMove(&where,*data,sizeof(Point));
		SetPreference(kCombugWindowLocPref,kCombugWindowLocPrefID,data);
		DisposeHandle(data);
		visible = IsWindowVisible(gCombugWindow);
		//visible = ((WindowPeek)gCombugWindow)->visible;
		data = NewHandle(sizeof(short));
	//	assert(data!=NULL);
		if (data==NULL)
			DebugStr("\p Failed to allocate memory in CleanUpCombug");
		BlockMove(&visible,*data,sizeof(short));
		SetPreference(kCombugWindowVisiblePref,kCombugWindowVisiblePrefID,data);
		DisposeHandle(data);
		
		DisposeWindow(gCombugWindow);
	}
}

#endif



void CombugWindowCloseProc(WindowPtr window,long refcon)
{
	ToggleCombugWindow();
}




#if 0
void CombugWindowUpdateProc(WindowPtr window,long refcon)
{
	PixMapHandle	pixBase=GetGWorldPixMap(gCombugWorld);
	Rect	thePortRect;
	
	//
	//	assuming pixels are already locked when the gworld is allocated!
	//
	GetWindowPortBounds(window, &thePortRect);

	CopyBits((BitMap*)*pixBase,
				GetPortBitMapForCopyBits(GetWindowPort(window)),
				&thePortRect,
				&thePortRect,srcCopy,NULL);




}

#endif


#if 0

void CombugMenuProc(short menu,short item)
{
	switch(item)
	{
		case 1:
			ToggleCombugWindow();
			break;
		case 3:
			gCombugState.halted = 0;
			SelectWindow((WindowPtr)theMonitorWindow);
			break;
		case 4:
			if (gCombugState.halted)
				gCombugState.step = 1;
			break;
		case 5:
			gCombugState.halted = 1;
			gCombugState.gto=0;
			SelectWindow(gCombugWindow);
			break;
		case 6:
			gCombugState.symbols = !gCombugState.symbols;
			break;
	}
	
	DrawCombugWindow();
}

#endif



void ToggleCombugWindow(void)
{
#if 0
	MenuHandle	menu;
	Str255		text;
	
	
	static int shown=0;
	
	if (shown)
	{
		HideWindow(gCombugWindow);
		shown=0;
		
	}
	else
	{
		ShowWindow(gCombugWindow);
		shown=1;
	}
	menu = GetMenu(kCombugMENURsrcID);
	GetMenuItemText(menu,1,text);
	
	if (text[1] == 'S')
	{
		SetMenuItemText(menu,1,"\pHide Combug");
		ShowWindow(gCombugWindow);
	}
	else
	{
		SetMenuItemText(menu,1,"\pShow Combug");
		HideWindow(gCombugWindow);
	}
#endif
}




#if 0
void DrawCombugWindow(void)
{
	GDHandle	gdh;
	CGrafPtr	port;
	Rect		thePortRect;

	
	GetGWorld(&port,&gdh);
	SetGWorld(gCombugWorld,NULL);
	
	TextFont(4);
	TextSize(9);
	TextMode(srcOr);
	
	//
   GetWindowPortBounds(gCombugWindow, &thePortRect);
	EraseRect(&thePortRect);
	
	MoveTo(50,0);
	LineTo(50,gHeight);
	MoveTo(50,gHeight-kVLineOffset-4);
	LineTo(gWidth,gHeight-kVLineOffset-4);
	MoveTo(50,gHeight-kVLineOffset*(kDisAssemLines+1)-7);
	LineTo(gWidth,gHeight-kVLineOffset*(kDisAssemLines+1)-7);
	
	DrawRegisters();
	DrawDisassembly();
	CombugDrawCommandLine(gCommandLineBuffer);
	CombugDrawScrollBackRegion();
	
	SetGWorld(port,gdh);
	GetPort((GrafPtr*)&port);
	SetPortWindowPort(gCombugWindow);
	CombugWindowUpdateProc(gCombugWindow,GetWindowRefCon(gCombugWindow));
	SetPort((GrafPtr)port);
}

#endif




void CombugPrintf(char *format,...)
{
	char		string[512];
	va_list	argptr;
	long		len;
	
	
	va_start(argptr,format);
	len = (long)vsprintf(string,format,argptr);
	va_end (argptr);
	
//	CtoPstr(string);
//	DrawString((StringPtr)string);
}



void DrawRegisters(void)
{
    
}
#if 0
void DrawRegisters(void)
{
	char	s[10],t[10];
	unsigned long	data;
	long	index;
	
	
	MoveTo(10,kBottomOffset);
	CombugPrintf("Y: %02lX",gINYRegister);
	MoveTo(10,kBottomOffset-kVLineOffset-1);
	CombugPrintf("X: %02lX",gINXRegister);
	MoveTo(10,kBottomOffset-kVLineOffset*2-2);
	CombugPrintf("A: %02lX",gACCRegister);
	
	MoveTo(8,kBottomOffset-kVLineOffset*5);
	CombugPrintf("SR: %02lX",gSRRegister);
	MoveTo(5,kBottomOffset-kVLineOffset*4);
	CombugPrintf("%c%c%c%c%c%c%c",(gSRRegister & 0x80) ? 'N' : 'n',
				(gSRRegister & 0x40) ? 'V' : 'v',(gSRRegister & 0x10) ? 'B' : 'b',
				(gSRRegister & 0x08) ? 'D' : 'd',(gSRRegister & 0x04) ? 'I' : 'i',
				(gSRRegister & 0x02) ? 'Z' : 'z',(gSRRegister & 0x01) ? 'C' : 'c');
	
	MoveTo(10,kVLineOffset+1);
	CombugPrintf("S: %02lX",gSTACKRegister);
	
	for (index=0;index<11;index++)
	{
		MoveTo(5,kVLineOffset*12-kVLineOffset*index+4);
		
		sprintf(s,"%02lX",(unsigned long)gRAMBlock[0x100+gSTACKRegister+index*2+1]);
		sprintf(t,"%02lX",(unsigned long)gRAMBlock[0x100+gSTACKRegister+index*2+2]);
		
		if ((gSTACKRegister+index*2+1) > 0xFF)
			s[0] = s[1] = ' ';
		if ((gSTACKRegister+index*2+2) > 0xFF)
			t[0] = t[1] = ' ';
		
		CombugPrintf("%02lX %s%s",(unsigned long)((unsigned char)(gSTACKRegister+1+(index*2))),s,t);
	}
}


#endif



void DrawDisassembly(void)
{
    
}
#if 0
void DrawDisassembly(void)
{
	unsigned long	op,len,index;
	unsigned short	base;
	char				s[100],line[256];
	
	
	base = FindCombugSymbol(s,gPCRegister);
	MoveTo(60,kBottomOffset-kVLineOffset*kDisAssemLines-2);
	CombugPrintf("%s",s);
	
	op = MEMREADBYTE(gPCRegister);
	if (OpInfoTable[op].mode == REL)
	{
		MoveTo(gWidth-102,kBottomOffset-kVLineOffset*kDisAssemLines-2);
		if (BranchPredict(op))
			CombugPrintf("Will branch");
		else
			CombugPrintf("Will not branch");
	}
	
	len = 0;
	for (index=1;index<kDisAssemLines;index++)
	{
		MoveTo(60,kBottomOffset-kVLineOffset*kDisAssemLines+kVLineOffset*index-2);
		len += DisasmOneOpcode(line,gPCRegister+len);
		CombugPrintf("%s",line);
	}
}

#endif



long BranchPredict(unsigned char op)
{
	switch(op)
	{
		case 0x10:  //	BPL
			return !(gSRRegister&0x80);
		case 0x30:  //	BMI
			return (gSRRegister&0x80);
		case 0x50:  //	BVC
			return !(gSRRegister&0x40);
		case 0x70:  //	BVS
			return (gSRRegister&0x40);
		case 0x90:  //	BCC
			return !(gSRRegister&0x01);
		case 0xB0:  //	BCS
			return (gSRRegister&0x01);
		case 0xD0:  //	BNE
			return !(gSRRegister&0x02);
		case 0xF0:  //	BEQ
			return (gSRRegister&0x02);
	}
	
	return 0;
}





unsigned short DisasmOneOpcode(char *line,unsigned short address)
{
	unsigned long	op,data,index;
	unsigned short	base,target,has_target;
	char	sym[256],temp[100];
	
	
	op = MEMREADBYTE(address);
	base = FindCombugSymbol(sym,address);
	
	if (base)
		sprintf(temp,"  +%04lX %04lX ",(long int)(address-base),(long int)address);
	else
		sprintf(temp,"        %04lX ",(long int)address);
	strcpy(line,temp);
	
	if (address == gPCRegister)
		strcat(line,"*");
	else
		strcat(line," ");
	
	strcat(line,OpInfoTable[op].op);
	strcat(line," ");
	
	has_target = 0;
	switch(OpInfoTable[op].mode)
	{
		case ABS:
			data = MEMREADWORD(address+1);
			strcpy(temp,"$");
			DecToHexStr(temp+1,data);
			break;
		case ABS|JF:
			data = MEMREADWORD(address+1);
			FindAddrModeSymbol(temp,data);
			target = data;has_target = 1;
			break;
		case ABSX:
			data = MEMREADWORD(address+1);
			sprintf(temp,"$%04lX,X",data);
			target = data + gINXRegister;has_target = 1;
			break;
		case ABSY:
			data = MEMREADWORD(address+1);
			sprintf(temp,"$%04lX,Y",data);
			target = data + gINYRegister;has_target = 1;
			break;
		case ABSIND:
			data = MEMREADWORD(address+1);
			sprintf(temp,"($%04lX)",data);
			data = MEMREADWORD(data);
			target = data;has_target = 1;
			break;
		case ABSIND|JF:
			data = MEMREADWORD(address+1);
			data = MEMREADWORD(data);
			FindAddrModeSymbol(temp,data);
			target = data;has_target = 1;
			break;
		case ACC:
			strcpy(temp,"A");
			break;
		case IMM:
			data = MEMREADBYTE(address+1);
			sprintf(temp,"#$%02lX",data);
			break;
		case REL:
			data = MEMREADWORD(address+1);
			FindRelativeSymbol(temp,address+2,data);
			target = address+2+(char)data;has_target = 1;
			break;
		case ZP:
			data = MEMREADBYTE(address+1);
			sprintf(temp,"$%02lX",data);
			break;
		case ZPX:
			data = MEMREADBYTE(address+1);
			sprintf(temp,"$%02lX,X",data);
			target = (unsigned char)(data + gINXRegister);has_target = 1;
			break;
		case ZPXIND:
			data = MEMREADBYTE(address+1);
			sprintf(temp,"($%02lX,X)",data);
			target = MEMREADWORD((unsigned char)(data + gINXRegister));has_target = 1;
			break;
		case ZPY:
			data = MEMREADBYTE(address+1);
			sprintf(temp,"$%02lX,Y",data);
			target = (unsigned char)(data + gINYRegister);has_target = 1;
			break;
		case ZPINDY:
			data = MEMREADBYTE(address+1);
			sprintf(temp,"($%02lX),Y",data);
			target = (unsigned short)(MEMREADWORD(data) + gINYRegister);has_target = 1;
			break;
		default:
			temp[0] = '\0';
			break;
	}
	strcat(line,temp);
	
	while(strlen(line) < (kMaxAddrModeSymbolLength+26))
		strcat(line," ");
		
	if (has_target)
	{
		sprintf(temp,"; %04lX | ",(unsigned long)target);
		strcat(line,temp);
	}
	else
		strcat(line,"       | ");
	
	sprintf(temp,"%02lX ",(unsigned long)op);
	strcat(line,temp);
	for (index=1;index<OpInfoTable[op].len;index++)
	{
		data = MEMREADBYTE(address+index);
		sprintf(temp,"%02lX",(unsigned long)data);
		strcat(line,temp);
	}
	
	return OpInfoTable[op].len;
}





void FindRelativeSymbol(char *symbol,unsigned short address,char offset)
{
	unsigned short	base;
	char	name[256];
	
	
	base = FindCombugSymbol(name,address-2);
	if (!base)
	{
		if (offset<0)
			sprintf(symbol,"*-$%02lX",(unsigned long)-offset);
		else
			sprintf(symbol,"*+$%02lX",(unsigned long)offset);
	}
	else
	{
		if (strlen(name) > kMaxAddrModeSymbolLength)
			name[kMaxAddrModeSymbolLength] = '\0';
		sprintf(symbol,"%s+$%04lX",name,(unsigned long)(address-base+offset));
	}
}





void FindAddrModeSymbol(char *symbol,unsigned short address)
{
	unsigned short	base;
	char	name[256];
	
	
	base = FindCombugSymbol(name,address);
	if (!base)
	{
		sprintf(symbol,"$%04X",address);
		return;
	}
	
	if (base == address)
	{
		if (strlen(name) > (kMaxAddrModeSymbolLength+6))
			name[kMaxAddrModeSymbolLength+6] = '\0';
		strcpy(symbol,name);
	}
	else
	{
		if (strlen(name) > kMaxAddrModeSymbolLength)
			name[kMaxAddrModeSymbolLength] = '\0';
		sprintf(symbol,"%s+$%04lX",name,(unsigned long)(address-base));
	}
}





unsigned short FindCombugSymbol(char *symbol,unsigned short address)
{
	unsigned short	num,match;
	unsigned char	*walk;
	SymbolEntry		*sym;
	
	
	match = 0;
	num = gNumSymbols;
	walk = (unsigned char*)gSymbolTable;
	
	if (gCombugState.symbols && walk)
		while(num--)
		{
			sym = (SymbolEntry*)walk;
			if ((sym->address <= address) && (sym->address > match))
			{
				match = sym->address;
			//	PtoCstr(sym->symbol);
				strcpy(symbol,(char*)sym->symbol);
			//	CtoPstr((char*)sym->symbol);
			}
			
			walk += sym->symbol[0]+3 + (sym->symbol[0]+3)%2;
		}
	
	if (!match)
		strcpy(symbol,"No procedure name");
	return match;
}