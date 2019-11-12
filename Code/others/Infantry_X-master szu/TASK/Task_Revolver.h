#ifndef _TASK_REVOLVER_H
#define _TASK_REVOLVER_H

#include "system.h"


void REVOLVER_StopMotor(void);
void REVOLVER_InitArgument(void);
void REVOLVER_Rest(void);
void Revolver_Angle_Rest(void);

/*********�����ܿ���*************/
void Task_Revolver(void *pvParameters);

void REVOLVER_Rc_Ctrl(void);
bool REVOLVER_Rc_Switch(void);
/*******����ģʽ************/
void REVOLVER_Key_Ctrl(void);

/******���̼���ģʽ����ģʽС����*******/
void SHOOT_NORMAL_Ctrl(void);
void SHOOT_SINGLE_Ctrl(void);
void SHOOT_TRIPLE_Ctrl(void);
void SHOOT_HIGHTF_LOWS_Ctrl(void);
void SHOOT_MIDF_HIGHTS_Ctrl(void);
void SHOOT_AUTO_Ctrl(void);
void SHOOT_BUFF_Ctrl(void);
void SHOOT_BUFF_Ctrl_Gimbal(void);

void REVOLVER_KeySpeedCtrl(void);
void REVOLVER_KeyPosiCtrl(void);

void REVOLVER_CANbusCtrlMotor(void);

/****���̵�����ݸ���,CAN2�ж��е���****/
void REVOLVER_UpdateMotorAngle( int16_t angle );
void REVOLVER_UpdateMotorSpeed( int16_t speed );
void REVOL_UpdateMotorAngleSum( void );

/*****PID����*******/
void REVOL_SpeedLoop( void );
void REVOL_PositionLoop( void );

/***��Ƶ��������***/
bool Revolver_Heat_Limit(void);

/****��������*****/
void REVOL_SpeedStuck(void);
void REVOL_PositStuck(void);

/******���̸�������******/
void Revol_Angle_Clear(void);
portTickType REVOL_uiGetRevolTime(void);

#endif
