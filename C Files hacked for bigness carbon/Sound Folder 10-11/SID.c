/* SID.c */
#include <math.h>
#include <Carbon/Carbon.h>
///#include	"Timer.h"
#include	"SIDPriv.h"
#include "DebugWindow.h"
#include "SID.h"

#define TURNSIDOFF 1

/*local globals*/
EGStage	stage1;

short	oldLevel1,newLevel1;
short	oldLevel2,newLevel2;

//no clue about these declarations
short	SndError;
unsigned short	V1Gate;
SCStatus	mySCStatus;

long	RndSeed;

#define	kOutOfMemory		-1
#define	kMissingResource	-1

/*	local constants	*/
#define	kSIDBaseAddress	0xD400
#define	LOCALDEBUG			0

/*	local globals	*/
unsigned char	gSIDReg[32];
Ptr gVoice1 = NULL, gVoice2 = NULL, gVoice3 = NULL;
//SndDoubleBackUPP		gDoubleBackProcUPP=NULL; // AJS



void SIDWrite(unsigned long address,unsigned long value)
{
	register	unsigned long	reg;
	register	unsigned long	val;
	register unsigned long pulseWidth;
	short	onoff;
	
	val = value;
	reg = (address-kSIDBaseAddress)&0x1F;
	
	switch(reg)
	{
		case 0x00:		//	Voice 1: Frequency Control -- Low-Byte
			break;
		case 0x01:		//	Voice 1: Frequency Control -- High-Byte
			break;
		case 0x02:		//	Voice 1: Pulse Waveform Width -- Low-Byte
			if ((gVoice1 >= gTimbres[2]) && (gVoice1 <= gTimbres[2] + kSampleLen))
			{
				pulseWidth = ((unsigned long)val + ((unsigned long)gSIDReg[0x03] * 256)) >> 4;
				gVoice1 = gTimbres[2] + pulseWidth;
			}
			break;
		case 0x03:		//	Voice 1: Pulse Waveform Width -- High-Nybble
			if ((gVoice1 >= gTimbres[2]) && (gVoice1 <= gTimbres[2] + kSampleLen))
			{
				pulseWidth = ((unsigned long)gSIDReg[0x02] + ((unsigned long)val * 256)) >> 4;
				gVoice1 = gTimbres[2] + pulseWidth;
			}
			break;
		case 0x04:		//	Voice 1: Control Register
			switch (val & kWaveSelectMask)
			{
				case 0x10 :
					gVoice1 = gTimbres[0];
					break;
				case 0x20 :
					gVoice1 = gTimbres[1];
					break;
				case 0x40 :
					//figure out pulse width here.
					pulseWidth = ((unsigned short)gSIDReg[0x02] + ((unsigned short)gSIDReg[0x03] * 256)) >> 4;
					gVoice1 = gTimbres[2] + pulseWidth;
					break;
				case 0x80 :
					gVoice1 = gTimbres[3];
					break;
				default :
					/* They're trying to do something tricky, so I ignore it (assume Triangle wave)*/
					gVoice1 = gTimbres[0];
					break;
			}
			
			if (((onoff = (gSIDReg[0x04] & kGateMask)) ^ val))
			{
				if (!onoff)
				{
					//attack accumulation per sample
					InUse = TRUE;
					gStage1 = Attack;
					gEnvVal1 = (attackCPS[(gSIDReg[0x05] & kAttackMask) >> 4] * kShiftAmount);
					InUse = FALSE;
				}
				else
				{
					//release accumulation per sample
					InUse = TRUE;
					gStage1 = Release;
					gEnvVal1 = kEnvelopeConstant * ((float)(gCurVol1>>16)/susMults[gSIDReg[0x06] & kReleaseMask]) * -1 * kShiftAmount;
					InUse = FALSE;
				}
			}
			
			if ((val & kRingModMask) && (gVoice1 == gTimbres[0]))
			{
				gRingMod1 = TRUE;
				debug_window_printf ("Ring Mod voice 1 on.");
			}
			else
			{
				gRingMod1 = FALSE;
			}
			
			break;
		case 0x05:		//	Envelope Generator 1: Attack/Decay Cycle Control
			gDecayVal1 = kEnvelopeConstant * (((float)(255-gSusLev1))/susMults[val & kDecayMask]) * -1 * kShiftAmount;
			break;
		case 0x06:		//	Envelope Generator 1: Sustain/Release Cycle Control
			gSusLev1 = ((val & kSustainMask) >> 4) * 17;
			break;
		case 0x07:		//	Voice 2: Frequency Control -- Low-Byte
			break;
		case 0x08:		//	Voice 2: Frequency Control -- High-Byte
			break;
		case 0x09:		//	Voice 2: Pulse Waveform Width -- Low-Byte
			if ((gVoice2 >= gTimbres[2]) && (gVoice2 <= gTimbres[2] + kSampleLen))
			{
				pulseWidth = ((unsigned long)val + ((unsigned long)gSIDReg[0x0A] * 256)) >> 4;
				gVoice2 = gTimbres[2] + pulseWidth;
			}
			break;
		case 0x0A:		//	Voice 2: Pulse Waveform Width -- High-Nybble
			if ((gVoice2 >= gTimbres[2]) && (gVoice2 <= gTimbres[2] + kSampleLen))
			{
				pulseWidth = ((unsigned long)gSIDReg[0x09] + ((unsigned long)val * 256)) >> 4;
				gVoice2 = gTimbres[2] + pulseWidth;
			}
			break;
		case 0x0B:		//	Voice 2: Control Register
			switch (val & kWaveSelectMask)
			{
				case 0x10 :
					gVoice2 = gTimbres[0];
					break;
				case 0x20 :
					gVoice2 = gTimbres[1];
					break;
				case 0x40 :
					//figure out pulse width here.
					pulseWidth = ((unsigned short)gSIDReg[0x09] + ((unsigned short)gSIDReg[0x0A] * 256)) >> 4;
					gVoice2 = gTimbres[2] + pulseWidth;
					break;
				case 0x80 :
					gVoice2 = gTimbres[3];
					break;
				default :
					/* They're trying to do something tricky, so I ignore it (assume Triangle wave)*/
					gVoice2 = gTimbres[0];
					break;
			}
			
			if (((onoff = (gSIDReg[0x0B] & kGateMask)) ^ val))
			{
				if (!onoff)
				{
					//attack accumulation per sample
					InUse = TRUE;
					gStage2 = Attack;
					gEnvVal2 = (attackCPS[(gSIDReg[0x0C] & kAttackMask) >> 4] * kShiftAmount);
					InUse = FALSE;
				}
				else
				{
					//release accumulation per sample
					InUse = TRUE;
					gStage2 = Release;
					gEnvVal2 = kEnvelopeConstant * ((float)(gCurVol2>>16)/susMults[gSIDReg[0x0D] & kReleaseMask]) * -1 * kShiftAmount;
					InUse = FALSE;
				}
			}
			
			if ((val & kRingModMask) && (gVoice2 == gTimbres[0]))
			{
				gRingMod2 = TRUE;
				debug_window_printf ("Ring Mod voice 2 on.");
			}
			else
			{
				gRingMod2 = FALSE;
			}

			break;
		case 0x0C:		//	Envelope Generator 2: Attack/Decay Cycle Control
			gDecayVal2 = kEnvelopeConstant * (((float)(255-gSusLev2))/susMults[val & kDecayMask]) * -1 * kShiftAmount;
			break;
		case 0x0D:		//	Envelope Generator 2: Sustain/Release Cycle Control
			break;
		case 0x0E:		//	Voice 3: Frequency Control -- Low-Byte
			break;
		case 0x0F:		//	Voice 3: Frequency Control -- High-Byte
			break;
		case 0x10:		//	Voice 3: Pulse Waveform Width -- Low-Byte
			if ((gVoice3 >= gTimbres[2]) && (gVoice3 <= gTimbres[2] + kSampleLen))
			{
				pulseWidth = ((unsigned long)val + ((unsigned long)gSIDReg[0x11] * 256)) >> 4;
				gVoice3 = gTimbres[2] + pulseWidth;
			}
			break;
		case 0x11:		//	Voice 3: Pulse Waveform Width -- High-Nybble
			if ((gVoice3 >= gTimbres[2]) && (gVoice3 <= gTimbres[2] + kSampleLen))
			{
				pulseWidth = ((unsigned long)gSIDReg[0x10] + ((unsigned long)val * 256)) >> 4;
				gVoice3 = gTimbres[2] + pulseWidth;
			}
			break;
		case 0x12:		//	Voice 3: Control Register
			switch (val & kWaveSelectMask)
			{
				case 0x10 :
					gVoice3 = gTimbres[0];
					break;
				case 0x20 :
					gVoice3 = gTimbres[1];
					break;
				case 0x40 :
					//figure out pulse width here.
					pulseWidth = ((unsigned short)gSIDReg[0x10] + ((unsigned short)gSIDReg[0x11] * 256)) >> 4;
					gVoice3 = gTimbres[2] + pulseWidth;
					break;
				case 0x80 :
					gVoice3 = gTimbres[3];
					break;
				default :
					/* They're trying to do something tricky, so I ignore it (assume Triangle wave)*/
					gVoice3 = gTimbres[0];
					break;
			}
			
			if (((onoff = (gSIDReg[0x12] & kGateMask)) ^ val))
			{
				if (!onoff)
				{
					//attack accumulation per sample
					InUse = TRUE;
					gStage3 = Attack;
					gEnvVal3 = (attackCPS[(gSIDReg[0x13] & kAttackMask) >> 4] * kShiftAmount);
					InUse = FALSE;
				}
				else
				{
					//release accumulation per sample
					InUse = TRUE;
					gStage3 = Release;
					gEnvVal3 = kEnvelopeConstant * ((float)(gCurVol3>>16)/susMults[gSIDReg[0x14] & kReleaseMask]) * -1 * kShiftAmount;
					InUse = FALSE;
				}
			}
			
			if ((val & kRingModMask) && (gVoice3 == gTimbres[0]))
			{
				gRingMod3 = TRUE;
				debug_window_printf ("Ring Mod voice 3 on.");
			}
			else
			{
				gRingMod3 = FALSE;
			}

			break;
		case 0x13:		//	Envelope Generator 3: Attack/Decay Cycle Control
			gDecayVal3 = kEnvelopeConstant * (((float)(255-gSusLev3))/susMults[val & kDecayMask]) * -1 * kShiftAmount;
			break;
		case 0x14:		//	Envelope Generator 3: Sustain/Release Cycle Control
			gSusLev3 = ((val & kSustainMask) >> 4) * 17;
			break;
		case 0x15:		//	Filter Cutoff Frequency: Low-Nybble (Bits 2-0)
			break;
		case 0x16:		//	Filter CutOff Frequency: High-Byte
			break;
		case 0x17:		//	Filter Resonance Control/Voice Input Control
			break;
		case 0x18:		//	Select Filter Mode and Volume
			AmpCommand.param1 = (val & kVolumeMask) * 17;
			SndDoImmediate (gVoices,&AmpCommand);
			break;
		case 0x19:		//	Analog/Digital Converter: Game Paddle 1 (0-255)
			/* These last 4 should not set the values for writes. */
			break;
		case 0x1A:		//	Analog/Digital Converter: Game Paddle 2 (0-255)
			break;
		case 0x1B:		//	Oscillator 3 Random Number Generator
			break;
		case 0x1C:		//	Envelope Generator 3 Output
			break;
		default:
			break;
	}
	
	#if	LOCALDEBUG && DEBUG
		debug_window_printf("SIDWrite: reg: %X, val: %X",(int)reg,(int)val);
	#endif
	if (reg < 0x19)
		gSIDReg[reg] = val;
}





