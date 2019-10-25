#ifndef _FRICTION_H
#define _FRICTION_H

#include "system.h"

void FRICTION_StopMotor(void);

/*********Ħ��������************/
void FRICTION_Ctrl( void );
bool FRIC_RcSwitch( void );
void FRIC_KeyLevel_Ctrl(void);

/***Ħ����������̨̧ͷ�жϺ���***/
uint8_t FRIC_IfWait( void );
uint8_t FRIC_IfOpen( void );

/****Ħ���ָ�������*****/
void Friction_Ramp(void);
uint16_t Fric_GetHeatInc(void);
float Fric_GetSpeedReal(void);

#endif

