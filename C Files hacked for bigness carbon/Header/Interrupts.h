/*-------------------------------------------------------------------------------*\||	File:	Interrupts.h||	Description:||||||	Copyright �	1994, Alan Steremberg and Ed Wynne|\*-------------------------------------------------------------------------------*/#ifndef	_INTERRUPTS_#define	_INTERRUPTS_unsigned long InterruptCheck(void);void CalculateNextInterrupt(void);short IRQInterrupt(void);short NMIInterrupt(void);void CheckForLatchedInterrupts(void);void SetCPUTimingInterruptCounter(long count);void SetRasterLineInterruptCounter(long count);void SetCIA1TimerAInterruptCounter(long count);void SetCIA2TimerAInterruptCounter(long count);#endif	//_INTERRUPTS_