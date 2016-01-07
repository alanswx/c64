/*SIDPriv.h*/

#if 1

/* Control Register Masks */
#define	kGateMask		0x01
#define	kSyncMask		0x02
#define	kRingModMask	0x04
#define	kTestMask		0x08
#define	kTriMask			0x10
#define	kSawMask			0x20
#define	kPulseMask		0x40
#define	kNoiseMask		0x80
#define	kWaveSelectMask	0xF0

/* Pulse Width Mask */
#define	kPulseWidthMask	0x0FFF

/* Envelope Generator Masks */
#define	kAttackMask		0xF0
#define	kDecayMask		0x0F
#define	kSustainMask	0xF0
#define	kReleaseMask	0x0F

/* Filter and Volume Masks */
#define	kResonanceMask	0xF0
#define	kFilterVoice1Mask	0x01
#define	kFilterVoice2Mask	0x02
#define	kFilterVoice3Mask	0x04
#define	kFilterInputVoiceMask	0x08
#define	kFilterMask		0x70
#define	kVolumeMask		0x0F

/* Other Stuff */
#define	Pi				3.141592654

/* Sample specific defines */
#define	kMaxVoices		3
#define	kMaxTimbres		4

#define	kSampleLen		256
#define	kBufferSize		256

#define	kFrequencyConstant	0.059604645
#define	kBaseFrequency			(86.93181797/*/2*/)	//Frequency of Base note in Hz? (divided by 2 for 11khz sampling rate)
#define	kEnvelopeConstant		0.08986928127 /2 //?

#define	kShiftAmount			65536

#define	kFiltFreqConstant		((2*Pi)/22254.5454)
#define	kFilterSteps			5.844726563

#define	kMod256Mask				0x000000FF

/* Type definitions */
typedef	enum	{Attack,Decay,Sustain,Release,Done} EGStage;
typedef	enum	{Lowpass,Hipass,Notch,Bandpass} FilterType;

typedef char vol_array[256];

/* Function prototypes */
//pascal	void	DoubleBackProc 	(SndChannelPtr, SndDoubleBufferPtr); // AJS

/* Globals */
//SndChannelPtr	gVoices;
Ptr				gTimbres[kMaxTimbres];
//SndDoubleBufferHeader		gSndHeaders; // AJS
//SndCommand		AmpCommand,
//					StopCommand;
									
float	susMults [16] = {		6,
									24,
									48,
									72,
									114,
									168,
									204,
									240,
									300,
									750,
									1500,
									2400,
									3000,
									9000,
									15000,
									24000 };

float	attackCPS [16] = {	5.751634001 /* /2 */,
									1.4379085 /* /2 */,
									0.7189542501 /* /2 */ ,
									0.4793028334 ,// /2 ,
									0.302717579 ,// /2 ,
									0.2054155 ,// /2 ,
									0.1691657059 ,// /2 ,
									0.14379085 ,// /2 ,
									0.11503268 ,// /2 ,
									0.04601307201 ,// /2 ,
									0.023006536 ,// /2 ,
									0.014379085 ,// /2 ,
									0.011503268 ,// /2 ,
									0.003834422667 ,// /2 ,
									0.0023006536 ,// /2 ,
									0.0014379085 ,///2
								};
vol_array	*volumes[256];
long			gLastPhase1 = 0,gLastPhase2 = 0,gLastPhase3 = 0;
long			gCurVol1 = 0, gCurVol2 = 0, gCurVol3 = 0;
long			gEnvVal1 = 0, gEnvVal2 = 0, gEnvVal3 = 0;
long			gSusLev1 = 0, gSusLev2 = 0, gSusLev3 = 0;
long			gDecayVal1 = 0, gDecayVal2 = 0, gDecayVal3 = 0;
EGStage		gStage1 = Sustain, gStage2 = Sustain, gStage3 = Sustain;
Boolean		InUse = FALSE;
Boolean		gSoundOn = TRUE;

Boolean		gRingMod1=FALSE,gRingMod2=FALSE,gRingMod3=FALSE;

/*long	gFilter	[16]	=	{	30,
									828,
									1626,
									2424,
									3222,
									4020,
									4818,
									5616,
									6414,
									7212,
									8010,
									8808,
									9606,
									10404,
									11202,
									12000
								};
float	gResonance	[16]	=	{	2,
										1.875,
										1.75,
										1.625,
										1.5,
										1.375,
										1.25,
										1.125,
										1,
										0.875,
										0.75,
										0.625,
										0.5,
										0.375,
										0.25,
										0.125
									};
long	gL,gB,gH,gF1,gD1=0,gD2=0;
float	gQ1;
FilterType	gFilterType;
Boolean		gFilter1;*/


#endif