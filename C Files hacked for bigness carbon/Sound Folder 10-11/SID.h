/* SID.h */#ifndef	_SID_#define	_SID_#ifndef	_GLOBALDEFS_#include "GlobalDefs.h"#endif	_GLOBALDEFS_void SIDWrite(unsigned long address,unsigned long value);//#ifndef powerc//unsigned long SIDRead(unsigned long address:__A0);//			int	SoundInitialize 	(void);//			void 	DisposeSounds 		(void);//			void	ToggleSound			(void);//#elseunsigned long SIDRead(unsigned long address);			int	SoundInitialize 	(void);			void 	DisposeSounds 		(void);			void	ToggleSound			(void);//#endif#endif	_SID_