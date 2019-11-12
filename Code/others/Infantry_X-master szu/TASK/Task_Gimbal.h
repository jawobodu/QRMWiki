#ifndef _TASK_GIMBAL_H
#define _TASK_GIMBAL_H

#include "system.h"


#define YAW 0
#define PITCH 1

#define MECH 0
#define GYRO 1

#define NOW  0
#define LAST 1

/*       �ٽ�ֵ����ṹ��        */
typedef struct 
{

	float LastAngle;       //��һ�ζ�ȡ�ĽǶ�
	float CurAngle;	//��ǰ��ȡ�ĽǶ�
	float AngleSum;	//�Ƕ��ۼ�ֵ
	
}Critical_t;

typedef struct  //�Ӿ�Ŀ���ٶȲ���
{
  int delay_cnt;//����������֡Ŀ�겻�����ʱ��,�����ж��ٶ��Ƿ�Ϊ0
  int freq;
  int last_time;//�ϴ��ܵ�Ŀ��Ƕȵ�ʱ��
  float last_position;//�ϸ�Ŀ��Ƕ�
  float speed;//�ٶ�
  float last_speed;//�ϴ��ٶ�
  float processed_speed;//�ٶȼ�����
}speed_calc_data_t;

void GIMBAL_InitArgument(void);
void GIMBAL_kPID_Init(void);
void GIMBAL_StopMotor(void);
void GIMBAL_InitCtrl(void);

/***********��̨�ܿ���,loop�е���************/
void Task_Gimbal(void *pvParameters);
void GIMBAL_Rc_Ctrl(void);
void GIMBAL_Key_Ctrl(void);
void Gimbal_Chass_Separ_Limit(void);

/***********��̨����ģʽ����ģʽС����*************/
void GIMBAL_NORMAL_Mode_Ctrl(void);
void GIMBAL_HIGH_Mode_Ctrl(void);
void GIMBAL_LEVEL_Mode_Ctrl(void);
void GIMBAL_AUTO_Mode_Ctrl(void);
void GIMBAL_BUFF_Mode_Ctrl_Chassis(void);
void GIMBAL_BUFF_Mode_Ctrl_Gimbal(void);
void GIMBAL_BASE_Mode_Ctrl(void);
void GIMBAL_MANUAL_Mode_Ctrl(void);

/************��̨����ֵ���¼�����****************/
void GIMBAL_UpdateAngle( char eAxis, int16_t angle );
void GIMBAL_UpdatePalstance(void);
void GIMBAL_CanbusCtrlMotors(void);

/*****************************��̨λ��PID����***********************************/
void GIMBAL_PositionLoop(void);
void vPitch_Mech_PositionLoop(void);
void vPitch_Gyro_PositionLoop(void);
void vYaw_Mech_PositionLoop(void);
void vYaw_Gyro_PositionLoop(void);


/****************************��������**************************************/
void Critical_Handle_Init(Critical_t *critical, float get);
float Gimbal_Yaw_Gryo_AngleSum(Critical_t *critical, float get);
int16_t GIMBAL_GetOffsetAngle(void);//����YAW����ƫ��
uint8_t GIMBAL_IfPitchLevel(void);//�ж�Pitch�Ƿ�ص�ˮƽλ��
uint8_t GIMBAL_IfPitchHigh(void);//��̨�Ƿ�̧ͷ
bool GIMBAL_IfGIMBAL_LEVEL(void);
//�Ӿ�
bool GIMBAL_IfBuffHit(void);//���
bool GIMBAL_If_Big_Buff(void);//���
bool GIMBAL_If_Small_Buff(void);//С��
bool GIMBAL_If_Base(void);
bool GIMBAL_IfManulHit(void);//�ֶ����
bool GIMBAL_IfAutoHit(void);//����
float Target_Speed_Calc(speed_calc_data_t *S, uint32_t time, float position);
bool Gimb_If_Small_Top(float angle);
bool GIMBAL_IfAuto_MobPre_Yaw(void);
bool GIMBAL_MOBPRE_YAW_FIRE(void);

bool GIMBAL_BUFF_YAW_READY(void);
bool GIMBAL_BUFF_PITCH_READY(void);
bool GIMBAL_AUTO_PITCH_SB(void);
bool GIMBAL_AUTO_PITCH_SB_SK(void);
float GIMBAL_PITCH_Judge_Angle(void);
int Base_Angle_Measure(void);

#endif