//#ifndef	powerc
//unsigned long SIDRead(unsigned long address:__A0)
//#else
unsigned long SIDRead(unsigned long address)
//#endif
{
	register	unsigned long	reg;
	register	unsigned long	val;
	Point P;
	
	
	reg = (address-kSIDBaseAddress)&0x0F;
	val = gSIDReg[reg];
	
	switch(reg)
	{
		//Maybe make this an if reg < 0x19?
		case 0x00:		//	Voice 1: Frequency Control -- Low-Byte
		case 0x01:		//	Voice 1: Frequency Control -- High-Byte
		case 0x02:		//	Voice 1: Pulse Waveform Width -- Low-Byte
		case 0x03:		//	Voice 1: Pulse Waveform Width -- High-Nybble
		case 0x04:		//	Voice 1: Control Register
		case 0x05:		//	Envelope Generator 1: Attack/Decay Cycle Control
		case 0x06:		//	Envelope Generator 1: Sustain/Release Cycle Control
		case 0x07:		//	Voice 2: Frequency Control -- Low-Byte
		case 0x08:		//	Voice 2: Frequency Control -- High-Byte
		case 0x09:		//	Voice 2: Pulse Waveform Width -- Low-Byte
		case 0x0A:		//	Voice 2: Pulse Waveform Width -- High-Nybble
		case 0x0B:		//	Voice 2: Control Register
		case 0x0C:		//	Envelope Generator 2: Attack/Decay Cycle Control
		case 0x0D:		//	Envelope Generator 2: Sustain/Release Cycle Control
		case 0x0E:		//	Voice 3: Frequency Control -- Low-Byte
		case 0x0F:		//	Voice 3: Frequency Control -- High-Byte
		case 0x10:		//	Voice 3: Pulse Waveform Width -- Low-Byte
		case 0x11:		//	Voice 3: Pulse Waveform Width -- High-Nybble
		case 0x12:		//	Voice 3: Control Register
		case 0x13:		//	Envelope Generator 3: Attack/Decay Cycle Control
		case 0x14:		//	Envelope Generator 3: Sustain/Release Cycle Control
		case 0x15:		//	Filter Cutoff Frequency: Low-Nybble (Bits 2-0)
		case 0x16:		//	Filter CutOff Frequency: High-Byte
		case 0x17:		//	Filter Resonance Control/Voice Input Control
		case 0x18:		//	Select Filter Mode and Volume
			return 0;
			break;
		case 0x19:		//	Analog/Digital Converter: Game Paddle 1 (0-255)
		{
				   Rect bounds;
			BitMap bmap;

			/* Could maybe get mouse position for this and the next one. */
			GetMouse (&P);
			GetQDGlobalsScreenBits(&bmap);
			bounds = bmap.bounds;

			val = (P.h % bounds.right) * 255;
		}
				break;
		case 0x1A:		//	Analog/Digital Converter: Game Paddle 2 (0-255)
		{
		   Rect bounds;
			BitMap bmap;
			GetQDGlobalsScreenBits(&bmap);
			bounds = bmap.bounds;
			GetMouse (&P);
			val = (P.v % bounds.bottom) * 255;
			}
			break;
		case 0x1B:		//	Oscillator 3 Random Number Generator
			break;
		case 0x1C:		//	Envelope Generator 3 Output
			break;
		default:
			break;
	}
	
//	#if	LOCALDEBUG && DEBUG
		debug_window_printf("SIDRead: reg: %X, val: %X",(int)reg,(int)val);
//	#endif
	gSIDReg[reg] = val;
	return val;
}

