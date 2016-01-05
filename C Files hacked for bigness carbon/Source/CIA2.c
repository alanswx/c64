/*-------------------------------------------------------------------------------*\||	File:	CIA2.c||	Description:||||||	Copyright �	1994, Alan Steremberg and Ed Wynne|\*-------------------------------------------------------------------------------*/#include "DebugWindow.h"#include "Drawing.h"#include "Interrupts.h"#include "CIA2.h"/*	local constants	*/#define	kCIA2BaseAddress	0xDD00#define	LOCALDEBUG			0/*	local globals	*/unsigned char	gCIA2Reg[16];CIA2ICR			gCIA2NMIICR;CIA2ICR			gCIA2MaskICR;CIA2CRA			gCIA2CRegA;CIA2CRB			gCIA2CRegB;CIA2TimerLatch	gCIA2TimerALatch;long				gCIA2TimerACount;void CIA2Write(unsigned long address,unsigned long value){	register	unsigned long	reg;	register	unsigned long	val;			val = value;	reg = (address-kCIA2BaseAddress)&0x0F;		switch(reg)	{		case 0x00:		//	Data Port A (Serial Bus, RS-232, VIC Memory Control)				/*				Data Port A (Serial Bus, RS-232, VIC Memory Control)					7	Serial Bus Data Input					6	Serial Bus Clock Pulse Input					5	Serial Bus Data Output					4	Serial Bus Clock Pulse Output					3	Serial Bus ATN Signal Output					2	RS-232 Data Output (User Port)					1-O	VIC Chip System Memory Bank Select (Default = 11)				*/				//				//	Update based on the two VIC bits.				//				gCIA2Reg[reg] = val;				CalculateVideoRamOffsets ();			break;		case 0x01:		//	Data Port B (User Port, RS-232)			break;		case 0x02:		//	Data Direction Register -- Port A			break;		case 0x03:		//	Data Direction Register -- Port B			break;		case 0x04:		//	Timer A: Low-Byte			//	store low byte of timer latch			gCIA2TimerALatch.bytes.low = val;			break;		case 0x05:		//	Timer A: High-Byte			//	store high byte of timer latch and store latch into counter			gCIA2TimerALatch.bytes.high = val;			gCIA2TimerACount = htonl(gCIA2TimerALatch.val);						//	if timer is running, restart it with new value//			if (gCIA2CRegA.bits.start)            if (gCIA2CRegA.byte&0x01)				SetCIA2TimerAInterruptCounter(gCIA2TimerACount);			break;		case 0x06:		//	Timer B: Low-Byte			break;		case 0x07:		//	Timer B: High-Byte			break;		case 0x08:		//	Time-of-Day Clock: 1/10 Seconds			break;		case 0x09:		//	Time-of-Day Clock: Seconds			break;		case 0x0A:		//	Time-of-Day Clock: Minutes			break;		case 0x0B:		//	Time-of-Day Clock: Hours + AM/PM Flag (Bit 7)			break;		case 0x0C:		//	Synchronous Serial I/O Data Buffer			break;		case 0x0D:		//	CIA Interrupt Control Register (Read NMIs/Write Mask)			if (val & 0x80)				gCIA2MaskICR.byte |= (val & 0x1F);			else				gCIA2MaskICR.byte &= ~(val & 0x1F);			break;		case 0x0E:		//	CIA Control Register A			gCIA2CRegA.byte = (val & 0xEF);			if (val & 0x10)			{				//	force load timer counter with latch value				gCIA2TimerACount = htonl(gCIA2TimerALatch.val);								//	if timer is running, restart it with new value                if (gCIA2CRegA.byte&0x01)//				if (gCIA2CRegA.bits.start)					SetCIA2TimerAInterruptCounter(gCIA2TimerACount);			}			break;		case 0x0F:		//	CIA Control Register B			gCIA2CRegB.byte = (val & 0xEF);			if (val & 0x10)				;//	do timer force load			break;		default:			break;	}		#if	LOCALDEBUG && DEBUG		debug_window_printf("CIA2Write: reg: %X, val: %X",(int)reg,(int)val);	#endif	gCIA2Reg[reg] = val;}//#ifndef	powerc//unsigned long CIA2Read(unsigned long address:__A0)//#elseunsigned long CIA2Read(unsigned long address)//#endif{	register	unsigned long	reg;	register	unsigned long	val;			reg = (address-kCIA2BaseAddress)&0x0F;	val = gCIA2Reg[reg];		switch(reg)	{		case 0x00:		//	Data Port A (Serial Bus, RS-232, VIC Memory Control)			break;		case 0x01:		//	Data Port B (User Port, RS-232)			break;		case 0x02:		//	Data Direction Register -- Port A			break;		case 0x03:		//	Data Direction Register -- Port B			break;		case 0x04:		//	Timer A: Low-Byte			val = gCIA2TimerALatch.bytes.low;			break;		case 0x05:		//	Timer A: High-Byte			val = gCIA2TimerALatch.bytes.high;			break;		case 0x06:		//	Timer B: Low-Byte			#if	LOCALDEBUG && DEBUG				debug_window_printf("trying to read timerb");			#endif			break;		case 0x07:		//	Timer B: High-Byte			#if	LOCALDEBUG && DEBUG				debug_window_printf("trying to read timerb");			#endif			break;		case 0x08:		//	Time-of-Day Clock: 1/10 Seconds			break;		case 0x09:		//	Time-of-Day Clock: Seconds			break;		case 0x0A:		//	Time-of-Day Clock: Minutes			break;		case 0x0B:		//	Time-of-Day Clock: Hours + AM/PM Flag (Bit 7)			break;		case 0x0C:		//	Synchronous Serial I/O Data Buffer			break;		case 0x0D:		//	CIA Interrupt Control Register (Read NMIs/Write Mask)			val = gCIA2NMIICR.byte;			gCIA2NMIICR.byte = 0;			break;		case 0x0E:		//	CIA Control Register A			val = gCIA2CRegA.byte;			break;		case 0x0F:		//	CIA Control Register B			val = gCIA2CRegB.byte;			break;		default:			break;	}		#if	LOCALDEBUG && DEBUG		debug_window_printf("CIA2Read: reg: %X, val: %X",(int)reg,(int)val);	#endif	gCIA2Reg[reg] = val;	return val;}void CIA2TimerAExpire(void){		//	they turned off the timer, so ignore it   // if (!gCIA2CRegA.bits.start)        if (!gCIA2CRegA.byte&0x01)		return;		//	load timer counter with latch value on underflow	gCIA2TimerACount = htonl(gCIA2TimerALatch.val);		//	restart timer counting if in continous run mode   // if (gCIA2CRegA.bits.RUNMode)        if (gCIA2CRegA.byte&0x08)//		gCIA2CRegA.bits.start = 0;    gCIA2CRegA.byte &= ~(1);   	else		SetCIA2TimerAInterruptCounter(gCIA2TimerACount);		//	set interrupt bit and cause interrupt if enabled//	gCIA2NMIICR.bits.timerA = 1;    gCIA2NMIICR.byte = gCIA2NMIICR.byte | 0x01;//    if (gCIA2MaskICR.bits.timerA && !gCIA2NMIICR.bits.NMIFlag)  //  fprintf(stderr,"Checking for NMI\n");        if ((gCIA2MaskICR.byte & kTimerABitFlag) && !(gCIA2NMIICR.byte & kNMIBitFlag))	{   //     fprintf(stderr,"about to call NMIInterrupt\n");        gCIA2NMIICR.byte &= ~(1<<7);//		gCIA2NMIICR.bits.NMIFlag = 1;		NMIInterrupt();	}}extern unsigned long		gPCRegister; short CIA2InterruptPending(void){	//	is there a pending/latched interrupt on CIA1?    return gCIA2NMIICR.byte & kNMIBitFlag;//    return gCIA2NMIICR.bits.NMIFlag;}