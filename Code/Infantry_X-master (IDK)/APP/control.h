#ifndef _CONTROL_H
#define _CONTROL_H

#include "system.h"

typedef enum
{
    RC   = 0,  
    KEY  = 1,  

} eRemoteMode;  // ң�ط�ʽ


typedef enum
{
	  SYSTEM_STARTING  = 0,
	  SYSTEM_RUNNING   = 1,

} eSystemState;




//����
void SYSTEM_Reset( void );
void SYSTEM_OutCtrlProtect( void );
void SYSTEM_UpdateSystemState( void );
void SYSTEM_UpdateRemoteMode( void );
eRemoteMode SYSTEM_GetRemoteMode( void );
eSystemState SYSTEM_GetSystemState( void );



#endif