#if (TURNSIDOFF==0)

//Calculate the sounds
pascal void DoubleBackProc (SndChannelPtr chan, SndDoubleBufferPtr doubleBuffer)
{
	register	unsigned char	*base1,*base2,*base3;
	register	Ptr	dst;
	
	register long	index1, index2,index3;
	register	unsigned	long	frequency1,frequency2,frequency3;
	short	scale1[256],scale2[256],scale3[256];
	register short *scale1Ptr,*scale2Ptr,*scale3Ptr;
	register short *endscale1,*endscale2,*endscale3;
	register	long	i1,i2,i3;
	long	OldA5;
	
	OldA5 = SetA5(doubleBuffer->dbUserInfo[0]);

	if (!InUse)
	{
		//Get the frequency of voices
		frequency1 = (unsigned long)
		(((float)(((float)((unsigned short)gSIDReg[0x00] | ((unsigned short)gSIDReg[0x01] << 8)) * kFrequencyConstant)/kBaseFrequency))
		* kShiftAmount);
		frequency2 = (unsigned long)
		(((float)(((float)((unsigned short)gSIDReg[0x07] | ((unsigned short)gSIDReg[0x08] << 8)) * kFrequencyConstant)/kBaseFrequency))
		* kShiftAmount);
		frequency3 = (unsigned long)
		(((float)(((float)((unsigned short)gSIDReg[0x0E] | ((unsigned short)gSIDReg[0x0F] << 8)) * kFrequencyConstant)/kBaseFrequency))
		* kShiftAmount);

		
		//Get the waveform of voices
		base1 = (unsigned char*)gVoice1;
		base2 = (unsigned char*)gVoice2;
		base3 = (unsigned char*)gVoice3;
		
		scale1Ptr = &scale1[0];
		scale2Ptr = &scale2[0];
		scale3Ptr = &scale3[0];
		
		endscale1 = scale1+256;
		endscale2 = scale2+256;
		endscale3 = scale3+256;
		
		//Voice 1!
		if (gStage1 == Attack)
		{
			while ((scale1Ptr < endscale1) && (gCurVol1 < /*256 << 16*/256 * kShiftAmount))
			{
				*scale1Ptr++ = gCurVol1 >> 16;
				gCurVol1 += gEnvVal1;
			}
			
			if (gCurVol1 >= 255 * kShiftAmount)
			{
				gCurVol1 = 255 << 16;
				gEnvVal1 = gDecayVal1;
				gStage1 = Decay;
			}
		}
		
		if (gStage1 == Decay)
		{
			while ((scale1Ptr < endscale1) && ((gCurVol1 >> 16) > gSusLev1))
			{
				*scale1Ptr++ = gCurVol1 >> 16;
				gCurVol1 += gEnvVal1;
			}
			
			if ((gCurVol1 >> 16) < gSusLev1)
			{
				gCurVol1 = gSusLev1 << 16;
				gEnvVal1 = 0;
				gStage1 = Sustain;
			}
		}
		
		while (scale1Ptr < endscale1)
		{
			gCurVol1 += gEnvVal1;
			*scale1Ptr++ = gCurVol1 >> 16;
			if (gCurVol1 < 0)
			{
				gCurVol1 = 0;
				scale1Ptr--;
				*scale1Ptr++ = 0;
				gEnvVal1 = 0;
			}
		}
		
		//Voice 2!
		if (gStage2 == Attack)
		{
			while ((scale2Ptr < endscale2) && (gCurVol2 < 256 * kShiftAmount/*<< 16*/))
			{
				*scale2Ptr++ = gCurVol2 >> 16;
				gCurVol2 += gEnvVal2;
			}
			
			if (gCurVol2 >= 255 * kShiftAmount)
			{
				gCurVol2 = 255 << 16;
				gEnvVal2 = gDecayVal2;
				gStage2 = Decay;
			}
		}
		
		if (gStage2 == Decay)
		{
			while ((scale2Ptr < endscale2) && ((gCurVol2 >> 16) > gSusLev2))
			{
				*scale2Ptr++ = gCurVol2 >> 16;
				gCurVol2 += gEnvVal2;
			}
			
			if ((gCurVol2 >> 16) < gSusLev2)
			{
				gCurVol2 = gSusLev2 << 16;
				gEnvVal2 = 0;
				gStage2 = Sustain;
			}
		}
		
		while (scale2Ptr < endscale2)
		{
			gCurVol2 += gEnvVal2;
			*scale2Ptr++ = gCurVol2 >> 16;
			if (gCurVol2 < 0)
			{
				gCurVol2 = 0;
				scale2Ptr--;
				*scale2Ptr++ = 0;
				gEnvVal2 = 0;
			}
		}
		
		//Voice 3!
		if (gStage3 == Attack)
		{
			while ((scale3Ptr < endscale3) && (gCurVol3 < 256 * kShiftAmount /*<< 16*/))
			{
				*scale3Ptr++ = gCurVol3 >> 16;
				gCurVol3 += gEnvVal3;
			}
			
			if (gCurVol3 >= 255 * kShiftAmount)
			{
				gCurVol3 = 255 << 16;
				gEnvVal3 = gDecayVal3;
				gStage3 = Decay;
			}
		}
		
		if (gStage3 == Decay)
		{
			while ((scale3Ptr < endscale3) && ((gCurVol3 >> 16) > gSusLev3))
			{
				*scale3Ptr++ = gCurVol3 >> 16;
				gCurVol3 += gEnvVal3;
			}
			
			if ((gCurVol3 >> 16) < gSusLev3)
			{
				gCurVol3 = gSusLev3 << 16;
				gEnvVal3 = 0;
				gStage3 = Sustain;
			}
		}
		
		while (scale3Ptr < endscale3)
		{
			gCurVol3 += gEnvVal3;
			*scale3Ptr++ = gCurVol3 >> 16;
			if (gCurVol3 < 0)
			{
				gCurVol3 = 0;
				scale3Ptr--;
				*scale3Ptr++ = 0;
				gEnvVal3 = 0;
			}
		}
		
		// For reading later.
		gSIDReg[0x1C] = (unsigned char)(gCurVol3 >> 16);

		index1 = gLastPhase1 << 16;
		index2 = gLastPhase2 << 16;
		index3 = gLastPhase3 << 16;
		dst = &doubleBuffer->dbSoundData[0];
		
		scale1Ptr = &scale1[0];
		scale2Ptr = &scale2[0];
		scale3Ptr = &scale3[0];
		
		//000
		if ((!gRingMod1) && (!gRingMod2) && (!gRingMod3))
		{
			while (scale1Ptr < endscale1)
			{
				i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]));
				i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]));
				i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]));
				
				*dst++ = (i1+i2+i3) + 0x80;
				
				index1 += frequency1;
				index2 += frequency2;
				index3 += frequency3;
			}
		}
		//001
		else if ((!gRingMod1) && (!gRingMod2) && (gRingMod3))
		{
			while (scale1Ptr < endscale1)
			{
					i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]));
					i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]));
					i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base2[(index2 >> 16) & kMod256Mask]]))>>8;
				
				*dst++ = (i1+i2+i3) + 0x80;
				
				index1 += frequency1;
				index2 += frequency2;
				index3 += frequency3;
			}
		}
		//010
		else if ((!gRingMod1) && (gRingMod2) && (!gRingMod3))
		{
			while (scale1Ptr < endscale1)
			{
				i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]));
				i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base1[(index1 >> 16) & kMod256Mask]]))>>8;
				i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]));
				
				*dst++ = (i1+i2+i3) + 0x80;
				
				index1 += frequency1;
				index2 += frequency2;
				index3 += frequency3;
			}
		}
		//011
		else if ((!gRingMod1) && (gRingMod2) && (gRingMod3))
		{
			while (scale1Ptr < endscale1)
			{
				i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]));
				i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base1[(index1 >> 16) & kMod256Mask]]))>>8;
				i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base2[(index2 >> 16) & kMod256Mask]]))>>8;
							
				*dst++ = (i1+i2+i3) + 0x80;
				
				index1 += frequency1;
				index2 += frequency2;
				index3 += frequency3;
			}
		}
		//100
		else if ((gRingMod1) && (!gRingMod2) && (!gRingMod3))
		{
			while (scale1Ptr < endscale1)
			{
				i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base3[(index3 >> 16) & kMod256Mask]]))>>8;
				i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]));
				i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]));
				
				*dst++ = (i1+i2+i3) + 0x80;
				
				index1 += frequency1;
				index2 += frequency2;
				index3 += frequency3;
			}
		}
		//101
		else if ((gRingMod1) && (!gRingMod2) && (gRingMod3))
		{
			while (scale1Ptr < endscale1)
			{
				i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base3[(index3 >> 16) & kMod256Mask]]))>>8;
				i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]));
				i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base2[(index2 >> 16) & kMod256Mask]]))>>8;
								
				*dst++ = (i1+i2+i3) + 0x80;
				
				index1 += frequency1;
				index2 += frequency2;
				index3 += frequency3;
			}
		}
		//110
		else if ((gRingMod1) && (gRingMod2) && (!gRingMod3))
		{
			while (scale1Ptr < endscale1)
			{
				i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base3[(index3 >> 16) & kMod256Mask]]))>>8;
				i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base1[(index1 >> 16) & kMod256Mask]]))>>8;
				i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]));
				
				*dst++ = (i1+i2+i3) + 0x80;
				
				index1 += frequency1;
				index2 += frequency2;
				index3 += frequency3;
			}
		}
		//111
		else if ((gRingMod1) && (gRingMod2) && (gRingMod3))
		{
			while (scale1Ptr < endscale1)
			{
				i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base3[(index3 >> 16) & kMod256Mask]]))>>8;
				i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base1[(index1 >> 16) & kMod256Mask]]))>>8;
				i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]) 
						* ((long)(*volumes[255])[base2[(index2 >> 16) & kMod256Mask]]))>>8;
								
				*dst++ = (i1+i2+i3) + 0x80;
				
				index1 += frequency1;
				index2 += frequency2;
				index3 += frequency3;
			}
		}


