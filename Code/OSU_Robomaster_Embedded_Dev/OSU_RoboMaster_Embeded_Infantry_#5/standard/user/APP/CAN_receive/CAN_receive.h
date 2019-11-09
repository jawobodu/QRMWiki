/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       can_receive.c/h
  * @brief      can device transmit and recevice function��receive via CAN interrupt
  * @note       This is NOT a freeRTOS TASK
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              Complete
  *  V1.0.1     Feb-17-2019     Tony-OSU        Add tx2 can bus config
	*  V1.1.0     Feb-21-2019     Tony-OSU        Finish Custom CAN Bus, fully functional
	*  V1.2.0     Mar-01-2019     Tony-OSU        Package ID modified 
	*																							@note TX2 package is now 0x111 for higher priority
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2016 DJI****************************
  **************Modifid by Ohio State University Robomaster Team****************

  */

#ifndef CANTASK_H
#define CANTASK_H
#include "main.h"

#define CHASSIS_CAN CAN2
#define GIMBAL_CAN CAN1
#define TX2_CAN CAN2
//#define PID_TUNING_CAN CAN2


/* Enumerate CAN send and receive ID */
/* ö������CAN�շ�ID*/
typedef enum
{
	  CAN_AIM_DATA_ID = 0x300,//��������ID
	
    CAN_CHASSIS_ALL_ID = 0x200,
    CAN_3508_M1_ID = 0x201,
    CAN_3508_M2_ID = 0x202,
    CAN_3508_M3_ID = 0x203,
    CAN_3508_M4_ID = 0x204,

    CAN_YAW_MOTOR_ID = 0x205,
    CAN_PIT_MOTOR_ID = 0x206,
    CAN_TRIGGER_MOTOR_ID = 0x207,
    CAN_GIMBAL_ALL_ID = 0x1FF,
		
	
		GYRO_DATA_TX2_ID=0x215,//�����Ǿ��ԽǶ�����ID
	  CAN_GIMBAL_YAW_INTER_TRANSFER_ID=0x210, //Transfer Gimbal data to CAN2
	  CAN_GIMBAL_PITCH_INTER_TRANSFER_ID=0x211,
		CAN_TRIGGER_INTER_TRANSFER_ID=0x212//������ID
} can_msg_id_e;

//RM electrical motor unified data struct
//RM���ͳһ���ݽṹ��
typedef struct
{
    uint16_t ecd;//encoder
    int16_t speed_rpm;//round per minute
    int16_t given_current;
    uint8_t temperate;
    int16_t last_ecd;
} motor_measure_t;

//TX2 to Gimbal Motor aim coordinate location data package
//TX2����̨�����׼�������ݰ�
typedef struct{
	uint32_t raw_horizontal_pixel;
	uint32_t raw_vertical_pixel;
	uint32_t horizontal_pixel;
	uint32_t vertical_pixel;
} tx2_aim_package_t;

//��̨�����Ǿ��ԽǶȽṹ��
typedef struct
{
	uint16_t absolute_yaw_angle;
	uint16_t absolute_pitch_angle;
} gimbal_gyro_absolute_angle_t;




//����������̵��ID����
extern void CAN_CMD_CHASSIS_RESET_ID(void);

//������̨�����������revΪ�����ֽ�
extern void CAN_CMD_GIMBAL(int16_t yaw, int16_t pitch, int16_t shoot, int16_t rev);
//���͵��̵����������
extern void CAN_CMD_CHASSIS(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4);
//������̨����������
extern void CAN_GIMBAL_GYRO_DATA(int16_t yaw, int16_t pitch);
//������̨����������
extern void CAN_GIMBAL_ENCODE_DATA(uint8_t *data,int id);
//����yaw���������ַ��ͨ��ָ�뷽ʽ��ȡԭʼ����
extern const motor_measure_t *get_Yaw_Gimbal_Motor_Measure_Point(void);
//����pitch���������ַ��ͨ��ָ�뷽ʽ��ȡԭʼ����
extern const motor_measure_t *get_Pitch_Gimbal_Motor_Measure_Point(void);
//����trigger���������ַ��ͨ��ָ�뷽ʽ��ȡԭʼ����
extern const motor_measure_t *get_Trigger_Motor_Measure_Point(void);
//���ص��̵��������ַ��ͨ��ָ�뷽ʽ��ȡԭʼ����,i�ķ�Χ��0-3����Ӧ0x201-0x204,
extern const motor_measure_t *get_Chassis_Motor_Measure_Point(uint8_t i);
//��������
extern tx2_aim_package_t tx2;
#if GIMBAL_MOTOR_6020_CAN_LOSE_SLOVE
extern void GIMBAL_lose_slove(void);
#endif

#endif
