/*-------------------------------------------------------------------------------*\||	File:	CommandLineArgsTable.h||	Description:||||||	Copyright �	1994, Alan Steremberg and Ed Wynne|\*-------------------------------------------------------------------------------*/typedef unsigned long (*CLProc)(char	*commandLine);#define	kMaxCLNameSize	60typedef 	char		CLString[kMaxCLNameSize];typedef	struct	CLLookupEntry		{	CLString			name;	CLProc			function;	}	CLLookupEntry;#if 0CLLookupEntry	CLLokkupTable[] =	{{"BEEP",		CL_Beep},{"G",			CL_G},{"IL",		CL_IL},{"S",			CL_Step},{"GOTO",		CL_Goto},{"DM",		CL_DM},{"DB",		CL_DB},{"DW",		CL_DW},{"SB",		CL_SB},{"SW",		CL_SW},{"HELP",		CL_Help},{"BR",		CL_BR},{"DV",		CL_Version},{"SX",		CL_SX},{"SC",		CL_SC}};#endif