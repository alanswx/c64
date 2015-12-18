/*-------------------------------------------------------------------------------*\||	File:	Keyboard.c||	Description:||||||	Copyright �	1994, Alan Steremberg and Ed Wynne|\*-------------------------------------------------------------------------------*/#include "KeyboardTable.h"#include "Keyboard.h"#include <SDL2/SDL.h>/*	local constants	*/#define	kNoKey		0xFF/*	local globals	*/KeyboardScanMask	gKeyMask;KeyTranslation		*gKeyTrans;extern	Boolean	gCommdoreHasFocus;int tempKeyLookup(int key){    if (key==SDL_SCANCODE_A) return 0;    if (key==SDL_SCANCODE_S) return 1;    if (key==SDL_SCANCODE_D) return 2;    if (key==SDL_SCANCODE_F) return 3;    if (key==SDL_SCANCODE_H) return 4;    if (key==SDL_SCANCODE_G) return 5;    if (key==SDL_SCANCODE_Z) return 6;    if (key==SDL_SCANCODE_X) return 7;    if (key==SDL_SCANCODE_C) return 8;    if (key==SDL_SCANCODE_V) return 9;    if (key==SDL_SCANCODE_B) return 0x0B;    if (key==SDL_SCANCODE_Q) return 0x0C;    if (key==SDL_SCANCODE_W) return 0x0D;    if (key==SDL_SCANCODE_E) return 0x0E;    if (key==SDL_SCANCODE_R) return 0x0F;    if (key==SDL_SCANCODE_Y) return 0x10;    if (key==SDL_SCANCODE_T) return 0x11;    if (key==SDL_SCANCODE_1) return 0x12;    if (key==SDL_SCANCODE_2) return 0x13;    if (key==SDL_SCANCODE_3) return 0x14;    if (key==SDL_SCANCODE_4) return 0x15;    if (key==SDL_SCANCODE_6) return 0x16;    if (key==SDL_SCANCODE_5) return 0x17;    if (key==SDL_SCANCODE_EQUALS) return 0x18;    if (key==SDL_SCANCODE_9) return 0x19;    if (key==SDL_SCANCODE_7) return 0x1A;#if 0    {0x02,0x08,0x00,0x00},		// 11111101 MAC: $15  '4'          VIC: '4'    {0x04,0x08,0x00,0x00},		// 11111011 MAC: $16  '6'          VIC: '6'    {0x04,0x01,0x00,0x00},		// 11111011 MAC: $17  '5'          VIC: '5'    {0x40,0x20,0x00,0x00},		// 10111111 MAC: $18  '='          VIC: '='    {0x10,0x01,0x00,0x00},		// 11101111 MAC: $19  '9'          VIC: '9'    {0x08,0x01,0x00,0x00},		// 11110111 MAC: $1A  '7'          VIC: '7'    {0x20,0x08,0x00,0x00},		// 11011111 MAC: $1B  '-'          VIC: '-'    {0x08,0x08,0x00,0x00},		// 11110111 MAC: $1C  '8'          VIC: '8'    {0x10,0x08,0x00,0x00},		// 11101111 MAC: $1D  '0' (ZERO)   VIC: '0' (ZERO)    {0x40,0x04,0x02,0x80},		// 10111111 MAC: $1E  ']'          VIC: ']' (; + SHIFT)    {0x10,0x40,0x00,0x00},		// 11101111 MAC: $1F  'O'          VIC: 'O'    {0x08,0x40,0x00,0x00},		// 11110111 MAC: $20  'U'          VIC: 'U'    {0x20,0x20,0x02,0x80},		// 11011111 MAC: $21  '['          VIC: '[' (: + SHIFT)    {0x10,0x02,0x00,0x00},		// 11101111 MAC: $22  'I'          VIC: 'I'    {0x20,0x02,0x00,0x00},		// 11011111 MAC: $23  'P'          VIC: 'P'    {0x01,0x02,0x00,0x00},		// 11111110 MAC: $24  RETURN       VIC: RETURN    {0x20,0x04,0x00,0x00},		// 11011111 MAC: $25  'L'          VIC: 'L'    {0x10,0x04,0x00,0x00},		// 11101111 MAC: $26  'J'          VIC: 'J'    {0x08,0x01,0x02,0x80},		// 11110111 MAC: $27  '''          VIC: ''' (7 + SHIFT)    {0x10,0x20,0x00,0x00},		// 11101111 MAC: $28  'K'          VIC: 'K'    {0x40,0x04,0x00,0x00},		// 10111111 MAC: $29  ';'          VIC: ';'    {0x40,0x40,0x00,0x00},		// 10111111 MAC: $2A  '|'          VIC: '^' UPWARDS ARROW GRAPHIC    {0x20,0x80,0x00,0x00},		// 11011111 MAC: $2B  ','          VIC: ','    {0x40,0x80,0x00,0x00},		// 10111111 MAC: $2C  '/'          VIC: '/'    {0x10,0x80,0x00,0x00},		// 11101111 MAC: $2D  'N'          VIC: 'N'    {0x10,0x10,0x00,0x00},		// 11101111 MAC: $2E  'M'          VIC: 'M'    {0x20,0x10,0x00,0x00},		// 11011111 MAC: $2F  '.'          VIC: '.'    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $30  TAB          VIC: NONE    {0x80,0x10,0x00,0x00},		// 01111111 MAC: $31  SPACE        VIC: SPACE    {0x80,0x02,0x00,0x00},		// 01111111 MAC: $32  '`'          VIC: '<-' LEFTWARDS ARROW GRAPHIC    {0x01,0x01,0x00,0x00},		// 11111110 MAC: $33  DELETE       VIC: INST/DELETE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $34  NONE         VIC: NONE    {0x80,0x80,0x00,0x00},		// 01111111 MAC: $35  ESC          VIC: RUN/STOP    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $36  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $37  COMMAND      VIC: NONE    {0x40,0x10,0x00,0x00},		// 10111111 MAC: $38  SHIFT        VIC: RIGHT SHIFT    {0x02,0x80,0x00,0x00},		// 11111101 MAC: $39  CAPS LOCK    VIC: LEFT SHIFT (CAPS LOCK)    {0x80,0x20,0x00,0x00},		// 01111111 MAC: $3A  OPTION       VIC: COMMODORE KEY    {0x80,0x04,0x00,0x00},		// 01111111 MAC: $3B  CONTROL      VIC: CONTROL    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $3C  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $3D  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $3E  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $3F  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $40  NONE         VIC: NONE    {0x20,0x10,0x00,0x00},		// 11011111 MAC: $41  '.' (KEYPAD) VIC: '.'    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $42  NONE         VIC: NONE    {0x40,0x02,0x00,0x00},		// 10111111 MAC: $43  '*' (KEYPAD) VIC: '*'    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $44  NONE         VIC: NONE    {0x20,0x01,0x00,0x00},		// 11011111 MAC: $45  '+' (KEYPAD) VIC: '+'    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $46  NONE         VIC: NONE    {0x40,0x08,0x02,0x80},		// 10111111 MAC: $47  CLR (KEYPAD) VIC: CLR/HOME + SHIFT    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $48  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $49  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $4A  NONE         VIC: NONE    {0x40,0x80,0x00,0x00},		// 10111111 MAC: $4B  '/' (KEYPAD) VIC: '/'    {0x01,0x02,0x00,0x00},		// 11111110 MAC: $4C  ENTER        VIC: RETURN    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $4D  NONE         VIC: NONE    {0x20,0x08,0x00,0x00},		// 11011111 MAC: $4E  '-' (KEYPAD) VIC: '-'    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $4F  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $50  NONE         VIC: NONE    {0x40,0x20,0x00,0x00},		// 10111111 MAC: $51  '=' (KEYPAD) VIC: '='    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $52  '0' (ZEROKP) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $53  '1' (KEYPAD) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $54  '2' (KEYPAD) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $55  '3' (KEYPAD) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $56  '4' (KEYPAD) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $57  '5' (KEYPAD) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $58  '6' (KEYPAD) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $59  '7' (KEYPAD) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $5A  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $5B  '8' (KEYPAD) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $5C  '9' (KEYPAD) VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $5D  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $5E  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $5F  NONE         VIC: NONE    {0x01,0x40,0x00,0x00},		// 11111110 MAC: $60  F5           VIC: F5    {0x01,0x40,0x02,0x80},		// 11111110 MAC: $61  F6           VIC: F6 (F5 + SHIFT)    {0x01,0x08,0x00,0x00},		// 11111110 MAC: $62  F7           VIC: F7    {0x01,0x20,0x00,0x00},		// 11111110 MAC: $63  F3           VIC: F3    {0x01,0x08,0x02,0x80},		// 11111110 MAC: $64  F8           VIC: F8 (F7 + SHIFT)    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $65  F9           VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $66  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $67  F11          VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $68  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $69  F13          VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $6A  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $6B  F14          VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $6C  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $6D  F10          VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $6E  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $6F  F12          VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $70  NONE         VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $71  F15          VIC: NONE    {0x01,0x01,0x02,0x80},		// 11111110 MAC: $72  INSERT       VIC: INST/DEL + SHIFT    {0x40,0x08,0x00,0x00},		// 10111111 MAC: $73  HOME         VIC: CLR/HOME    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $74  PAGEUP       VIC: NONE    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $75  DEL          VIC: NONE    {0x01,0x20,0x02,0x80},		// 11111110 MAC: $76  F4           VIC: F4 (F3 + SHIFT)    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $77  END          VIC: NONE    {0x01,0x10,0x02,0x80},		// 11111110 MAC: $78  F2           VIC: F2 (F1 + SHIFT)    {0x00,0x00,0x00,0x00},		// 00000000 MAC: $79  PAGEDOWN     VIC: NONE    {0x01,0x10,0x00,0x00},		// 11111110 MAC: $7A  F1           VIC: F1    {0x01,0x04,0x02,0x80},		// 11111110 MAC: $7B  RIGHT ARROW  VIC: RIGHT/LEFT CRSR + SHIFT (LEFT)    {0x01,0x04,0x00,0x00},		// 11111110 MAC: $7C  RIGHT ARROW  VIC: RIGHT/LEFT CRSR (RIGHT)    {0x01,0x80,0x00,0x00},		// 11111110 MAC: $7D  DOWN ARROW   VIC: UP/DOWN CRSR (DOWN)    {0x01,0x80,0x02,0x80},		// 11111110 MAC: $7E  UP ARROW     VIC: UP/DOWN CRSR + SHIFT (UP)    {0x00,0x00,0x00,0x00}		// 00000000 MAC: $7F  NONE         VIC: NONE#endif        return kNoKey;}unsigned long ReadKeyboardRow(void){	register unsigned char	byte;	register unsigned char	bit;	register unsigned char	cur;	unsigned char				keys[16];	unsigned long				val;    int keyboard;	    	//if (!gCommdoreHasFocus)	//	return	kNoKey;        SDL_PumpEvents();    keyboard = SDL_GetKeyboardState(NULL);    for (byte=0;byte<16;byte++)    {        keys[byte]=0;    }    keys[7]=tempKeyLookup(keyboard);        fprintf(stderr,"ReadKeyboardRow %d %d\n",keyboard,keys[7]);#if 0    GetKeys((long*)&keys[0]);	if (keys[6]&0x80)		return kNoKey;#endif		val = 0;	keys[15] &= 0x7F;	gKeyTrans = (keys[7]&0x03) ? ShiftTranslationTable : NormalTranslationTable;	keys[7] &= 0xFC;		for (byte=0;byte<16;byte++)		if ((cur = keys[byte]) != 0)			for (bit=0;bit<8;bit++)			{				if (cur&0x01)					val |= ConvertKeyCode((byte<<3)+bit);								cur >>= 1;			}	return (0xFF-val);}unsigned long ConvertKeyCode(unsigned long key){	unsigned long	result = 0;			if (gKeyTrans[key].mask & ~gKeyMask.byte)		result |= gKeyTrans[key].val;	if (gKeyTrans[key].mask2 & ~gKeyMask.byte)		result |= gKeyTrans[key].val2;	return result;}