/*-------------------------------------------------------------------------------*\||	File:	CIA2.h||	Description:||||||	Copyright �	1994, Alan Steremberg and Ed Wynne|\*-------------------------------------------------------------------------------*/#ifndef	_CIA2_#define	_CIA2_#ifndef	_GLOBALDEFS_#include "GlobalDefs.h"#endif	_GLOBALDEFS_typedef union CIA2ICR		//	CIA2 Interrupt Control Register{	struct {		unsigned char	NMIFlag	: 1;		unsigned char				: 2;		unsigned char	FLAG1		: 1;		unsigned char	serial	: 1;		unsigned char				: 1;		unsigned char	timerB	: 1;		unsigned char	timerA 	: 1;	} bits;	unsigned char byte;} CIA2ICR;typedef union CIA2CRA		//	CIA2 Control Register A{	struct {		unsigned char	TODIn		: 1;		unsigned char	SPMode	: 1;		unsigned char	INMode	: 1;		unsigned char	load		: 1;		unsigned char	RUNMode	: 1;		unsigned char	OUTMode	: 1;		unsigned char	PBOn		: 1;		unsigned char	start 	: 1;	} bits;	unsigned char byte;} CIA2CRA;typedef union CIA2CRB		//	CIA2 Control Register B{	struct {		unsigned char	alarm		: 1;		unsigned char	INMode	: 2;		unsigned char	load		: 1;		unsigned char	RUNMode	: 1;		unsigned char	OUTMode	: 1;		unsigned char	PBOn		: 1;		unsigned char	start 	: 1;	} bits;	unsigned char byte;} CIA2CRB;typedef union CIA2TimerLatch{	struct {		unsigned char	unused1;		unsigned char	unused2;		unsigned char	high;		unsigned char	low;	} bytes;	long	val;} CIA2TimerLatch;void CIA2Write(unsigned long address,unsigned long value);//#ifndef powerc//unsigned long CIA2Read(unsigned long address:__A0);//#elseunsigned long CIA2Read(unsigned long address);//#endifvoid CIA2TimerAExpire(void);short CIA2InterruptPending(void);#endif	_CIA2_