/*		while (scale1Ptr < endscale1)
		{
			if (gRingMod1)
				i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]) 
					* ((long)(*volumes[255])[base3[(index3 >> 16) & kMod256Mask]]))>>8;
			else
				i1 = ((long)((*volumes[*scale1Ptr++])[base1[(index1 >> 16) & kMod256Mask]]));
			
			if (gRingMod2)
				i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]) 
					* ((long)(*volumes[255])[base1[(index1 >> 16) & kMod256Mask]]))>>8;
			else
				i2 = ((long)((*volumes[*scale2Ptr++])[base2[(index2 >> 16) & kMod256Mask]]));
			
			if (gRingMod3)
				i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]) 
					* ((long)(*volumes[255])[base2[(index2 >> 16) & kMod256Mask]]))>>8;
			else
				i3 = ((long)((*volumes[*scale3Ptr++])[base3[(index3 >> 16) & kMod256Mask]]));
			
			*dst++ = (i1+i2+i3) + 0x80;
			
			index1 += frequency1;
			index2 += frequency2;
			index3 += frequency3;
		}
*/
		gLastPhase1 = (long)(index1 >> 16) % 256;
		gLastPhase2 = (long)(index2 >> 16) % 256;
		gLastPhase3 = (long)(index3 >> 16) % 256;
		
		doubleBuffer->dbNumFrames = (long)kBufferSize;
	
		/* Always ready because we stop it with a quiet command */
		doubleBuffer->dbFlags |= dbBufferReady;

		/* This sets the Read-only values of the SID*/
		gSIDReg[0x1B] = (unsigned char)((*volumes[255])[base3[(index3 >> 16) % 256]] + 0x80); // need the + 0x80?

	}
	else
	{
		doubleBuffer->dbNumFrames = 0L;
		doubleBuffer->dbFlags |= dbBufferReady;
	}
	OldA5 = SetA5(OldA5);
}
#endif

//Setup Sounds
int SoundInitialize (void)
{
#if (TURNSIDOFF==0)

	OSErr 	sndErr;
	float	delta,angle;
	short		i,t,j;
	short	volume,value;
	Ptr		g0,g1,g2,g3;

	for (i=0;i<256;++i)
	{
		volumes [i] = (vol_array*)NewPtr (256);
		for (j=0;j<256;++j)
		{
			(*volumes[i])[j] = (char)(((float)i/255.0 * (j-127)))/3;
		}
	}
	
	// Allocate Timbres
	if (!(gTimbres[0] = NewPtr (kBufferSize)))
		return (kOutOfMemory);

	if (!(gTimbres[1] = NewPtr (kBufferSize)))
		return (kOutOfMemory);

	if (!(gTimbres[2] = NewPtrClear (kBufferSize*2)))
		return (kOutOfMemory);

	if (!(gTimbres[3] = NewPtr (kBufferSize)))
		return (kOutOfMemory);
	
	//Fill the space we allocated with sound samples	
	GetDateTime ((unsigned long*)&RndSeed);
	
	for (i=0,g0=gTimbres[0],g1=gTimbres[1],g2=gTimbres[2],g3=gTimbres[3];
			i<kSampleLen;
				++i,++g0,++g1,++g2,++g3)
	{
		//SawTooth
		if (i < kSampleLen)
			/*(gTimbres[1])[i]*/*g1 = (unsigned char)(255 - i);
				
		//Triangle
		if (i < kSampleLen/4)
		{
			(gTimbres[0])[i] = 0x80 + (i*2);
#if	0
//THIS CAUSES A BAD CRASH
			(gTimbres[0])[kSampleLen - i] = 0x80 - (i*2);
#endif
		}
		else if (i < kSampleLen - kSampleLen/4)
		{
			(gTimbres[0])[i] = 255 - ((i-(kSampleLen/4))*2);
		}
		
		//Pulse
		/*(gTimbres[2])[i]*/*g2 = 255;
		
		//Noise, kind of. We'll work on this.
		/*(gTimbres[3])[i]*/*g3 = Random() % 256;
	}
	
	/* Setup Channel(s) */
	sndErr = SndNewChannel (&gVoices,sampledSynth/*+initNoInterp*/,initMono,nil);
	
	if (sndErr == resProblem)
		return (kMissingResource);
	else if (sndErr == badChannel)
		return (kOutOfMemory);			/* This should return a more specific error */

	gSndHeaders.dbhNumChannels = 1;
	gSndHeaders.dbhSampleSize = 8;
	gSndHeaders.dbhCompressionID = 0;
	gSndHeaders.dbhPacketSize = 0;
	gSndHeaders.dbhSampleRate = /*rate11khz*/rate22khz;
	if (!(gSndHeaders.dbhBufferPtr[0] = (SndDoubleBufferPtr) NewPtr (kBufferSize + sizeof (SndDoubleBuffer))) )
		return (kOutOfMemory);
	if (!(gSndHeaders.dbhBufferPtr[1] = (SndDoubleBufferPtr) NewPtr (kBufferSize + sizeof (SndDoubleBuffer))) )
		return (kOutOfMemory);

	gDoubleBackProcUPP=NewSndDoubleBackProc(DoubleBackProc);	
	if (gDoubleBackProcUPP==NULL)
		return (kOutOfMemory);
	
	gSndHeaders.dbhDoubleBack = gDoubleBackProcUPP;
	
	gSndHeaders.dbhBufferPtr[0]->dbNumFrames = 0;
	gSndHeaders.dbhBufferPtr[0]->dbFlags = 0;
	gSndHeaders.dbhBufferPtr[0]->dbUserInfo[0] = SetCurrentA5();
	gSndHeaders.dbhBufferPtr[0]->dbUserInfo[1] = 0L;
	
	gSndHeaders.dbhBufferPtr[1]->dbNumFrames = 0;
	gSndHeaders.dbhBufferPtr[1]->dbFlags = 0;
	gSndHeaders.dbhBufferPtr[1]->dbUserInfo[0] = SetCurrentA5();
	gSndHeaders.dbhBufferPtr[1]->dbUserInfo[1] = 0L;
						
	AmpCommand.cmd = ampCmd;
	AmpCommand.param1 = 0;
	AmpCommand.param2 = 0;
	
	StopCommand.cmd = quietCmd;
	StopCommand.param1 = 0;
	StopCommand.param2 = 0;
	
	gVoice1 = gTimbres[0];
	DoubleBackProc (gVoices, gSndHeaders.dbhBufferPtr[0]);
	DoubleBackProc (gVoices, gSndHeaders.dbhBufferPtr[1]);
	SndError = SndPlayDoubleBuffer (gVoices,&gSndHeaders);
#endif	
	return (noErr);
}

//Get rid of memory used by sounds.
void DisposeSounds (void)
{
#if (TURNSIDOFF==0)

	SndDoImmediate (gVoices,&StopCommand);
	SndDisposeChannel (gVoices,TRUE);
	DisposeRoutineDescriptor(gDoubleBackProcUPP);
#endif
}

//This toggles the sound on or off depending on the current condition.
void ToggleSound (void)
{
#if (TURNSIDOFF==0)

	if (gSoundOn)
	{
		SndError = SndDoImmediate (gVoices,&StopCommand);
	}
	else
	{
		DoubleBackProc (gVoices,gSndHeaders.dbhBufferPtr[0]);
		DoubleBackProc (gVoices,gSndHeaders.dbhBufferPtr[1]);
		SndError = SndPlayDoubleBuffer (gVoices,&gSndHeaders);
	}
	
	gSoundOn = !gSoundOn;
#endif
}