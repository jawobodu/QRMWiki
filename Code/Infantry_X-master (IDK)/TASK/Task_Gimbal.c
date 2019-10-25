#include "Task_Gimbal.h"


#include "arm_math.h"
#include "kalman.h"
#include "kalman_filter.h"

#include "can1.h"
#include "remote.h"
#include "control.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "Task_Chassis.h"

#include "vision.h"
#include "friction.h"
#include "magazine.h"
#include "led.h"


//ע��,����ģʽ�ʹ��ģʽ��PID����ͨģʽ�ǲ�ͬ��

/*-------------------------��̨�Ƕ�Ԥ����---------------------------*/
#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
	//��̨
	#define Mech_Min_Pitch     2800   //up
	#define Mech_Mid_Pitch     3500
	#define Mech_Max_Pitch     3920    //down
	#define Mech_Min_Yaw       2300     //right
	#define Mech_Mid_Yaw       4055
	#define Mech_Max_Yaw       5900     //left

	//��̨̧ͷ�Ƕ�,����Ħ���ֿ���
	#define CLOUD_FRIC_PIT_UP  (Mech_Min_Pitch + 200)
	
	//��̨���̷���Ƕ�,����yaw��λ
	#define CLOUD_SEPAR_ANGLE  950		//С�Ļ��Ťͷģʽ��ͻ
	float down_sb_pitch = 530;//450
	float up_sb_pitch   = 100;//
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE
	//��̨
	#define Mech_Min_Pitch     4140   //up
	#define Mech_Mid_Pitch     5050
	#define Mech_Max_Pitch     5360    //down
	#define Mech_Min_Yaw       1065     //right
	#define Mech_Mid_Yaw       2770
	#define Mech_Max_Yaw       4450     //left

	//��̨̧ͷ�Ƕ�,����Ħ���ֿ���
	#define CLOUD_FRIC_PIT_UP  (Mech_Min_Pitch + 200)//�ӵ�Խ��̧ͷԽС
	
	//��̨���̷���Ƕ�,����yaw��λ
	#define CLOUD_SEPAR_ANGLE  800		//С�Ļ��Ťͷģʽ��ͻ
	
	float down_sb_pitch = 650;
	float up_sb_pitch   = 100;//
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
	//��̨
	#define Mech_Min_Pitch     3380   //up
	#define Mech_Mid_Pitch     4095		
	#define Mech_Max_Pitch     4340    //down
	#define Mech_Min_Yaw       6100     //right
	#define Mech_Mid_Yaw       4134		
	#define Mech_Max_Yaw       1960     //left

	//��̨̧ͷ�Ƕ�,����Ħ���ֿ���
	#define CLOUD_FRIC_PIT_UP  (Mech_Min_Pitch + 200)
	
	//��̨���̷���Ƕ�,����yaw��λ
	#define CLOUD_SEPAR_ANGLE  800		//С�Ļ��Ťͷģʽ��ͻ
	float down_sb_pitch = 470;//430;
	float up_sb_pitch   = 0;//100;//
	
	//����Ƕ�
	float base_mech_pitch = 3965;//Mech_Mid_Pitch;
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
	//��̨
	#define Mech_Min_Pitch     3400   //up
	#define Mech_Mid_Pitch     4100
	#define Mech_Max_Pitch     4430    //down
	#define Mech_Min_Yaw       6100     //right
	#define Mech_Mid_Yaw       4080
	#define Mech_Max_Yaw       2030     //left

	//��̨̧ͷ�Ƕ�,����Ħ���ֿ���
	#define CLOUD_FRIC_PIT_UP  (Mech_Min_Pitch + 200)
	
	//��̨���̷���Ƕ�,����yaw��λ
	#define CLOUD_SEPAR_ANGLE  800		//С�Ļ��Ťͷģʽ��ͻ
	float down_sb_pitch = 430;
	float up_sb_pitch   = 0;//100;//
	
	//����Ƕ�
	float base_mech_pitch = 3890;
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
	//��̨
	#define Mech_Min_Pitch     3355   //up
	#define Mech_Mid_Pitch     4150
	#define Mech_Max_Pitch     4390    //down
	#define Mech_Min_Yaw       6170     //right
	#define Mech_Mid_Yaw       4093
	#define Mech_Max_Yaw       2020     //left

	//��̨̧ͷ�Ƕ�,����Ħ���ֿ���
	#define CLOUD_FRIC_PIT_UP  (Mech_Min_Pitch + 200)
	
	//��̨���̷���Ƕ�,����yaw��λ
	#define CLOUD_SEPAR_ANGLE  800		//С�Ļ��Ťͷģʽ��ͻ
	float down_sb_pitch = 530;
	float up_sb_pitch   = 0;//100;//
	
	//����Ƕ�
	float base_mech_pitch = 3960;
	
#endif
/*-----------------------------------------------------------*/


/*---------------------�����ǽ��ٶȲ���------------------------*/
#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
	#define PALST_COMPS_YAW        (38)
	#define PALST_COMPS_PITCH      (81)
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE
	#define PALST_COMPS_YAW        (0)
	#define PALST_COMPS_PITCH      (58)
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
	#define PALST_COMPS_YAW        (10)
	#define PALST_COMPS_PITCH      (102)
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
	#define PALST_COMPS_YAW        (-5)
	#define PALST_COMPS_PITCH      (62)
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
	#define PALST_COMPS_YAW        (-10)
	#define PALST_COMPS_PITCH      (35)
	
#endif
/*--------------------------------------------------------------*/

//��̨ģʽѡ��
typedef enum
{
	CLOUD_MECH_MODE = 0,
	CLOUD_GYRO_MODE = 1,
} eGimbalCtrlMode;
eGimbalCtrlMode  modeGimbal;


/* ��̨����ģʽ:
   
   ��ͨ             	NORMAL
   ��ͷ180��             AROUND
   ���             	BUFF
   ����,pitchˮƽ   	LEVEL
   ��еģʽpitcḩͷ	HIGH
   ����Ťͷ90��          TURN
*/
typedef enum
{
	GIMBAL_NORMAL  = 0,//����ģʽ,����ģʽѡ��
	GIMBAL_AROUND  = 1,//180���ͷ
	GIMBAL_BUFF    = 2,//���ģʽ,��
	GIMBAL_LEVEL   = 3,//���ֿ���,��̨ˮƽ
	GIMBAL_MANUAL  = 4,//�ֶ����ģʽ
	GIMBAL_SM_BUFF = 5,//С��
	GIMBAL_TURN    = 7,//90��Ťͷ
	GIMBAL_AUTO    = 8,//����
	GIMBAL_BASE    = 9,//��ͷ�������
	
}eGimbalAction;
eGimbalAction  actGimbal;

Critical_t Yaw_Gyro_Angle;

/*************������************/

//��еģʽ�±���ϵ��,����ҡ����Ӧ�ٶ�
float kRc_Mech_Pitch, kRc_Mech_Yaw;

//������ģʽ�±���ϵ��,����ҡ����Ӧ�ٶ�
float kRc_Gyro_Pitch, kRc_Gyro_Yaw;
float krc_gyro_yaw = 0.018;

//��еģʽ�±���ϵ��,���Ƽ�����Ӧ�ٶ�
float kKey_Mech_Pitch, kKey_Mech_Yaw;

//������ģʽ�±���ϵ��,���Ƽ�����Ӧ�ٶ�
float kKey_Gyro_Pitch, kKey_Gyro_Yaw;
float kkey_gyro_yaw = 0.38;

/*******************PID����**********************/

//�����ǲ���
float angleMpuPitch,	angleMpuYaw,	angleMpuRoll;//�����ǽǶ�ֵ
short palstanceMpuPitch,	palstanceMpuYaw,	palstanceMpuRoll;//�����ǽ��ٶ�ֵ

//��е�Ƕ��м����,��CAN�ж�ȡ����
int16_t  angleMotorPit,  angleMotorYaw; 

//�Ƕ����
float Cloud_Angle_Error[2][2];//  pitch/yaw    mech/gyro
float Cloud_Palstance_Error[2][2];//  pitch/yaw    mech/gyro

//���ٶ�����ۼӺ�
float Cloud_Palstance_Error_Sum[2][2];//  pitch/yaw    mech/gyro

//�����Ƕ�
float Cloud_Angle_Target[2][2];//  pitch/yaw    mech/gyro

//�����Ƕ�
float Cloud_Angle_Measure[2][2];//  pitch/yaw    mech/gyro

//�������ٶ�
float Cloud_Palstance_Measure[2][2];//  pitch/yaw    mech/gyro

//PID����
float Cloud_Angle_kpid[2][2][3];//  pitch/yaw    mech/gyro    kp/ki/kd
float Cloud_Palstance_kpid[2][2][3];


//PID
float  pTermPit[2],       pTermYaw[2][2];//   outer/inner    outer/inner//mech/gyro
float  iTermPit[2],       iTermYaw[2][2];
float  pidTermPit[2],     pidTermYaw[2][2];

/**************�޷�****************/
//������̨���������������
float PID_Out_Max;
float PID_Outter_Max;
float PID_Iterm_Max;


/**************б��***************/
float Slope_Mouse_Pitch, Slope_Mouse_Yaw;//Ħ���ֿ���ʱ̧ͷ����
	
//�ϵ�б�±���
float Slope_Begin_Pitch, Slope_Begin_Yaw;//���ϵ�ʱ�ƶ�����

//����������ģʽ�����ͳ��yawƫ����,��ֵ���Լ�������С,��ֹ˦ͷ����
float Mouse_Gyro_Yaw, Mouse_Gyro_Pitch;

//����������ģʽ��QECŤͷ����
float Slope_Turn_Yaw;
float Slope_Back_Yaw;

//����Ħ����̧ͷб��
float Slope_Fric_Pitch;

//����б��
float Slope_Auto_Yaw;
float Slope_Auto_Pitch;

   
/****************��̨����ģʽ�¸�С������������********************/
//��ͷģʽ�Ƕ�Ŀ��
float TURNMode_Yaw_Back_Total;//����C,yaw��Ҫ�ı�ĽǶ�ֵ
float TURNMode_Yaw_Turn_Total;//����QE,yaw��Ҫ�ı�ĽǶ�ֵ,������������ת


/**********************�ִ���*******************/
#define CONFIRM_BEGIN		0//�ս����ֶ���������ȷ��λ��
#define CONFIRM_CENTRE		1//ȷ��Բ��
#define CONFIRM_RADIUS		2//ȷ�ϰ뾶
#define CONFIRM_HIGH		3//ȷ�ϸ߶�
#define CONFIRM_LOCATION	4//ȷ��λ��
int Manual_Step = CONFIRM_BEGIN;//��һ��ȷ��Բ�ģ��ڶ���ȷ���뾶��������WASDȷ��λ��
float Manual_Pitch_Comp = 70;//�ֶ�̧ͷ����
float Buff_Pitch_Comp;//���̧ͷ�Զ�����
float Buff_Yaw_Comp;//������������Զ���������Ϊ�������װ��
float Buff_Pitch_Comp_Gimbal;//���̧ͷ����,����ͷ����̨
float Buff_Yaw_Comp_Gimbal;

float Base_Yaw_Comp_Gimbal;//��������У׼

//���У����ͷ����
float Buff_Pitch_Correct_Chassis;
float Buff_Yaw_Correct_Chassis;
float Buff_Pitch_Correct_Gimbal;
float Buff_Yaw_Correct_Gimbal;

float debug_y_mid;// = 5785;//5797;//�Ӿ�������ת��
float debug_p_mid;// = 3450;//3522;//7.15m̧ͷ��72��е�Ƕ�,쫷���,600pwm

float gb_yaw_posit_error = 0;//λ�ò�����ж��Ƿ��ƶ���λ
float gb_pitch_posit_error = 0;//λ�ò�����ж��Ƿ��ƶ���λ

//�ս������ж�
bool is_firstime_into_buff = TRUE;

/***************����******************/

//���
float Auto_Error_Yaw[2];//    now/last
float Auto_Error_Pitch[2];
float Auto_Distance;//���뵥Ŀ

//���ص�������
float Base_Error_Yaw;
bool first_time_into_base = TRUE;

//����ͻȻ����,�������˲�������ʱ
uint16_t Auto_KF_Delay = 0;


float debug_y_sk;// = 38;//35;//30;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
float debug_y_sb_sk;//�ڱ�Ԥ��ϵ��
float debug_y_sb_brig_sk;//��ͷ�ڱ�
float debug_p_sk;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
float debug_auto_err_y;// = 10;//15;//10;//15;//yaw�Ƕȹ���ر�Ԥ��
float debug_auto_err_p;//pitch�Ƕȹ���ر�Ԥ��
float debug_kf_delay;// = 150;//100;//200;//120;//150;//Ԥ����ʱ����
float debug_kf_speed_yl;//yaw�ٶȹ��͹ر�Ԥ��
float debug_kf_speed_yl_sb;//̧ͷ���ڱ�ʱ��С��Ϳɿ�Ԥ����
float debug_kf_speed_yh;//yaw�ٶȹ��߹ر�Ԥ��
float debug_kf_speed_pl;//pitch�ٶȹ��͹ر�Ԥ��
float debug_kf_y_angcon;// = 130;//125;//115;//135;//yawԤ�����޷�
float debug_kf_p_angcon;//pitchԤ�����޷�


float Vision_Angle_Speed_Yaw, Vision_Angle_Speed_Pitch;//�������˲��ٶȲ���ֵ
float *yaw_kf_result, *pitch_kf_result;//���׿������˲����,0�Ƕ� 1�ٶ�

/*************�������˲�**************/
/*һ�׿�����*/
//��̨�Ƕ�������
extKalman_t Gimbal_Pitch_Mech_Error_Kalman;//����һ��kalmanָ��
extKalman_t Gimbal_Pitch_Gyro_Error_Kalman;//����һ��kalmanָ��
extKalman_t Gimbal_Yaw_Mech_Error_Kalman;//����һ��kalmanָ��
extKalman_t Gimbal_Yaw_Gyro_Error_Kalman;//����һ��kalmanָ��

extKalman_t Vision_Distance_Kalman;

extKalman_t Gimbal_Buff_Yaw_Error_Kalman;//���̴��
extKalman_t Gimbal_Buff_Pitch_Error_Kalman;//

extKalman_t Gimbal_Buff_Yaw_Error_Gim_Kalman;//��̨���
extKalman_t Gimbal_Buff_Pitch_Error_Gim_Kalman;//


/*���׿�����*/
#define KF_ANGLE	0
#define KF_SPEED	1
#define KF_ACCEL	2

speed_calc_data_t Vision_Yaw_speed_Struct;
speed_calc_data_t Vision_Pitch_speed_Struct;

kalman_filter_init_t yaw_kalman_filter_para = {
  .P_data = {2, 0, 0, 2},
  .A_data = {1, 0.002/*0.001*/, 0, 1},//����ʱ����
  .H_data = {1, 0, 0, 1},
  .Q_data = {1, 0, 0, 1},
  .R_data = {200, 0, 0, 400}//500 1000
};//��ʼ��yaw�Ĳ���kalman����

kalman_filter_init_t pitch_kalman_filter_para = {
  .P_data = {2, 0, 0, 2},
  .A_data = {1, 0.002/*0.001*/, 0, 1},//����ʱ����
  .H_data = {1, 0, 0, 1},
  .Q_data = {1, 0, 0, 1},
  .R_data = {200, 0, 0, 400}
};//��ʼ��pitch�Ĳ���kalman����

kalman_filter_t yaw_kalman_filter;
kalman_filter_t pitch_kalman_filter;

/*�Զ����õ�һЩ��־λ*/
bool Mobility_Prediction_Yaw = FALSE;//Ԥ���Ƿ�����־λ
bool Mobi_Pre_Yaw_Fire = FALSE;//Ĭ��Ԥ��û��λ����ֹ��ǹ

uint16_t mobpre_yaw_left_delay = 0;//����Ԥ����ʱ�жϿɿ�������
uint16_t mobpre_yaw_right_delay = 0;//����Ԥ����ʱ�жϿɿ�������
uint16_t mobpre_yaw_stop_delay = 0;//Ԥ��ر���ʱ�жϿɿ�������
/********************************************************************************/
/********************************************************************************/

//ÿ2msִ��һ��������
void Task_Gimbal(void *pvParameters)
{
	portTickType currentTime;	
	
	for(;;)
	{	
		currentTime = xTaskGetTickCount();//��ǰϵͳʱ��
		
		/* ����� */
		if (SYSTEM_GetSystemState() == SYSTEM_STARTING)//��ʼ��ģʽ
		{
			  GIMBAL_InitCtrl();
		}
		else
		{
			if (SYSTEM_GetRemoteMode() == RC)//ң�ؿ���ģʽ
			{
				GIMBAL_Rc_Ctrl();//ң��ģʽ
				actGimbal = GIMBAL_NORMAL;
			}
			else
			{
				GIMBAL_Key_Ctrl();//����ģʽ
			}
		}
		
		if(modeGimbal == CLOUD_MECH_MODE)
		{
			iTermYaw[INNER][GYRO] = 0;
		}
		else
		{
			iTermYaw[INNER][MECH] = 0;
		}
		
		GIMBAL_kPID_Init();//���ݲ���ģʽ�任kpid,ÿ�ζ�Ҫ��
		
		GIMBAL_PositionLoop();
		GIMBAL_CanbusCtrlMotors();
		
		if(actGimbal != GIMBAL_AUTO)
		{
			//�ԽǶȺ��ٶȽ��ж��׿������˲��ں�,0λ��,1�ٶ�
			Vision_Angle_Speed_Yaw = Target_Speed_Calc(&Vision_Yaw_speed_Struct, xTaskGetTickCount(), Cloud_Angle_Measure[YAW][GYRO]);
			Vision_Angle_Speed_Pitch = Target_Speed_Calc(&Vision_Pitch_speed_Struct, xTaskGetTickCount(), Cloud_Angle_Measure[PITCH][MECH]);
			//�ԽǶȺ��ٶȽ��ж��׿������˲��ں�,0λ��,1�ٶ�
			yaw_kf_result = kalman_filter_calc(&yaw_kalman_filter, Cloud_Angle_Measure[YAW][GYRO], 0);
			pitch_kf_result = kalman_filter_calc(&pitch_kalman_filter, Cloud_Angle_Measure[PITCH][MECH], 0);
		}
		
		if(VisionRecvData.distance == 999.99f)
		{
			Orange_On;
		}
		else
		{
			Orange_Off;
		}
		
		vTaskDelayUntil(&currentTime, TIME_STAMP_2MS);//������ʱ
	}
}


/**
  * @brief  ��̨������ʼ��
  * @param  void
  * @retval void
  * @attention û�м�I�������,ֻ��ϵͳ����ʱ����һ��
  */
void GIMBAL_InitArgument(void)
{
	/* ������,��Ӧ���� */	
	kRc_Mech_Yaw   = 0.01;
	kRc_Mech_Pitch = 0.01;
	
	kRc_Gyro_Yaw   = krc_gyro_yaw;//0.015;//����������ģʽ��תͷ�ٶȿ�����Ӱ��������ģʽ�³�Ťͷ�ٶ�
	kRc_Gyro_Pitch = 0.01;
	
	kKey_Mech_Yaw   = 0;
	kKey_Mech_Pitch = 0.45;
	
	kKey_Gyro_Yaw   = -kkey_gyro_yaw;//-0.38;//ע������,����ᷴ��
	kKey_Gyro_Pitch = 0.38;//0.45;
	
	/* б��,�仯���� */
	Slope_Begin_Pitch =  1;//�ϵ�ʱ�ƶ�����
  	Slope_Begin_Yaw   =  1;//�ϵ�ʱ�ƶ�����
	
	Slope_Mouse_Pitch = 15;//20;//�����Ӧ,̧ͷ��ͷ�ٶ�
	Slope_Mouse_Yaw   = 15;//�����Ӧ,Ťͷ�ٶ�
	
	Slope_Turn_Yaw = 20;//25;//QEŤͷ����
	Slope_Back_Yaw = 20;//30;//C��ͷ����
	
	Slope_Auto_Yaw   = 4;//1;//50;
	Slope_Auto_Pitch = 3;//1;//50;
	
	Slope_Fric_Pitch = 8;
	
	/* �޷� */
	#if YAW_POSITION == YAW_UP 
		PID_Out_Max    = 4999;//������̨���������������
		PID_Outter_Max = 6000;
		PID_Iterm_Max  = 3000;
	#else
		PID_Out_Max    = 29999;
		PID_Outter_Max = 29999;
		PID_Iterm_Max  = 18000;
	#endif
		
	/* ����ģʽѡ��,Ĭ������ģʽ */
	actGimbal = GIMBAL_NORMAL;
	GIMBAL_kPID_Init();//PID������ʼ��
	
	/*--------------����ǶȲ�����ʼ��----------------*/
	#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
		
		debug_y_sk = 14.8;//80;//72;//80;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_y_sb_sk = 10;//105;//120;//140;//
		debug_p_sk = 75;//26;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_auto_err_y = 150;//25;//�Ƕȹ���ر�Ԥ��
		debug_auto_err_p = 8;
		debug_kf_delay = 50;//40;//200;//220;//Ԥ����ʱ����
		debug_kf_speed_yl = 0.35;//0.35;//0.45;//�ٶȹ��͹ر�Ԥ��
		debug_kf_speed_yl_sb = 0.3;//
		debug_kf_speed_yh = 25;//4;//6.5;//�ٶȹ��߹ر�Ԥ��
		debug_kf_speed_pl = 0.05;//0.25;//0.08;//pitch�ٶȹ��͹ر�Ԥ��
		debug_kf_y_angcon = 220;//Ԥ�����޷�
		debug_kf_p_angcon = 120;//45;//pitchԤ�����޷�
		
		//���,����
		debug_y_mid     	= 4050;//Mech_Mid_Yaw;
		debug_p_mid     	= 3172;//Mech_Mid_Pitch;		
		Buff_Pitch_Comp 	= 0;//-30;//-33;//-47;//-49;//-59;//-53;//����25.5~26.5
		Buff_Yaw_Comp   	= 0;//-10;//10;//-7;//-12;//-28;//-42;//-45;//-38;//-23;
		Buff_Pitch_Comp_Gimbal = 50;//��̨���У׼
		Buff_Yaw_Comp_Gimbal   = 45;//��̨���У׼
		Buff_Pitch_Correct_Chassis  = 1;//0.93;
		Buff_Yaw_Correct_Chassis 	= 1;//0.97;
		Buff_Pitch_Correct_Gimbal	= 1;
		Buff_Yaw_Correct_Gimbal		= 1;
		
		Base_Yaw_Comp_Gimbal = 0;
		
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE
		
		debug_y_sk = 88;//38;//35;//30;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_y_sb_sk = 10;
		debug_p_sk = 26;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_auto_err_y = 16;//15;//10;//15;//�Ƕȹ���ر�Ԥ��
		debug_auto_err_p = 8;
		debug_kf_delay = 180;//100;//200;//120;//150;//Ԥ����ʱ����
		debug_kf_speed_yl = 0.38;//0.1;//0.2;//0.1;//0.08;//0.1;//0.3;//�ٶȹ��͹ر�Ԥ��
		debug_kf_speed_yh = 6;//�ٶȹ��߹ر�Ԥ��
		debug_kf_speed_yl_sb = 0.1;//
		debug_kf_speed_pl = 0.1;//pitch�ٶȹ��͹ر�Ԥ��
		debug_kf_y_angcon = 260;//125;//115;//135;//Ԥ�����޷�
		debug_kf_p_angcon = 45;//pitchԤ�����޷�
		
		//���
		//����
		debug_y_mid 		= Mech_Mid_Yaw;
		debug_p_mid 		= Mech_Mid_Pitch;
		Buff_Pitch_Comp 	= 0;
		Buff_Yaw_Comp   	= 0;
		
		//��̨
		Buff_Pitch_Comp_Gimbal = 0;
		Buff_Yaw_Comp_Gimbal   = 0;

		Buff_Pitch_Correct_Chassis  = 1;
		Buff_Yaw_Correct_Chassis 	= 1;
		Buff_Pitch_Correct_Gimbal	= 1;
		Buff_Yaw_Correct_Gimbal		= 1;
		
		Base_Yaw_Comp_Gimbal = 0;
		
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
		
		debug_y_sk = 52;//45;//14.8;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_y_sb_sk = 62;//61;//55;
		debug_y_sb_brig_sk = 88;//
		debug_p_sk = 20;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_auto_err_y = 120;//�Ƕȹ���ر�Ԥ��
		debug_auto_err_p = 150;
		debug_kf_delay = 80;//Ԥ����ʱ����
		debug_kf_speed_yl = 0.2;//0.35;//�ٶȹ��͹ر�Ԥ��
		debug_kf_speed_yl_sb = 0.2;//
		debug_kf_speed_yh = 4.2;//�ٶȹ��߹ر�Ԥ��
		debug_kf_speed_pl = 0.15;//pitch�ٶȹ��͹ر�Ԥ��
		debug_kf_y_angcon = 220;//125;//115;//135;//Ԥ�����޷�
		debug_kf_p_angcon = 45;//pitchԤ�����޷�
		
		//���
		//����
		debug_y_mid 		= 4122;//5883;
		debug_p_mid 		= 3860;//3565;
		Buff_Pitch_Comp 	= 0;
		Buff_Yaw_Comp   	= 0;
		
		//��̨
		//7.1��	
		Buff_Pitch_Comp_Gimbal = -78;//-88;//����-88��������-96	
		//8��
		//Buff_Pitch_Comp_Gimbal = -86;//-96;
		Buff_Yaw_Comp_Gimbal   = -8;			//28m/s

		Buff_Pitch_Correct_Chassis  = 1;
		Buff_Yaw_Correct_Chassis 	= 0.99;
		Buff_Pitch_Correct_Gimbal	= 1;
		Buff_Yaw_Correct_Gimbal		= 1;
		
		Base_Yaw_Comp_Gimbal = -20;
		
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
		
		debug_y_sk = 45;//14.8;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_y_sb_sk = 55;
		debug_y_sb_brig_sk = 90;//
		debug_p_sk = 20;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_auto_err_y = 120;//�Ƕȹ���ر�Ԥ��
		debug_auto_err_p = 150;
		debug_kf_delay = 80;//Ԥ����ʱ����
		debug_kf_speed_yl = 0.2;//�ٶȹ��͹ر�Ԥ��
		debug_kf_speed_yl_sb = 0.2;//
		debug_kf_speed_yh = 4;//�ٶȹ��߹ر�Ԥ��
		debug_kf_speed_pl = 0.15;//pitch�ٶȹ��͹ر�Ԥ��
		debug_kf_y_angcon = 220;//125;//115;//135;//Ԥ�����޷�
		debug_kf_p_angcon = 45;//pitchԤ�����޷�
		
		//���
		//����
		debug_y_mid 		= Mech_Mid_Yaw;
		debug_p_mid 		= Mech_Mid_Pitch;
		Buff_Pitch_Comp 	= 0;
		Buff_Yaw_Comp   	= 0;
		
		//��̨
		//7.1��
		Buff_Pitch_Comp_Gimbal = -22;//-30;//-24;//-20;//-10;//����-24��������-30		
		//8��
		//Buff_Pitch_Comp_Gimbal = -30;
		
		Buff_Yaw_Comp_Gimbal   = -13;//19;

		Buff_Pitch_Correct_Chassis  = 1;
		Buff_Yaw_Correct_Chassis 	= 1;
		Buff_Pitch_Correct_Gimbal	= 1;
		Buff_Yaw_Correct_Gimbal		= 1;
		
		Base_Yaw_Comp_Gimbal = -13;
		
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
		
		debug_y_sk = 50;//45;//35;//14.8;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_y_sb_sk = 59;//55;
		debug_y_sb_brig_sk = 90;//
		debug_p_sk = 20;//�ƶ�Ԥ��ϵ��,Խ��Ԥ��Խ��
		debug_auto_err_y = 120;//�Ƕȹ���ر�Ԥ��
		debug_auto_err_p = 150;
		debug_kf_delay = 80;//Ԥ����ʱ����
		debug_kf_speed_yl = 0.2;//0.35;//�ٶȹ��͹ر�Ԥ��
		debug_kf_speed_yl_sb = 0.2;//0.2;//
		debug_kf_speed_yh = 5;//�ٶȹ��߹ر�Ԥ��
		debug_kf_speed_pl = 0.15;//pitch�ٶȹ��͹ر�Ԥ��
		debug_kf_y_angcon = 220;//125;//115;//135;//Ԥ�����޷�
		debug_kf_p_angcon = 45;//pitchԤ�����޷�
		
		//���
		//����
		debug_y_mid 		= Mech_Mid_Yaw;
		debug_p_mid 		= Mech_Mid_Pitch;
		Buff_Pitch_Comp 	= 0;
		Buff_Yaw_Comp   	= 0;
		
		//��̨
		//7.1��
		Buff_Pitch_Comp_Gimbal = -54;//-52;//-25;//����-52��������-60		
		//8��
		//Buff_Pitch_Comp_Gimbal = -63;
		
		Buff_Yaw_Comp_Gimbal   = -27;//-30;

		Buff_Pitch_Correct_Chassis  = 1;
		Buff_Yaw_Correct_Chassis 	= 1;
		Buff_Pitch_Correct_Gimbal	= 1;
		Buff_Yaw_Correct_Gimbal		= 1;
		
		Base_Yaw_Comp_Gimbal = -27;//-30;
		
	#endif	
	
	//�������˲�����ʼ��
	  /*PID�Ƕ�������,һ��*/
	KalmanCreate(&Gimbal_Pitch_Mech_Error_Kalman, 1, 40);
	KalmanCreate(&Gimbal_Pitch_Gyro_Error_Kalman, 1, 40);
	
	KalmanCreate(&Gimbal_Yaw_Mech_Error_Kalman, 1, 40);
	KalmanCreate(&Gimbal_Yaw_Gyro_Error_Kalman, 1, 40);
	
	KalmanCreate(&Vision_Distance_Kalman, 1, 2000);
	
	
	KalmanCreate(&Gimbal_Buff_Yaw_Error_Kalman, 1, 0);//���̴��
	KalmanCreate(&Gimbal_Buff_Pitch_Error_Kalman, 1, 0);//
	
	KalmanCreate(&Gimbal_Buff_Yaw_Error_Gim_Kalman, 1, 0);//��̨���
	KalmanCreate(&Gimbal_Buff_Pitch_Error_Gim_Kalman, 1, 0);//
	
	  /*���鿨�����˲�,����*/
	mat_init(&yaw_kalman_filter.Q,2,2, yaw_kalman_filter_para.Q_data);
	mat_init(&yaw_kalman_filter.R,2,2, yaw_kalman_filter_para.R_data);
	kalman_filter_init(&yaw_kalman_filter, &yaw_kalman_filter_para);
	
	mat_init(&pitch_kalman_filter.Q,2,2, pitch_kalman_filter_para.Q_data);
	mat_init(&pitch_kalman_filter.R,2,2, pitch_kalman_filter_para.R_data);
	kalman_filter_init(&pitch_kalman_filter, &pitch_kalman_filter_para);

}

/**
  * @brief  ��̨pid������ʼ��
  * @param  void
  * @retval void
  * @attention ���/����ģʽ����ͨģʽ�ǲ�һ����,���/����Ҫ��Ӳ
  *            ��û�Ӵ��ģʽ��kpid
  */
/*- ���� -*/
float  v_y_k = 9/*7.2*//*6*/,  v_y_p = 23/*9*//*8*/,  v_y_i = 800/*25*/;
float gv_p_k = 8/*7.7*//*16*/, gv_p_p = 18/*5.2*/, gv_p_i = 400;
/*- ��� -*/
float gb_y_k = 3.1/*11*/, gb_y_p = 26/*7.8*/, gb_y_i = 600/*70*/;//��kp ��kp ��ki
float gb_p_k = 3.1/*14*/, gb_p_p = 23/*5*/, gb_p_i = 500/*50*/;
void GIMBAL_kPID_Init(void)
{
	//���Ϊ����ģʽ���ҵ���Ŀ��
	if( actGimbal ==  GIMBAL_AUTO && VisionRecvData.identify_target == TRUE )
	{
		/*-------------��̨PID����Ԥ����---------------*/
		#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP]=gv_p_k;//7;//7;//7.5;//gv_p_k;//10;
			
			Cloud_Angle_kpid[YAW][MECH][KP]=20;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP]=gv_p_p;//3.7;//4.5;//gv_p_p;//4;
			Cloud_Palstance_kpid[PITCH][MECH][KI]=gv_p_i;//70;//100;//gv_p_i;//80;
			Cloud_Palstance_kpid[PITCH][MECH][KD]=0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]=2.2;
			Cloud_Palstance_kpid[YAW][MECH][KI]=0;
			Cloud_Palstance_kpid[YAW][MECH][KD]=0;
			
			/* kPID,������ģʽ */
				//outer
			Cloud_Angle_kpid[YAW][GYRO][KP]=v_y_k;//6;//6.5;//v_y_k;//8;
			
				//inner		
			Cloud_Palstance_kpid[YAW][GYRO][KP]=v_y_p;//8;//v_y_p;//5;
			Cloud_Palstance_kpid[YAW][GYRO][KI]=v_y_i;//100;//v_y_i;//50;
			Cloud_Palstance_kpid[YAW][GYRO][KD]=0;

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP] = 9;//10;//gv_p_k;//16;
			
			Cloud_Angle_kpid[YAW][MECH][KP]   = 0;//13;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP] = 4.2;//4.8;//gv_p_p;//2.2;
			Cloud_Palstance_kpid[PITCH][MECH][KI] = 65;//75;//gv_p_i;//0.08;
			Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]   = 0;//2.2;
			Cloud_Palstance_kpid[YAW][MECH][KI]   = 0;//0.2;
			Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;
			
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 5;//v_y_k;//0;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 9;//v_y_p;//0;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 50;//v_y_i;//0;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP] = 9.1;//11;//gv_p_k;//7.5;//gv_p_k;//16;
			
			Cloud_Angle_kpid[YAW][MECH][KP]   = 10;//13;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP] = 14;//18;//gv_p_p;//4.5;//gv_p_p;//6;
			Cloud_Palstance_kpid[PITCH][MECH][KI] = 300;//gv_p_i;//100;//gv_p_i;//1;
			Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]   = 6;//2.2;
			Cloud_Palstance_kpid[YAW][MECH][KI]   = 1.2;//0.2;
			Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;
			
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 8.6;//9.2;//12;//v_y_k;//6.5;//v_y_k;//17;//gv_y_k;//14;//10;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 16;//19;//22;//26;//v_y_p;//8;//v_y_p;//2.9;//gv_y_p;//2.6;//2.2;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 400;//600;//v_y_i;//100;//v_y_i;//0.8;//gv_y_i;//0.1;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP] = 9.1;//gv_p_k;//0;//16;
			
			Cloud_Angle_kpid[YAW][MECH][KP]   = 0;//v_y_k;//0;//13;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP] = 15.5;//gv_p_p;//0;//2.2;
			Cloud_Palstance_kpid[PITCH][MECH][KI] = 400;//600;//gv_p_i;//0;//0.08;
			Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]   = 0;//2.2;
			Cloud_Palstance_kpid[YAW][MECH][KI]   = 0;//0.2;
			Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;
			
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 9.2;//v_y_k;//14;//10;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 15;//25;//v_y_p;//2.6;//2.2;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 500;//700;//v_y_i;//0.1;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP] = 8.1;//gv_p_k;//0;//16;
			
			Cloud_Angle_kpid[YAW][MECH][KP]   = 0;//13;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP] = 15;//20;//gv_p_p;//0;//2.2;
			Cloud_Palstance_kpid[PITCH][MECH][KI] = 400;//450;//gv_p_i;//0;//0.08;
			Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]   = 0;//2.2;
			Cloud_Palstance_kpid[YAW][MECH][KI]   = 0;//0.2;
			Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;
			
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 9.2;//v_y_k;//0;//14;//10;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 15;//21;//v_y_p;//0;//2.6;//2.2;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 400;//v_y_i;//0;//0.1;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

		#endif
		/*---------------------------------------------*/
	}
	else if( (GIMBAL_IfBuffHit() == TRUE && VisionRecvData.identify_buff == TRUE) )//���ģʽ��ֻ�л�е
	{
		/*-------------��̨PID����Ԥ����---------------*/
		#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP]=gb_p_k;//14;//gb_p_k;//16;//gb_p_k;//15;
			
			Cloud_Angle_kpid[YAW][MECH][KP]=11;//gb_y_k;//12;//13;//gb_y_k;//15;
		
			Cloud_Angle_kpid[YAW][GYRO][KP]=gb_y_k;//12;//13;//gb_y_k;//15;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP]=gb_p_p;//5;//gb_p_p;//4;//3;//gb_p_p;//3;//3.3;
			Cloud_Palstance_kpid[PITCH][MECH][KI]=gb_p_i;//50;//gb_p_i;//70;//30;//gb_p_i;//20;//0.1;
			Cloud_Palstance_kpid[PITCH][MECH][KD]=0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]=7.8;//gb_y_p;//8;//gb_y_p;//3;
			Cloud_Palstance_kpid[YAW][MECH][KI]=70;//gb_y_i;//125;//50;//gb_y_i;//5;
			Cloud_Palstance_kpid[YAW][MECH][KD]=0;

			Cloud_Palstance_kpid[YAW][GYRO][KP]=gb_y_p;//8;//gb_y_p;//3;
			Cloud_Palstance_kpid[YAW][GYRO][KI]=gb_y_i;//125;//50;//gb_y_i;//5;
			Cloud_Palstance_kpid[YAW][GYRO][KD]=0;
			
		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP] = 11;//12;//16;
			
			Cloud_Angle_kpid[YAW][MECH][KP]   = 10.5;//11;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP] = 4.8;//5;//2.2;
			Cloud_Palstance_kpid[PITCH][MECH][KI] = 70;//75;//0.08;
			Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]   = 5.4;//5.8;
			Cloud_Palstance_kpid[YAW][MECH][KI]   = 85;//100;
			Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;
			
			Cloud_Palstance_kpid[YAW][GYRO][KP]=gb_y_p;//8;//gb_y_p;//3;
			Cloud_Palstance_kpid[YAW][GYRO][KI]=gb_y_i;//125;//50;//gb_y_i;//5;
			Cloud_Palstance_kpid[YAW][GYRO][KD]=0;
				
		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
		
			#if 	BUFF_CAM_TYPE == BUFF_CAM_CHAS
					//outer
				Cloud_Angle_kpid[PITCH][MECH][KP] = 10;//gb_p_k;//15;				
				Cloud_Angle_kpid[YAW][MECH][KP]   = 12;//gb_y_k;//10;
				
					//inner
				Cloud_Palstance_kpid[PITCH][MECH][KP] = 12;//gb_p_p;//5;
				Cloud_Palstance_kpid[PITCH][MECH][KI] = 350;//gb_p_i;//40;
				Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
				
				Cloud_Palstance_kpid[YAW][MECH][KP]   = 24;//gb_y_p;//4;
				Cloud_Palstance_kpid[YAW][MECH][KI]   = 200;//gb_y_i;//3;
				Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;

			#elif 	BUFF_CAM_TYPE == BUFF_CAM_GIMB
					//outer
				Cloud_Angle_kpid[PITCH][MECH][KP] = 8;//5.5;//gb_p_k;			
				Cloud_Angle_kpid[YAW][GYRO][KP]   = 8;//5;//gb_y_k;
				
					//inner
				Cloud_Palstance_kpid[PITCH][MECH][KP] = 20;//22;//gb_p_p;
				Cloud_Palstance_kpid[PITCH][MECH][KI] = 300;//500;//gb_p_i;
				Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;

				Cloud_Palstance_kpid[YAW][GYRO][KP]   = 22;//gb_y_p;
				Cloud_Palstance_kpid[YAW][GYRO][KI]   = 300;//500;//gb_y_i;
				Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;
			
			#endif

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
			#if 	BUFF_CAM_TYPE == BUFF_CAM_CHAS
					//outer
				Cloud_Angle_kpid[PITCH][MECH][KP] = 10;//gb_p_k;//15;				
				Cloud_Angle_kpid[YAW][MECH][KP]   = 12;//gb_y_k;//10;
				
					//inner
				Cloud_Palstance_kpid[PITCH][MECH][KP] = 12;//gb_p_p;//5;
				Cloud_Palstance_kpid[PITCH][MECH][KI] = 350;//gb_p_i;//40;
				Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
				
				Cloud_Palstance_kpid[YAW][MECH][KP]   = 24;//gb_y_p;//4;
				Cloud_Palstance_kpid[YAW][MECH][KI]   = 200;//gb_y_i;//3;
				Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;

			#elif 	BUFF_CAM_TYPE == BUFF_CAM_GIMB
					//outer
				Cloud_Angle_kpid[PITCH][MECH][KP] = 9;//gb_p_k;			
				Cloud_Angle_kpid[YAW][GYRO][KP]   = 9;//gb_y_k;
				
					//inner
				Cloud_Palstance_kpid[PITCH][MECH][KP] = 16;//gb_p_p;
				Cloud_Palstance_kpid[PITCH][MECH][KI] = 500;//gb_p_i;
				Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;

				Cloud_Palstance_kpid[YAW][GYRO][KP]   = 20;//gb_y_p;
				Cloud_Palstance_kpid[YAW][GYRO][KI]   = 500;//gb_y_i;
				Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

			#endif

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
			#if 	BUFF_CAM_TYPE == BUFF_CAM_CHAS
					//outer
				Cloud_Angle_kpid[PITCH][MECH][KP] = 10;//gb_p_k;//15;				
				Cloud_Angle_kpid[YAW][MECH][KP]   = 12;//gb_y_k;//10;
				
					//inner
				Cloud_Palstance_kpid[PITCH][MECH][KP] = 12;//gb_p_p;//5;
				Cloud_Palstance_kpid[PITCH][MECH][KI] = 350;//gb_p_i;//40;
				Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
				
				Cloud_Palstance_kpid[YAW][MECH][KP]   = 24;//gb_y_p;//4;
				Cloud_Palstance_kpid[YAW][MECH][KI]   = 200;//gb_y_i;//3;
				Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;

			#elif 	BUFF_CAM_TYPE == BUFF_CAM_GIMB
					//outer
				Cloud_Angle_kpid[PITCH][MECH][KP] = 9;//9;//gb_p_k;			
				Cloud_Angle_kpid[YAW][GYRO][KP]   = 9;//gb_y_k;
				
					//inner
				Cloud_Palstance_kpid[PITCH][MECH][KP] = 13;//gb_p_p;
				Cloud_Palstance_kpid[PITCH][MECH][KI] = 300;//gb_p_i;
				Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;

				Cloud_Palstance_kpid[YAW][GYRO][KP]   = 20;//gb_y_p;
				Cloud_Palstance_kpid[YAW][GYRO][KI]   = 300;//gb_y_i;
				Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

			#endif

		#endif
		/*---------------------------------------------*/
	}
	else if( GIMBAL_If_Base() == TRUE && VisionRecvData.identify_target == 8 )
	{
		/*-------------��̨PID����Ԥ����---------------*/
		#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO		
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 4.8;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 20;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 200;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE			
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 4.8;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 20;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 200;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;
				
		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
			/* kPID,������ģʽ */
				//outer0	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 4;//4.8;//8;//11;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 18;//20;//12;//38;//5.5;//6;//5;//2.6;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 300;//0;//400;//800;//180;//200;//2;//0.6;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;
			
		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 4.8;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 20;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 200;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR			
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 4.8;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 20;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 200;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

		#endif
		/*---------------------------------------------*/
	}  
	//������������ģʽ�����
	else
	{
		/*-------------��̨PID����Ԥ����---------------*/
		#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
			/* kPID,��еģʽ */ 
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP]=10;//12;//15;//16;//16;//16;
			
			Cloud_Angle_kpid[YAW][MECH][KP]=11;//10;//10;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP]=2.5;//3.8;//4.1;//3;//2.2;
			Cloud_Palstance_kpid[PITCH][MECH][KI]=50;//75;//100;//40;//0.5;//0.08;
			Cloud_Palstance_kpid[PITCH][MECH][KD]=0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]=5.5;//2.5;//2.5;
			Cloud_Palstance_kpid[YAW][MECH][KI]=150;//70;//0.21;//0.2;
			Cloud_Palstance_kpid[YAW][MECH][KD]=0;
			
			/* kPID,������ģʽ */
				//outer		
			Cloud_Angle_kpid[YAW][GYRO][KP]=9.5;
			
				//inner		
			Cloud_Palstance_kpid[YAW][GYRO][KP]=8.5;//9;
			Cloud_Palstance_kpid[YAW][GYRO][KI]=50;//100;//150;//75;
			Cloud_Palstance_kpid[YAW][GYRO][KD]=0;

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP] = 10;//16;
			
			Cloud_Angle_kpid[YAW][MECH][KP]   = 10;//13;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP] = 3.2;//2.2;
			Cloud_Palstance_kpid[PITCH][MECH][KI] = 60;//0.08;
			Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]   = 5;//2.2;
			Cloud_Palstance_kpid[YAW][MECH][KI]   = 100;//0.2;
			Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;
			
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 9.5;//10;//14;//10;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 8.5;//9;//2.6;//2.2;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 50;//35;//0.1;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;
				
		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP] = 12;
			
			Cloud_Angle_kpid[YAW][MECH][KP]   = 12;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP] = 14;//3.5;//4.2;//4;//2.2;
			Cloud_Palstance_kpid[PITCH][MECH][KI] = 300;//70;//100;//70;//0.05;
			Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]   = 24;
			Cloud_Palstance_kpid[YAW][MECH][KI]   = 300;//1.2;//0.2;
			Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;
			
			/* kPID,������ģʽ */
				//outer0	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 9;//8;//11;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 26;//38;//5.5;//6;//5;//2.6;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 600;//800;//180;//200;//2;//0.6;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;
			
		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP] = 12;
			
			Cloud_Angle_kpid[YAW][MECH][KP]   = 11;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP] = 12;
			Cloud_Palstance_kpid[PITCH][MECH][KI] = 300;
			Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]   = 23;
			Cloud_Palstance_kpid[YAW][MECH][KI]   = 400;
			Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;
			
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 9;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 24;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 600;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

		#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
			/* kPID,��еģʽ */
				//outer
			Cloud_Angle_kpid[PITCH][MECH][KP] = 11;//16;
			
			Cloud_Angle_kpid[YAW][MECH][KP]   = 11;//13;
			
				//inner
			Cloud_Palstance_kpid[PITCH][MECH][KP] = 11;//2.2;
			Cloud_Palstance_kpid[PITCH][MECH][KI] = 350;//0.08;
			Cloud_Palstance_kpid[PITCH][MECH][KD] = 0;
			
			Cloud_Palstance_kpid[YAW][MECH][KP]   = 20;//2.2;
			Cloud_Palstance_kpid[YAW][MECH][KI]   = 500;//250;//0.2;
			Cloud_Palstance_kpid[YAW][MECH][KD]   = 0;
			
			/* kPID,������ģʽ */
				//outer	
			Cloud_Angle_kpid[YAW][GYRO][KP]   = 9;//14;//10;
			
				//inner	
			Cloud_Palstance_kpid[YAW][GYRO][KP]   = 20;//22;//2.6;//2.2;
			Cloud_Palstance_kpid[YAW][GYRO][KI]   = 500;//300;//0.1;
			Cloud_Palstance_kpid[YAW][GYRO][KD]   = 0;

		#endif
		/*---------------------------------------------*/
	}
}

/**
  * @brief  ��̨ʧ�ر���
  * @param  void
  * @retval void
  * @attention ���������0
  */
void GIMBAL_StopMotor(void)
{
	float fMotorOutput[2] = {0};
		
	//�⻷�����0
	pidTermPit[INNER] 		= 0;
	pidTermYaw[INNER][MECH] = 0;
	pidTermYaw[INNER][GYRO] = 0;
	
	iTermPit[INNER]   	  = 0;
	iTermYaw[INNER][MECH] = 0;
	iTermYaw[INNER][GYRO] = 0;
	
	fMotorOutput[ PITCH ] = 0;
	fMotorOutput[ YAW   ] = 0;
	
	Critical_Handle_Init(&Yaw_Gyro_Angle, Cloud_Angle_Measure[YAW][GYRO]);//���ò����Ƕ�

	//�ԽǶȺ��ٶȽ��ж��׿������˲��ں�,0λ��,1�ٶ�
	Vision_Angle_Speed_Yaw = Target_Speed_Calc(&Vision_Yaw_speed_Struct, xTaskGetTickCount(), Cloud_Angle_Measure[YAW][GYRO]);
	Vision_Angle_Speed_Pitch = Target_Speed_Calc(&Vision_Pitch_speed_Struct, xTaskGetTickCount(), Cloud_Angle_Measure[PITCH][MECH]);
	//�ԽǶȺ��ٶȽ��ж��׿������˲��ں�,0λ��,1�ٶ�
	yaw_kf_result = kalman_filter_calc(&yaw_kalman_filter, Cloud_Angle_Measure[YAW][GYRO], 0);
	pitch_kf_result = kalman_filter_calc(&pitch_kalman_filter, Cloud_Angle_Measure[PITCH][MECH], 0);

	
	CAN1_Cloud_Send( fMotorOutput );
	actGimbal = GIMBAL_NORMAL;
}

/**
  * @brief  ��̨��ʼ��
  * @param  void
  * @retval void
  * @attention 
  */
void GIMBAL_InitCtrl(void)
{
	static bool bAngleRecord  = FALSE;
	static portTickType  ulTimeCurrent = 0;


	if (xTaskGetTickCount( ) - ulTimeCurrent > TIME_STAMP_100MS)//��֤���ϵ�������´ο���
	{
		  bAngleRecord = FALSE;
	}

	ulTimeCurrent = xTaskGetTickCount( );

	//��¼�ϵ�ʱ��̨��е�Ƕ�
	if (bAngleRecord == FALSE)
	{
		bAngleRecord = TRUE;
			
		Cloud_Angle_Target[PITCH][MECH] = angleMotorPit;
		Cloud_Angle_Target[YAW][MECH] = angleMotorYaw;
	}
	
	Critical_Handle_Init(&Yaw_Gyro_Angle, Cloud_Angle_Measure[YAW][GYRO]);//��¼�����ǳ�ʼ�Ƕ�
	
	//�ԽǶȺ��ٶȽ��ж��׿������˲��ں�,0λ��,1�ٶ�
	Vision_Angle_Speed_Yaw = Target_Speed_Calc(&Vision_Yaw_speed_Struct, xTaskGetTickCount(), Cloud_Angle_Measure[YAW][GYRO]);
	Vision_Angle_Speed_Pitch = Target_Speed_Calc(&Vision_Pitch_speed_Struct, xTaskGetTickCount(), Cloud_Angle_Measure[PITCH][MECH]);
	//�ԽǶȺ��ٶȽ��ж��׿������˲��ں�,0λ��,1�ٶ�
	yaw_kf_result = kalman_filter_calc(&yaw_kalman_filter, Cloud_Angle_Measure[YAW][GYRO], 0);
	pitch_kf_result = kalman_filter_calc(&pitch_kalman_filter, Cloud_Angle_Measure[PITCH][MECH], 0);
	
	modeGimbal = CLOUD_MECH_MODE;//Ĭ���Ի�еģʽ����
	
	//ƽ��������̨�ƶ����м�,��ֹ���ϵ��˦
	Cloud_Angle_Target[PITCH][MECH] = RAMP_float( Mech_Mid_Pitch, Cloud_Angle_Target[PITCH][MECH], Slope_Begin_Pitch);
	Cloud_Angle_Target[YAW][MECH]   = RAMP_float( Mech_Mid_Yaw, Cloud_Angle_Target[YAW][MECH], Slope_Begin_Yaw);
}

/**
  * @brief  ң�ؿ�����̨ģʽ
  * @param  void
  * @retval void
  * @attention �ڴ˸ı�ǶȻ�Ŀ��ֵ
  */
void GIMBAL_Rc_Ctrl( void )
{	
	if(modeGimbal == CLOUD_GYRO_MODE)
	{
		//������̨����̷���Ƕ�
		Gimbal_Chass_Separ_Limit();
	}
	
	if (IF_RC_SW2_DOWN)//s2������,������ģʽ
	{
		modeGimbal = CLOUD_GYRO_MODE;
	}
	else if (IF_RC_SW2_MID)//S2��
	{
		modeGimbal = CLOUD_MECH_MODE;//S2��,��еģʽ
	}
	
	/*-----ң�ؿ��Ƶ��ֿ���------*/
	if ( Magazine_IfWait() == TRUE			
				&& Magazine_IfOpen() == TRUE )
	{
		Cloud_Angle_Target[PITCH][MECH] = Mech_Mid_Pitch;

		if (modeGimbal == CLOUD_MECH_MODE)//��еģʽ��yaw����
		{
			Cloud_Angle_Target[YAW][MECH] = Mech_Mid_Yaw;
		}
		else if (modeGimbal == CLOUD_GYRO_MODE)//������ģʽ��ҡ�˿���
		{
			Cloud_Angle_Target[YAW][GYRO] += -RC_CH0_RLR_OFFSET*kRc_Gyro_Yaw;
		}

	}
	else    
	{
		//��������pitch
		if (modeGimbal == CLOUD_MECH_MODE)
		{
			Cloud_Angle_Target[PITCH][MECH] += -RC_CH1_RUD_OFFSET*kRc_Mech_Pitch; 
			Cloud_Angle_Target[YAW][MECH]    =  Mech_Mid_Yaw; //��еģʽ,yaw�̶�����
			
			Cloud_Angle_Target[YAW][GYRO] = Cloud_Angle_Measure[YAW][GYRO];
		}
		else if (modeGimbal == CLOUD_GYRO_MODE)
		{
			Cloud_Angle_Target[PITCH][MECH] += -RC_CH1_RUD_OFFSET*kRc_Mech_Pitch;//pitch���û�е����ʽ  
			Cloud_Angle_Target[YAW][GYRO]   += -RC_CH0_RLR_OFFSET*kRc_Gyro_Yaw; 
		}
	}	
}

/**
  * @brief  ���̿�����̨ģʽ
  * @param  void
  * @retval void
  * @attention 
  */
void GIMBAL_Key_Ctrl(void)
{	
	if(modeGimbal == CLOUD_GYRO_MODE)
	{
		//������̨����̷���Ƕ�
		Gimbal_Chass_Separ_Limit();
	}
	
	switch(actGimbal)//SB keil���о���
	{
		/*--------------��̨ģʽѡ��----------------*/
		case GIMBAL_NORMAL:
			GIMBAL_NORMAL_Mode_Ctrl();//�ڴ�ѡ�����ģʽ
		break;
		
		/*--------------V  180���ͷ----------------*/
		case GIMBAL_AROUND:
			modeGimbal = CLOUD_GYRO_MODE;//����������ģʽ
		
			if (TURNMode_Yaw_Back_Total == 0)
			{
				actGimbal = GIMBAL_NORMAL;
			}
			else
			{
				Cloud_Angle_Target[YAW][GYRO] = RampInc_float( &TURNMode_Yaw_Back_Total, Cloud_Angle_Target[YAW][GYRO], Slope_Back_Yaw );
			}
		break;
		
		/*------------���ֿ���,��ֹ̧ͷ-----------------*/
		case GIMBAL_LEVEL:
			GIMBAL_LEVEL_Mode_Ctrl();
		break;
		
		/*--------------Q E  90���ͷ----------------*/
		case GIMBAL_TURN:				
			modeGimbal = CLOUD_GYRO_MODE;//����������ģʽ

		    if (TURNMode_Yaw_Turn_Total == 0)
			{
				actGimbal = GIMBAL_NORMAL;
			}
			else
			{
				Cloud_Angle_Target[YAW][GYRO] = RampInc_float( &TURNMode_Yaw_Turn_Total, Cloud_Angle_Target[YAW][GYRO], Slope_Back_Yaw );
			}
		break;
			
		/*--------------�Ҽ�����----------------*/	
		case GIMBAL_AUTO:
			modeGimbal = CLOUD_GYRO_MODE;//����������ģʽ
		
			if(!IF_MOUSE_PRESSED_RIGH)//�ɿ��Ҽ��˳�����
			{
				actGimbal = GIMBAL_NORMAL;
				
				//����Ŀ��ƫ������,�����л�ʱ��̨����
				VisionRecvData.identify_target = FALSE;
				Auto_KF_Delay = 0;//������´��ӳ�Ԥ����
				Mobility_Prediction_Yaw = FALSE;//���Ԥ��û����
				Mobi_Pre_Yaw_Fire = FALSE;//Ĭ�ϱ��Ԥ��û��λ����ֹ����
				
				mobpre_yaw_left_delay  = 0;//������Ԥ��Ŀ����ӳ�
				mobpre_yaw_right_delay = 0;//������Ԥ��Ŀ����ӳ�	
				mobpre_yaw_stop_delay = 0;//ֹͣԤ�⿪����ʱ����
			}
			else
			{
				GIMBAL_AUTO_Mode_Ctrl();//������ƺ���
			}
		break;
		
		/*--------------Ctrl+V����С��----------------*/	
		case GIMBAL_SM_BUFF:
			//���ⷽ���ƶ��˳����
			if(IF_KEY_PRESSED_W || IF_KEY_PRESSED_S || IF_KEY_PRESSED_A || IF_KEY_PRESSED_D
					|| IF_KEY_PRESSED_Q || IF_KEY_PRESSED_E)
			{
				actGimbal = GIMBAL_NORMAL;
				modeGimbal = CLOUD_GYRO_MODE;//�˳�����л�������ģʽ
				
				gb_yaw_posit_error   = 1000;
				gb_pitch_posit_error = 1000;
				
				is_firstime_into_buff = TRUE;
			}
			else
			{
				#if 	BUFF_CAM_TYPE == BUFF_CAM_CHAS
					modeGimbal = CLOUD_MECH_MODE;//���ʱ���̲���
					GIMBAL_BUFF_Mode_Ctrl_Chassis();//���̴��
				
				#elif	BUFF_CAM_TYPE == BUFF_CAM_TYPE == BUFF_CAM_GIMB
					modeGimbal = CLOUD_GYRO_MODE;//���ʱ���̲���
					GIMBAL_BUFF_Mode_Ctrl_Gimbal();//��̨���
				
				#endif
			}		
		break;	
			
		/*--------------Ctrl+F������----------------*/	
		case GIMBAL_BUFF:
			//���ⷽ���ƶ��˳����
			if(IF_KEY_PRESSED_W || IF_KEY_PRESSED_S || IF_KEY_PRESSED_A || IF_KEY_PRESSED_D
					|| IF_KEY_PRESSED_Q || IF_KEY_PRESSED_E || IF_KEY_PRESSED_V)
			{
				actGimbal = GIMBAL_NORMAL;
				modeGimbal = CLOUD_GYRO_MODE;//�˳�����л�������ģʽ
				
				gb_yaw_posit_error   = 1000;
				gb_pitch_posit_error = 1000;
				
				is_firstime_into_buff = TRUE;
			}
			else
			{
				#if 	BUFF_CAM_TYPE == BUFF_CAM_CHAS
					modeGimbal = CLOUD_MECH_MODE;//���ʱ���̲���
					GIMBAL_BUFF_Mode_Ctrl_Chassis();//���̴��
				
				#elif	BUFF_CAM_TYPE == BUFF_CAM_TYPE == BUFF_CAM_GIMB
					modeGimbal = CLOUD_GYRO_MODE;//���ʱ���̲���
					GIMBAL_BUFF_Mode_Ctrl_Gimbal();//��̨���
				
				#endif
			}		
		break;
			
		/*--------------C������----------------*/
		case GIMBAL_BASE:
			modeGimbal = CLOUD_GYRO_MODE;//����������ģʽ
		
			if(!IF_KEY_PRESSED_C)//�ɿ��Ҽ��˳�����
			{
				actGimbal = GIMBAL_NORMAL;
				first_time_into_base = TRUE;
			}
			else
			{
				GIMBAL_BASE_Mode_Ctrl();
			}
		break;
		/*--------------Ctrl+G���ִ���----------------*/	
//		case GIMBAL_MANUAL:
//			//QEV�˳�
//			if(IF_KEY_PRESSED_Q || IF_KEY_PRESSED_E || IF_KEY_PRESSED_V)
//			{
//				actGimbal   = GIMBAL_NORMAL;
//				modeGimbal  = CLOUD_GYRO_MODE;//�˳�����л�������ģʽ
//				Manual_Step = CONFIRM_BEGIN;
//				Manual_Pitch_Comp = 72;//���ò���
//			}
//			else
//			{
//				modeGimbal = CLOUD_MECH_MODE;//���ʱ���̲���
//				GIMBAL_MANUAL_Mode_Ctrl();
//			}		
//		break;
	}
	
}

/**
  * @brief  ������̨����̷���Ƕ�
  * @param  void
  * @retval void
  * @attention 
  */
void Gimbal_Chass_Separ_Limit(void)
{
	if ( (GIMBAL_GetOffsetAngle() <= -CLOUD_SEPAR_ANGLE				//�ҹ���
			&& (RC_Ctl.mouse.x>0||RC_Ctl.rc.ch0>RC_CH_VALUE_OFFSET))	
				|| ( GIMBAL_GetOffsetAngle() >= CLOUD_SEPAR_ANGLE	//�����
					&& ( RC_Ctl.mouse.x<0 || RC_Ctl.rc.ch0<RC_CH_VALUE_OFFSET) ) )
	{
//		RC_Ctl.mouse.x = 0;
//		//���Ե�ʱ��ǵ�ע�ͣ���������ch0�а�������ݵ����
//		RC_Ctl.rc.ch0  = RC_CH_VALUE_OFFSET;
		
		kRc_Gyro_Yaw   -= krc_gyro_yaw/300;//0.005;
		kKey_Gyro_Yaw   -= -krc_gyro_yaw/300;//-0.15;
		
		if(abs(kRc_Gyro_Yaw) < abs(krc_gyro_yaw/290)
			|| abs(kKey_Gyro_Yaw) < abs(krc_gyro_yaw/290))
		{
			kRc_Gyro_Yaw = 0;
			kKey_Gyro_Yaw = 0;
		}
	}
	else
	{
		kRc_Gyro_Yaw   = krc_gyro_yaw;//0.015;
		kKey_Gyro_Yaw   = -kkey_gyro_yaw;//-0.38;
	}
	
	#if YAW_POSITION == YAW_UP
		if ( (GIMBAL_GetOffsetAngle() <= (Mech_Min_Yaw - Mech_Mid_Yaw + 50)	&& RC_Ctl.mouse.x>0)//�ҹ���	
					|| ( GIMBAL_GetOffsetAngle() >= (Mech_Max_Yaw - Mech_Mid_Yaw - 50) && RC_Ctl.mouse.x<0 )	//�����
		   )
		{
			kKey_Gyro_Yaw = 0;
		}
		else
		{
			kKey_Gyro_Yaw   = -kkey_gyro_yaw;
		}
	#else
		if ( (GIMBAL_GetOffsetAngle() <= (Mech_Max_Yaw - Mech_Mid_Yaw +500) && RC_Ctl.mouse.x>0)//�ҹ���	
					|| ( GIMBAL_GetOffsetAngle() >= (Mech_Min_Yaw - Mech_Mid_Yaw - 500) && RC_Ctl.mouse.x<0 )//�����	
		   )
		{
			kKey_Gyro_Yaw = 0;
		}
		else
		{
			kKey_Gyro_Yaw   = -kkey_gyro_yaw;
		}
	#endif
}


/*******************��̨����ģʽ����ģʽС����*******************/

/**
  * @brief  ��̨����ģʽѡ��,������Ӧ
  * @param  void
  * @retval void
  * @attention ��̨���̿���״̬�µ�����ģʽ�л�������
  * ��ģʽ�л�ʱһֱ���ڴ�ģʽ
  */
void GIMBAL_NORMAL_Mode_Ctrl(void)
{
	//������ʱ��Ӧ,��ֹ�ּ���
	static portTickType  Key_Ctrl_CurrentTime = 0;
	static uint32_t PressV_Time  = 0;//��ͷ,500ms��ʱ��Ӧ,1����ఴ2��
	static uint32_t PressQ_Time  = 0;//90��,250ms��ʱ��Ӧ,1����ఴ4��
    static uint32_t PressE_Time  = 0;//90��,250ms��ʱ��Ӧ,1����ఴ4��
	static uint32_t PressCF_Time  = 0;//����,400ms��ʱ��Ӧ
//	static uint32_t PressCG_Time  = 0;//�ֶ����,400ms��ʱ��Ӧ
	static uint32_t PressCV_Time  = 0;//��С��,400ms��ʱ��Ӧ
	static uint32_t Mouse_Yaw_Stop  = 0;//��겻����������Ӧ
	static uint32_t Mouse_Pitch_Stop  = 0;//��겻����������Ӧ
	
	Key_Ctrl_CurrentTime = xTaskGetTickCount( );//��ȡʵʱʱ��,������������ʱ�ж�	
	
	
	if ( CHASSIS_IfActiveMode() == TRUE || Magazine_IfOpen() ==	 TRUE)//��ȡ����ģʽ,trueΪ��еģʽ
	{
		modeGimbal = CLOUD_MECH_MODE;
	} 
	else					//ע�͵�loop�еĵ��̻���������ģʽʧЧ
	{
		modeGimbal = CLOUD_GYRO_MODE;
	}

	Manual_Step = CONFIRM_BEGIN;//�˳��ִ�����������
	
	if ( !IF_KEY_PRESSED_CTRL && IF_KEY_PRESSED_V
					&& Key_Ctrl_CurrentTime > PressV_Time)
	{   //Ctrl�����ڰ���״̬ʱ��V��ͷ
		actGimbal  =  GIMBAL_AROUND;//�л��ɵ�ͷģʽ

		PressV_Time = Key_Ctrl_CurrentTime + TIME_STAMP_500MS;//500ms��ʱ���ּ���

		if(IF_KEY_PRESSED_A)//AV���ͷ
		{
			TURNMode_Yaw_Back_Total = 3579;
		}
		else if(IF_KEY_PRESSED_D)//DV�ҵ�ͷ
		{
			TURNMode_Yaw_Back_Total = -3579;
		}
		else//Ĭ���ҵ�ͷ
		{
				TURNMode_Yaw_Back_Total = -3579;//��Ϊ�ǶȷŴ���20��,������180��*20Լ����3579
		}
	}
	/*---------------------------------*/	
	else if ( !IF_KEY_PRESSED_CTRL
				&& ( (IF_KEY_PRESSED_Q && Key_Ctrl_CurrentTime > PressQ_Time)
					|| (IF_KEY_PRESSED_E && Key_Ctrl_CurrentTime > PressE_Time) ) )
	{   //Ctrl�����ڰ���״̬ʱ��Q(��),E(��)90���ͷ
		actGimbal = GIMBAL_TURN;//�л��ɿ���Ťͷģʽ
		
		//ע�ⷽ��
		if ( IF_KEY_PRESSED_Q)
		{
			PressQ_Time = Key_Ctrl_CurrentTime + TIME_STAMP_250MS;//250ms��ʱ���ּ���
			
			TURNMode_Yaw_Turn_Total = 1789;//Q��תԼ8192/4��
		}
		else if (IF_KEY_PRESSED_E)
		{
			PressE_Time = Key_Ctrl_CurrentTime + TIME_STAMP_250MS;//250ms��ʱ���ּ���
			
			TURNMode_Yaw_Turn_Total = -1789;//E��תԼ8192/4��
		}
			
	}	
	/*---------------------------------*/
	else if ( Magazine_IfWait() == TRUE			//���ֿ��������ڿ���,��̨���в�����
				|| Magazine_IfOpen() == TRUE )
	{
		actGimbal = GIMBAL_LEVEL;

	}
	/*---------------------------------*/
	else if (IF_MOUSE_PRESSED_RIGH && !IF_RC_SW1_MID)//��SW1������,���Ҽ�����
	{
		actGimbal = GIMBAL_AUTO;

	}
	/*----------------С��-----------------*/
	else if(IF_KEY_PRESSED_V && IF_KEY_PRESSED_CTRL && Key_Ctrl_CurrentTime > PressCV_Time)//Ctrl+F���,400ms��Ӧһ��
	{
		PressCV_Time = Key_Ctrl_CurrentTime + TIME_STAMP_400MS;
		actGimbal = GIMBAL_SM_BUFF;
	}
	/*----------------���-----------------*/
	else if(IF_KEY_PRESSED_F && IF_KEY_PRESSED_CTRL && Key_Ctrl_CurrentTime > PressCF_Time)//Ctrl+F���,400ms��Ӧһ��
	{
		PressCF_Time = Key_Ctrl_CurrentTime + TIME_STAMP_400MS;
		actGimbal = GIMBAL_BUFF;
	}
	/*----------------����-----------------*/
	else if(IF_KEY_PRESSED_C && !IF_MOUSE_PRESSED_RIGH && !IF_RC_SW1_MID)
	{
		actGimbal = GIMBAL_BASE;
	}
	/*---------------------------------*/
//	else if(IF_KEY_PRESSED_G && IF_KEY_PRESSED_CTRL && Key_Ctrl_CurrentTime > PressCG_Time)//Ctrl+G�ֶ����,400ms��Ӧһ��
//	{
//		PressCG_Time = Key_Ctrl_CurrentTime + TIME_STAMP_400MS;
//		actGimbal = GIMBAL_MANUAL;
//	}
	/*---------------------------------*/
	else       //�������̨�Ƕȼ���,������ͨģʽ�µĽǶȼ���,���ȼ����,���Է������
	{
		if (modeGimbal == CLOUD_MECH_MODE)//��еģʽ
		{
			Cloud_Angle_Target[PITCH][MECH] += MOUSE_Y_MOVE_SPEED * kKey_Mech_Pitch;
			Cloud_Angle_Target[YAW][MECH]    = Mech_Mid_Yaw;	//yaw���ֲ���,��Զ���м�
			
			Cloud_Angle_Target[YAW][GYRO] = Cloud_Angle_Measure[YAW][GYRO];
//			Critical_Handle_Init(&Yaw_Gyro_Angle, Cloud_Angle_Measure[YAW][GYRO]);//���ò����Ƕ�			
		}
		else if (modeGimbal == CLOUD_GYRO_MODE)
		{
			Mouse_Gyro_Yaw   += MOUSE_X_MOVE_SPEED * kKey_Gyro_Yaw;//��¼Ŀ��仯�Ƕ�
			Mouse_Gyro_Pitch += MOUSE_Y_MOVE_SPEED * kKey_Gyro_Pitch;//pitch�Ծ�ʹ�û�еģʽ
//			Cloud_Angle_Target[PITCH][MECH] += MOUSE_Y_MOVE_SPEED * kKey_Gyro_Pitch;//pitch�Ծ�ʹ�û�еģʽ
			if(MOUSE_X_MOVE_SPEED == 0)
			{
				Mouse_Yaw_Stop ++ ;
				if(Mouse_Yaw_Stop > 25)//��곤ʱ��ͣ����ֹͣ�ƶ�
				{
					Mouse_Gyro_Yaw = 0;
				}
			}
			else
			{
				Mouse_Yaw_Stop = 0;
			}
			
			if(MOUSE_Y_MOVE_SPEED == 0)
			{
				Mouse_Pitch_Stop ++ ;
				if(Mouse_Pitch_Stop > 25)//��곤ʱ��ͣ����ֹͣ�ƶ�
				{
					Mouse_Gyro_Pitch = 0;
				}
			}
			else
			{
				Mouse_Pitch_Stop = 0;
			}
			Cloud_Angle_Target[YAW][GYRO]   = RampInc_float( &Mouse_Gyro_Yaw, Cloud_Angle_Target[YAW][GYRO], Slope_Mouse_Yaw );
			Cloud_Angle_Target[PITCH][MECH] = RampInc_float( &Mouse_Gyro_Pitch, Cloud_Angle_Target[PITCH][MECH], Slope_Mouse_Pitch );
		}
	}
}

/**
  * @brief  Ħ���ֿ���,��̨̧ͷ
  * @param  void
  * @retval void
  * @attention ��ģʽ�½�ֹ����pitch
  */
void GIMBAL_HIGH_Mode_Ctrl(void)
{
	if (FRIC_IfOpen( ) == TRUE)//Ħ���ֿ������,�����л�������ģʽ��
	{
		actGimbal = GIMBAL_NORMAL;
		Cloud_Angle_Target[PITCH][MECH] = Mech_Mid_Pitch;//�������pitch����
	}
	else		//pitcḩͷ��Ħ���ֿ���
	{
		Cloud_Angle_Target[PITCH][MECH] = RAMP_float(CLOUD_FRIC_PIT_UP, Cloud_Angle_Target[PITCH][MECH], Slope_Fric_Pitch);
	}
	
	if (modeGimbal == CLOUD_MECH_MODE)
	{
		Cloud_Angle_Target[YAW][MECH]    =  Mech_Mid_Yaw; //��еģʽ,yaw�̶�����
	}
	else
	{
		Mouse_Gyro_Yaw += MOUSE_X_MOVE_SPEED * kKey_Gyro_Yaw;//��¼Ŀ��仯�Ƕ�
		Cloud_Angle_Target[YAW][GYRO] = RampInc_float( &Mouse_Gyro_Yaw, Cloud_Angle_Target[YAW][GYRO], Slope_Mouse_Yaw );
	}
}

/**
  * @brief  ����ģʽ
  * @param  void
  * @retval void
  * @attention ��ģʽ�½�ֹ����pitch
  */
void GIMBAL_LEVEL_Mode_Ctrl(void)
{
	modeGimbal = CLOUD_MECH_MODE;//����ʱ�����еģʽ
	
	//�������,�˳�����ģʽ
	if( Magazine_IfWait() == FALSE			
			&& Magazine_IfOpen() == FALSE )
	{
		actGimbal = GIMBAL_NORMAL;
	}
	else//����δ���,�Ƕȹ̶����м�
	{
		Cloud_Angle_Target[YAW][MECH]   = Mech_Mid_Yaw;
		Cloud_Angle_Target[PITCH][MECH] = Mech_Mid_Pitch;
	}
}

/**********************************************************************************/
/**
  * @brief  ������ƺ���
  * @param  void
  * @retval void
  * @attention �м�����(0,0),�����Ҹ�,�ϸ�����
  *            yawΪ������ģʽ,pitchΪ��еģʽ(��ʵpitchȫ�̶����û�еģʽ)
  *            ֻ�е����������˲��ڵ�ǰʵʱ�Ƕ����ۼ�����Ŀ��Ƕ�
  *            ������һ��,Ŀ��ǶȾ�ʵʱ����
  */
float debug_y_dk = 450;//yaw����Ԥ�������Խ��Ԥ��Խ��
uint32_t Vision_Time[2];// NOW/LAST

int vision_time_js;
float error_yaw_k   = 1;//7.5;//5.6;//2.2;//���Ŵ�
float error_pitch_k = 10;//5;//3;//2.1;//���Ŵ�
float debug_kf_y_angle;//yawԤ���ݴ�
float debug_kf_p_angle;//pitchԤ���ݴ�

//���ݾ������Ԥ��������޷�
float yaw_speed_k = 0;
float kf_yaw_angcon = 0;

float pitch_speed_k = 0;
float kf_pitch_angcon = 0;

float debug_kf_angle_temp;//Ԥ��Ƕ�б���ݴ���
float debug_kf_angle_ramp = 20;//Ԥ��Ƕ�б�±仯��
float debug_kf_dist;
float debug_dist_bc = 0;
float gim_auto_ramp_y = 5;//10;//�տ�������ʱ�����ƹ�ȥ����ֹ�Ӿ���Ӱ��֡
float gim_auto_ramp_p = 5;//�տ�������ʱ�����ƹ�ȥ����ֹ�Ӿ���Ӱ��֡
int js_yaw = 0;
int js_pitch = 0;

float kf_speed_yl = 0;//

void GIMBAL_AUTO_Mode_Ctrl(void)
{
	static uint32_t Mouse_Yaw_Stop  = 0;//��겻����������Ӧ
	static uint32_t Mouse_Pitch_Stop  = 0;//��겻����������Ӧ
	
	static float yaw_angle_raw, pitch_angle_raw;//�������˲��ǶȲ���ֵ
	static float yaw_angle_ref;//��¼Ŀ��Ƕ�
	static float pitch_angle_ref;//��¼Ŀ��Ƕ�
	
	float kf_delay_open = 0;
	
	Mobility_Prediction_Yaw = FALSE;//Ĭ�ϱ��Ԥ��û����
	Mobi_Pre_Yaw_Fire = FALSE;//Ĭ�ϱ��Ԥ��û��λ����ֹ����
	
	kf_speed_yl = debug_kf_speed_yl;

	//��ȡ�Ƕ�ƫ����,ŷ��������,�����������Ӿ��ľ�׼��
	Vision_Error_Angle_Yaw(&Auto_Error_Yaw[NOW]);
	Vision_Error_Angle_Pitch(&Auto_Error_Pitch[NOW]);
	Vision_Get_Distance(&Auto_Distance);
	
	Auto_Distance = KalmanFilter(&Vision_Distance_Kalman, Auto_Distance);
	
	/*�����������������������������������������������ݸ��¡�������������������������������������������������������������*/
	if(Vision_If_Update() == TRUE)//�Ӿ����ݸ�����
	{
		//����Ŀ��Ƕ�//��¼��ǰʱ�̵�Ŀ��λ��,Ϊ��������׼��
		yaw_angle_ref   = Cloud_Angle_Measure[YAW][GYRO]   + Auto_Error_Yaw[NOW]   * error_yaw_k;
		pitch_angle_ref = Cloud_Angle_Measure[PITCH][MECH] + Auto_Error_Pitch[NOW] * error_pitch_k;
		
		Vision_Clean_Update_Flag();//һ��Ҫ�ǵ�����,�����һֱִ��
		Vision_Time[NOW] = xTaskGetTickCount();//��ȡ�����ݵ���ʱ��ʱ��
	}
	/*���������������������������������������������������ݸ��¡���������������������������������������������������������*/
	
	/*�����������������������������������������������׿����������������������������������������������������������������������*/
	if(Vision_Time[NOW] != Vision_Time[LAST])//���������ݵ�����ʱ��
	{
		vision_time_js = Vision_Time[NOW] - Vision_Time[LAST];//�����Ӿ��ӳ�
		yaw_angle_raw  = yaw_angle_ref;//���¶��׿������˲�����ֵ
		pitch_angle_raw = pitch_angle_ref;
		Vision_Time[LAST] = Vision_Time[NOW];
	}
	
	//Ŀ���ٶȽ���
	if(VisionRecvData.identify_target == TRUE)//ʶ����Ŀ��
	{
		Vision_Angle_Speed_Yaw = Target_Speed_Calc(&Vision_Yaw_speed_Struct, Vision_Time[NOW], yaw_angle_raw);
		Vision_Angle_Speed_Pitch = Target_Speed_Calc(&Vision_Pitch_speed_Struct, Vision_Time[NOW], pitch_angle_raw);
		//�ԽǶȺ��ٶȽ��ж��׿������˲��ں�,0λ��,1�ٶ�
		yaw_kf_result = kalman_filter_calc(&yaw_kalman_filter, yaw_angle_raw, Vision_Angle_Speed_Yaw);
		pitch_kf_result = kalman_filter_calc(&pitch_kalman_filter, pitch_angle_raw, Vision_Angle_Speed_Pitch);

	}
	else
	{
//		//�ԽǶȺ��ٶȽ��ж��׿������˲��ں�,0λ��,1�ٶ�
		Vision_Angle_Speed_Yaw = Target_Speed_Calc(&Vision_Yaw_speed_Struct, xTaskGetTickCount(), Cloud_Angle_Measure[YAW][GYRO]);
		Vision_Angle_Speed_Pitch = Target_Speed_Calc(&Vision_Pitch_speed_Struct, xTaskGetTickCount(), Cloud_Angle_Measure[PITCH][MECH]);
//		//�ԽǶȺ��ٶȽ��ж��׿������˲��ں�,0λ��,1�ٶ�
		yaw_kf_result = kalman_filter_calc(&yaw_kalman_filter, Cloud_Angle_Measure[YAW][GYRO], 0);
		pitch_kf_result = kalman_filter_calc(&pitch_kalman_filter, Cloud_Angle_Measure[PITCH][MECH], 0);
		debug_kf_angle_temp = 0;
	}
	
	if(Chassis_IfCORGI() == TRUE && GIMBAL_AUTO_PITCH_SB() == FALSE)//Ť���Ҳ��ڴ��ڱ�
	{
		kf_delay_open = debug_kf_delay*3.f;
	}
	else
	{
		kf_delay_open = debug_kf_delay;
	}
	
	//δʶ��Ŀ��ʱ�������������̨
	if(VisionRecvData.identify_target == TRUE)//ʶ����Ŀ��
	{
		Auto_KF_Delay++;//�˲���ʱ����

		if(VisionRecvData.auto_too_close == TRUE 
			&& (Chassis_IfCORGI() == FALSE || GIMBAL_AUTO_PITCH_SB() == FALSE) )//Ŀ�����̫������СԤ��
		{
			yaw_speed_k = debug_y_sk;///4.f;//3.f;//Ԥ��ϵ������
			kf_yaw_angcon = debug_kf_y_angcon;//3.f;//2.f;
			kf_speed_yl = debug_kf_speed_yl;
		}
		else//����Ԥ����
		{
			if( GIMBAL_AUTO_PITCH_SB() == TRUE )
			{
				yaw_speed_k = debug_y_sb_sk;			
				kf_yaw_angcon = debug_kf_y_angcon;
				kf_speed_yl = debug_kf_speed_yl_sb;
				
				if(IF_KEY_PRESSED_G)
				{
					yaw_speed_k = debug_y_sb_brig_sk;
					kf_yaw_angcon = debug_kf_y_angcon*1.1f;
					kf_speed_yl = debug_kf_speed_yl_sb*0.4f;//0.9f;
				}
			}
			else
			{
				yaw_speed_k = debug_y_sk;
				kf_yaw_angcon = debug_kf_y_angcon;
				kf_speed_yl = debug_kf_speed_yl;
			}
		}
		/*���������������������������������������������������׿������������������������������������������������������������������*/
		js_yaw = yaw_kf_result[KF_SPEED]*1000;
		js_pitch = yaw_kf_result[KF_ANGLE]*1000;
		
		/*Ԥ�⿪������*/
		if(fabs(Auto_Error_Yaw[NOW]) < debug_auto_err_y//debug�� 
				&& Auto_KF_Delay > kf_delay_open 
					&& fabs(yaw_kf_result[KF_SPEED]) >= kf_speed_yl 
						&& fabs(yaw_kf_result[KF_SPEED]) < debug_kf_speed_yh )
		{
			
			if(yaw_kf_result[KF_SPEED]>=0)
			{
				debug_kf_angle_temp = yaw_speed_k * (yaw_kf_result[KF_SPEED] - kf_speed_yl) * 1;//debug_kf_dist;
			}
			else if(yaw_kf_result[KF_SPEED]<0)
			{
				debug_kf_angle_temp = yaw_speed_k * (yaw_kf_result[KF_SPEED] + kf_speed_yl) * 1;//debug_kf_dist;			
			}
//			debug_kf_angle_temp = debug_y_sk * yaw_kf_result[KF_SPEED];//�˴�����Ҫ�ٶ�̫����Ԥ��
			debug_kf_angle_temp = constrain_float(debug_kf_angle_temp, -debug_kf_y_angcon, debug_kf_y_angcon);//Ԥ���ݴ����޷�
			debug_kf_y_angle = RAMP_float(debug_kf_angle_temp, debug_kf_y_angle, debug_kf_angle_ramp);//Ԥ���������仯
			
			debug_kf_y_angle = constrain_float(debug_kf_y_angle, -debug_kf_y_angcon, debug_kf_y_angcon);
			Cloud_Angle_Target[YAW][GYRO] = yaw_kf_result[KF_ANGLE] + debug_kf_y_angle;//debug_y_sk * (yaw_kf_result[KF_SPEED] - debug_kf_speed);//Vision_Gyro_MovProj_Yaw(yaw_kf_result[1]);//yaw_kf_result[0];
			
			/*��������������������������������������������Ԥ�⵽λ�жϡ�������������������������������������������������������������*/
			/*yaw_kf_result[1]�����������Ƹ���debug��������*/
			/*���Զ��򵯷�Χ����С���*/
			if( (yaw_kf_result[KF_SPEED]>0) //Ŀ�������������ֵ��ʾ˵Ŀ�����ұߣ���˵��Ԥ�⵽λ�ã��ɴ�
					&& (Auto_Error_Yaw[NOW] < 0.3f) )
			{
				mobpre_yaw_right_delay = 0;//����Ԥ�⿪����ʱ����

				mobpre_yaw_left_delay++;
				if(mobpre_yaw_left_delay > 0/*75*/)//�ȶ�150ms
				{
					Mobi_Pre_Yaw_Fire = TRUE;//Ԥ�⵽λ���ɿ���
				}
				else
				{
					Mobi_Pre_Yaw_Fire = FALSE;//Ԥ��û��λ�����ɿ���
				}
			}
			else if( (yaw_kf_result[KF_SPEED]<0) //Ŀ�������������ֵ��ʾ˵Ŀ������ߣ���˵��Ԥ�⵽λ�ã��ɴ�
						&& (Auto_Error_Yaw[NOW] > -0.3f) )
			{
				mobpre_yaw_left_delay = 0;//����Ԥ�⿪����ʱ����
				
				mobpre_yaw_right_delay++;
				if(mobpre_yaw_right_delay > 0/*75*/)//�ȶ�150ms
				{
					Mobi_Pre_Yaw_Fire = TRUE;//Ԥ�⵽λ���ɿ���
				}
				else
				{
					Mobi_Pre_Yaw_Fire = FALSE;//Ԥ��û��λ�����ɿ���
				}
			}
			else
			{
				Mobi_Pre_Yaw_Fire = FALSE;//���Ԥ��û��λ����ֹ����
				
				mobpre_yaw_left_delay = 0;//����Ԥ�⿪����ʱ����
				mobpre_yaw_right_delay = 0;//����Ԥ�⿪����ʱ����
			}
			/*������������������������������������������������Ԥ�⵽λ�жϡ���������������������������������������������������������*/
			Mobility_Prediction_Yaw = TRUE;//���Ԥ���ѿ���

			mobpre_yaw_stop_delay = 0;//���þ�ֹʱ�Ŀ����ӳ�
			
			
			Green_On;
			Red_On;
			Orange_On;
		}
		/*Ԥ������û�ﵽ���ر�Ԥ��*/
		else
		{
			Cloud_Angle_Target[YAW][GYRO] = yaw_angle_ref;
			Mobility_Prediction_Yaw = FALSE;//���Ԥ��û����
			mobpre_yaw_left_delay  = 0;//������Ԥ��Ŀ����ӳ�
			mobpre_yaw_right_delay = 0;//������Ԥ��Ŀ����ӳ�	
			
			if( fabs(Auto_Error_Yaw[NOW]) < 1.5f )//�ӽ�Ŀ��
			{
				mobpre_yaw_stop_delay++;
				if(mobpre_yaw_stop_delay > 25)//ֹͣ�ȶ�50ms
				{
					Mobi_Pre_Yaw_Fire = TRUE;//��ʱ�����Ӿ������־λ������жϣ��ǵ�һ��Ҫ��TRUE
				}
			}
			else
			{
				Mobi_Pre_Yaw_Fire = FALSE;//���û�ظ���λ����ֹ����
			}	
			Green_Off;
			Red_Off;
			Orange_Off;
		}
		
		/*---------------pitch������С��Ԥ��------------------*/

		if( Auto_KF_Delay > debug_kf_delay 
				&& fabs(Auto_Error_Pitch[NOW]) < debug_auto_err_p
					&& fabs(pitch_kf_result[KF_SPEED]) > debug_kf_speed_pl
						&& (GIMBAL_AUTO_PITCH_SB_SK() == FALSE || GIMBAL_AUTO_PITCH_SB() == FALSE)
							&& VisionRecvData.distance/100 < 4.4f
		  )
		{	
			if(VisionRecvData.auto_too_close == TRUE)//Ŀ�����̫������СԤ��
			{
				pitch_speed_k = debug_p_sk/2.f;//Ԥ��ϵ������
				kf_pitch_angcon = debug_kf_p_angcon/1.5f;
			}
			else//����Ԥ����
			{
				pitch_speed_k = debug_p_sk;
				kf_pitch_angcon = debug_kf_p_angcon;
			}
			
			if(pitch_kf_result[KF_SPEED]>=0)
			{
				debug_kf_p_angle = pitch_speed_k * (pitch_kf_result[KF_SPEED] - debug_kf_speed_pl);
			}
			else
			{
				debug_kf_p_angle = pitch_speed_k * (pitch_kf_result[KF_SPEED] + debug_kf_speed_pl);			
			}
			//pitchԤ�����޷�
			debug_kf_p_angle = constrain_float(debug_kf_p_angle, -kf_pitch_angcon, kf_pitch_angcon);
			
			Cloud_Angle_Target[PITCH][MECH] = pitch_kf_result[KF_ANGLE] + debug_kf_p_angle;
		}
		/*Ԥ������û�ﵽ���ر�Ԥ��*/
		else
		{
			Cloud_Angle_Target[PITCH][MECH] = pitch_angle_ref;
		}
	}
	else		//δʶ��Ŀ��,�����������̨
	{
		yaw_kf_result = kalman_filter_calc(&yaw_kalman_filter, Cloud_Angle_Measure[YAW][GYRO], 0);
		pitch_kf_result = kalman_filter_calc(&pitch_kalman_filter, Cloud_Angle_Measure[PITCH][MECH], 0);
		
		if (modeGimbal == CLOUD_MECH_MODE)//��еģʽ
		{
			Cloud_Angle_Target[PITCH][MECH] += MOUSE_Y_MOVE_SPEED * kKey_Mech_Pitch;
			Cloud_Angle_Target[YAW][MECH]    = Mech_Mid_Yaw;	//yaw���ֲ���,��Զ���м�			
		}
		else if (modeGimbal == CLOUD_GYRO_MODE)
		{
			Mouse_Gyro_Yaw   += MOUSE_X_MOVE_SPEED * kKey_Gyro_Yaw;//��¼Ŀ��仯�Ƕ�
			Mouse_Gyro_Pitch += MOUSE_Y_MOVE_SPEED * kKey_Gyro_Pitch;//pitch�Ծ�ʹ�û�еģʽ
			if(MOUSE_X_MOVE_SPEED == 0)
			{
				Mouse_Yaw_Stop ++ ;
				if(Mouse_Yaw_Stop > 25)//��곤ʱ��ͣ����ֹͣ�ƶ�
				{
					Mouse_Gyro_Yaw = 0;
				}
			}
			else
			{
				Mouse_Yaw_Stop = 0;
			}
			
			if(MOUSE_Y_MOVE_SPEED == 0)
			{
				Mouse_Pitch_Stop ++ ;
				if(Mouse_Pitch_Stop > 25)//��곤ʱ��ͣ����ֹͣ�ƶ�
				{
					Mouse_Gyro_Pitch = 0;
				}
			}
			else
			{
				Mouse_Pitch_Stop = 0;
			}
			Cloud_Angle_Target[YAW][GYRO]   = RampInc_float( &Mouse_Gyro_Yaw, Cloud_Angle_Target[YAW][GYRO], Slope_Mouse_Yaw );
			Cloud_Angle_Target[PITCH][MECH] = RampInc_float( &Mouse_Gyro_Pitch, Cloud_Angle_Target[PITCH][MECH], Slope_Mouse_Pitch );
		}
		Auto_KF_Delay = 0;	

		mobpre_yaw_left_delay  = 0;//������Ԥ��Ŀ����ӳ�
		mobpre_yaw_right_delay = 0;//������Ԥ��Ŀ����ӳ�	
		mobpre_yaw_stop_delay = 0;//ֹͣԤ�⿪����ʱ����		
	}
}

float speed_threshold = 5.f;//�ٶȹ���
float debug_speed;//�����Ҹ�,һ�㶼��1����,debug��
float Target_Speed_Calc(speed_calc_data_t *S, uint32_t time, float position)
{
	S->delay_cnt++;

	if (time != S->last_time)
	{
		S->speed = (position - S->last_position) / (time - S->last_time) * 2;//�����ٶ�

//		if ((S->speed - S->processed_speed) < -speed_threshold)
//		{
//			S->processed_speed = S->processed_speed - speed_threshold;//�ٶ�б�±仯
//		}
//		else if ((S->speed - S->processed_speed) > speed_threshold)
//		{
//			S->processed_speed = S->processed_speed + speed_threshold;//�ٶ�б�±仯
//		}

		S->processed_speed = S->speed;

		S->last_time = time;
		S->last_position = position;
		S->last_speed = S->speed;
		S->delay_cnt = 0;
	}

	if(S->delay_cnt > 300/*100*/) // delay 200ms speed = 0
	{
		S->processed_speed = 0;//ʱ���������Ϊ�ٶȲ���
	}
	debug_speed = S->processed_speed;
	return S->processed_speed;//��������ٶ�
}


float debug_angle_diff = 2;
bool Gimb_If_Small_Top(float angle)
{
	static float last_angle = 0;
	static float prev_angle = 0;
		   float angle_diff = 0;
	static uint16_t top_change_times = 0;//��װ�״����������ж��Ƿ�С����
	
	prev_angle = angle;
	
	angle_diff = prev_angle - last_angle;
	if(angle_diff >= debug_angle_diff)//�����ж��Ƿ�С���ݣ�һ�������С���ݻ���װ���л�
	{
		top_change_times++;
	}
	else
	{
		top_change_times = 0;
	}
	
	last_angle = angle;
	
	if(top_change_times > 3)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  ���ģʽ������ͷλ�ڵ���
  * @param  void
  * @retval void
  * @attention �췽���ɫ,��������ɫ,ͣ��3�뼤����,������Ҫ�ؼ���
  *  5��������ȴ,�������1��2,��糵�������2��5,�ھ�70cm,�⾶80cm
  *  ѡȡ�������꣨��̨��е����Ϊ0�����ڴ˻����ϵ����Ӿ����꣬��
  *  �Ƕ�ֵת������̨��е�Ƕ�     x/8192 = angle/360;
  */
uint32_t Vision_Buff_Time[2];//����֡��
int vision_buff_time_js;

float gb_yaw_angle = 0;//��ȡ�Ƕ�
float gb_pitch_angle = 0;//��ȡ�Ƕ�
float debug_gb_y_error = 0;//��е�ǶȲ���ֵ��Ŀ��ֵ֮������
float debug_gb_p_error = 0;//��е�ǶȲ���ֵ��Ŀ��ֵ֮������
float buff_y_raw;//�˲����ֵ
float buff_p_raw;//�˲����ֵ
float gb_pitch_compe = 0;//̧ͷ����
//float gb_t;//�ӵ�����ʱ��
//float gb_dist = 7.15;//�������
//float gb_v = 27.5;//�ӵ������ٶ�
//float gb_vd;//�ӵ�ˮƽ�����ٶ�
void GIMBAL_BUFF_Mode_Ctrl_Chassis(void)
{
	float gb_yaw_mech = 0;
	float gb_pitch_mech = 0;
	float gb_y_ref = 0;
	float gb_p_ref = 0;
	
	
	
	/*- ֡�ʲ��� -*/
	if( Vision_If_Update() == TRUE //identify_buff=2Ҳ��ʶ����
			&& (VisionRecvData.identify_buff == TRUE || VisionRecvData.identify_buff == 2) )//�Ӿ����ݸ�����
	{
		//��ȡ�Ƕ�
		Vision_Buff_Error_Angle_Yaw(&gb_yaw_angle);
		Vision_Buff_Error_Angle_Pitch(&gb_pitch_angle);
		
		Vision_Buff_Time[NOW] = xTaskGetTickCount();//��ȡ�����ݵ���ʱ��ʱ��
		Vision_Clean_Update_Flag();//һ��Ҫ�ǵ�����,�����һֱִ��
	}
	
	if(Vision_Buff_Time[NOW] != Vision_Buff_Time[LAST])//���������ݵ�����ʱ��
	{
		vision_buff_time_js = Vision_Buff_Time[NOW] - Vision_Buff_Time[LAST];//�����Ӿ��ӳ�
		Vision_Buff_Time[LAST] = Vision_Buff_Time[NOW];
	}
	/*--------------------------------*/
	
	
	buff_y_raw = KalmanFilter(&Gimbal_Buff_Yaw_Error_Kalman, gb_yaw_angle);
	buff_p_raw = KalmanFilter(&Gimbal_Buff_Pitch_Error_Kalman, gb_pitch_angle);

//	//̧ͷ����
//	// 1/2gt^2  g=10  t = dist/vd   vd=v*cos(p)
//	gb_vd = gb_v * cos(fabs(buff_p_raw*PI/180.0f));//�����ӵ�ˮƽ�����ٶȣ��ǶȻ�û�����ô�㣬�Ƿ�Ҫ��ʵ�ʽǶȣ���Ϊ��̨���ͺ�
//	gb_t = gb_dist / gb_vd;//�����ӵ�����ʱ��	
//	gb_pitch_compe = 0.5f * 10 * gb_t * gb_t;//  1/2 g t^2
	
	//ŷ����ת���ɻ�е��
//	gb_yaw_mech   = gb_yaw_angle   / 360.0f * 8192;//δ�˲�
//	gb_pitch_mech = gb_pitch_angle / 360.0f * 8192;//δ�˲�
	gb_yaw_mech   = buff_y_raw / 360.0f * 8192 * Buff_Yaw_Correct_Chassis;
	gb_pitch_mech = buff_p_raw / 360.0f * 8192 * Buff_Pitch_Correct_Chassis;//δ��̧ͷ����
//	gb_pitch_mech = buff_p_raw / 360.0f * 8192;// - gb_pitch_compe/VisionRecvData.distance*180/PI/360.0f*8192;//����̧ͷ����
	
	//�ݴ�Ŀ��λ��
	gb_y_ref = /*Mech_Mid_Yaw*/debug_y_mid   + gb_yaw_mech;
	gb_p_ref = /*Mech_Mid_Pitch*/debug_p_mid + gb_pitch_mech;
	
	//�ƶ�,identify_buffΪ1��2����ʶ��
	if(VisionRecvData.identify_buff == TRUE || VisionRecvData.identify_buff == 2)
	{
		Cloud_Angle_Target[YAW][MECH]   = gb_y_ref + Buff_Yaw_Comp;
		Cloud_Angle_Target[PITCH][MECH] = gb_p_ref + Buff_Pitch_Comp;
	}
	else
	{
		Cloud_Angle_Target[YAW][MECH]   += 0;//debug_y_mid;//Mech_Mid_Yaw;
		Cloud_Angle_Target[PITCH][MECH] += 0;//debug_p_mid;//Mech_Mid_Pitch;
	}
	
	//�����������ж��Ƿ���׼��λ
	gb_yaw_posit_error   = Cloud_Angle_Measure[YAW][MECH]   - Cloud_Angle_Target[YAW][MECH];
	gb_pitch_posit_error = Cloud_Angle_Measure[PITCH][MECH] - Cloud_Angle_Target[PITCH][MECH];
	
	//���������������ͺ�����
	debug_gb_y_error = gb_yaw_posit_error;
	debug_gb_p_error = gb_pitch_posit_error;
}

/**
  * @brief  ���ģʽ������ͷλ����̨
  * @param  void
  * @retval void
  * @attention �췽���ɫ,��������ɫ,ͣ��3�뼤����,������Ҫ�ؼ���
  *  5��������ȴ,�������1��2,��糵�������2��5,�ھ�70cm,�⾶80cm
  *  pitch���ǻ�еģʽ��������һ��
  */
float gb_yaw_angle_gim = 0;//��ȡ�Ƕ�
float gb_pitch_angle_gim = 0;//��ȡ�Ƕ�
float buff_gimb_ramp_yaw = 72;//120;//����72��������120
float buff_gimb_ramp_pitch = 72;//120;
float buff_into_time = 0;
void GIMBAL_BUFF_Mode_Ctrl_Gimbal(void)
{
	float gb_yaw_gyro = 0;
	float gb_pitch_mech = 0;
	static float y_mid = 0;
	static float p_mid = 0;
	
	static float yaw_buff_angle_raw, pitch_buff_angle_raw;//�������˲��ǶȲ���ֵ
//	static float yaw_buff_angle_ref;//��¼Ŀ��Ƕ�
//	static float pitch_buff_angle_ref;//��¼Ŀ��Ƕ�
	static float shoot_time = 0;
	static float lost_time  = 0;//һ��ʱ��ûʶ�𵽣�����
	
	
	if(is_firstime_into_buff == TRUE)
	{
		is_firstime_into_buff = FALSE;
		buff_into_time = 0;
		
		//��¼����ʱ�ĽǶ�
		y_mid = Cloud_Angle_Measure[YAW][GYRO];
		p_mid = Cloud_Angle_Target[PITCH][MECH];
	}
	
	if(is_firstime_into_buff == FALSE)//����һ��ʱ��
	{
		buff_into_time++;
	}
	
	/*- ֡�ʲ��� -*/
	if( Vision_If_Update() == TRUE //identify_buff=2Ҳ��ʶ����
			&& (VisionRecvData.identify_buff == TRUE || VisionRecvData.identify_buff == 2) )//�Ӿ����ݸ�����
	{	
		//���ص�
		Vision_Error_Yaw(&gb_yaw_angle_gim);
		Vision_Error_Pitch(&gb_pitch_angle_gim);
		gb_yaw_gyro   = gb_yaw_angle_gim + Buff_Yaw_Comp_Gimbal;
		gb_pitch_mech = gb_pitch_angle_gim + Buff_Pitch_Comp_Gimbal;//δ��̧ͷ����
		/******************************************************************/
	
		yaw_buff_angle_raw 	 = Cloud_Angle_Measure[YAW][GYRO]   + gb_yaw_gyro;
		pitch_buff_angle_raw = Cloud_Angle_Measure[PITCH][MECH] + gb_pitch_mech;
		
		Vision_Buff_Time[NOW] = xTaskGetTickCount();//��ȡ�����ݵ���ʱ��ʱ��
		Vision_Clean_Update_Flag();//һ��Ҫ�ǵ�����,�����һֱִ��
	}
	
	if(Vision_Buff_Time[NOW] != Vision_Buff_Time[LAST])//���������ݵ�����ʱ��
	{
		vision_buff_time_js = Vision_Buff_Time[NOW] - Vision_Buff_Time[LAST];//�����Ӿ��ӳ�
		Vision_Buff_Time[LAST] = Vision_Buff_Time[NOW];
	}
	/*--------------------------------*/
	
//	yaw_buff_angle_ref   = KalmanFilter(&Gimbal_Buff_Yaw_Error_Gim_Kalman, yaw_buff_angle_raw);
//	pitch_buff_angle_ref = KalmanFilter(&Gimbal_Buff_Pitch_Error_Gim_Kalman, pitch_buff_angle_raw);

	//�ƶ�,identify_buffΪ1��2����ʶ��
	if(VisionRecvData.identify_buff == TRUE || VisionRecvData.identify_buff == 2)
	{
		if(buff_into_time > 100)//��һ�ν�����������̫����Ӧ
		{
			Cloud_Angle_Target[YAW][GYRO]   = RAMP_float(yaw_buff_angle_raw, Cloud_Angle_Measure[YAW][GYRO], buff_gimb_ramp_yaw);
			Cloud_Angle_Target[PITCH][MECH] = RAMP_float(pitch_buff_angle_raw, Cloud_Angle_Measure[PITCH][MECH], buff_gimb_ramp_pitch);
		}
		else
		{
			Cloud_Angle_Target[YAW][GYRO]   = y_mid;
			Cloud_Angle_Target[PITCH][MECH] = p_mid;
		}
		lost_time = 0;
	}
	else
	{
		Cloud_Angle_Target[YAW][GYRO]   += 0;//debug_y_mid;//Mech_Mid_Yaw;
		Cloud_Angle_Target[PITCH][MECH] += 0;//debug_p_mid;//Mech_Mid_Pitch;
		
		lost_time++;
		
		//����һ��ʱ��ûʶ��
		if(lost_time>300)
		{
			Cloud_Angle_Target[YAW][GYRO]	= y_mid;
			Cloud_Angle_Target[PITCH][MECH] = p_mid;
		}
	}
	

	
	//�����������ж��Ƿ���׼��λ
	if(VisionRecvData.identify_buff == FALSE)
	{
		shoot_time++;
		if(shoot_time > 100)//����200MSûʶ��,���Ӵ󣬷�ֹ�˳��ؽ��������
		{
			gb_yaw_posit_error = 1000;
			gb_pitch_posit_error = 1000;
		}
	}
	else
	{
		shoot_time = 0;
		
		if(buff_into_time > 200)
		{
			gb_yaw_posit_error   = Cloud_Angle_Measure[YAW][GYRO]   - yaw_buff_angle_raw;
			gb_pitch_posit_error = Cloud_Angle_Measure[PITCH][MECH] - pitch_buff_angle_raw;
		}
	}

}

/**
  * @brief  ��ͷ����ģʽ
  * @param  void
  * @retval void
  * @attention ���ص㣬������ڣ�ֻ��YAW��PITCH�������֣�����ʶ��Ŀ�귢8
  */
void GIMBAL_BASE_Mode_Ctrl(void)
{
	static uint32_t Mouse_Yaw_Stop  = 0;//��겻����������Ӧ
	static uint32_t Mouse_Pitch_Stop  = 0;//��겻����������Ӧ
	static float yaw_base_angle_raw = 0;
	
	float base_yaw_gyro = 0;
	
	Vision_Base_Yaw_Pixel(&Base_Error_Yaw);
	
	if(first_time_into_base == TRUE && VisionRecvData.identify_target == 8)
	{
		Cloud_Angle_Target[PITCH][MECH] = base_mech_pitch;
		first_time_into_base = FALSE;
	}
	
	if(Vision_If_Update() == TRUE)//�Ӿ����ݸ�����
	{	
		base_yaw_gyro = Base_Error_Yaw + Base_Yaw_Comp_Gimbal;
		
		yaw_base_angle_raw = Cloud_Angle_Measure[YAW][GYRO] + base_yaw_gyro;
		
		Vision_Clean_Update_Flag();//һ��Ҫ�ǵ�����,�����һֱִ��
		Vision_Time[NOW] = xTaskGetTickCount();//��ȡ�����ݵ���ʱ��ʱ��
	}
	
	//δʶ��Ŀ��ʱ�������������̨
	if(VisionRecvData.identify_target == 8)//ʶ����Ŀ�꣬ע����������
	{
		//yaw����
		Cloud_Angle_Target[YAW][GYRO] = yaw_base_angle_raw;
		
		//pitch��������
		if (modeGimbal == CLOUD_MECH_MODE)//��еģʽ
		{
			Cloud_Angle_Target[PITCH][MECH] += MOUSE_Y_MOVE_SPEED * (kKey_Mech_Pitch/3);
		}
		else if (modeGimbal == CLOUD_GYRO_MODE)
		{
			Mouse_Gyro_Pitch += MOUSE_Y_MOVE_SPEED * (kKey_Gyro_Pitch/3);//pitch�Ծ�ʹ�û�еģʽ
			
			if(MOUSE_Y_MOVE_SPEED == 0)
			{
				Mouse_Pitch_Stop ++ ;
				if(Mouse_Pitch_Stop > 25)//��곤ʱ��ͣ����ֹͣ�ƶ�
				{
					Mouse_Gyro_Pitch = 0;
				}
			}
			else
			{
				Mouse_Pitch_Stop = 0;
			}
			Cloud_Angle_Target[PITCH][MECH] = RampInc_float( &Mouse_Gyro_Pitch, Cloud_Angle_Target[PITCH][MECH], Slope_Mouse_Pitch );
		}
	}
	else		//δʶ��Ŀ��,�����������̨
	{	
		if (modeGimbal == CLOUD_MECH_MODE)//��еģʽ
		{
			Cloud_Angle_Target[PITCH][MECH] += MOUSE_Y_MOVE_SPEED * kKey_Mech_Pitch;
			Cloud_Angle_Target[YAW][MECH]    = Mech_Mid_Yaw;	//yaw���ֲ���,��Զ���м�			
		}
		else if (modeGimbal == CLOUD_GYRO_MODE)
		{
			Mouse_Gyro_Yaw   += MOUSE_X_MOVE_SPEED * kKey_Gyro_Yaw;//��¼Ŀ��仯�Ƕ�
			Mouse_Gyro_Pitch += MOUSE_Y_MOVE_SPEED * kKey_Gyro_Pitch;//pitch�Ծ�ʹ�û�еģʽ
			if(MOUSE_X_MOVE_SPEED == 0)
			{
				Mouse_Yaw_Stop ++ ;
				if(Mouse_Yaw_Stop > 25)//��곤ʱ��ͣ����ֹͣ�ƶ�
				{
					Mouse_Gyro_Yaw = 0;
				}
			}
			else
			{
				Mouse_Yaw_Stop = 0;
			}
			
			if(MOUSE_Y_MOVE_SPEED == 0)
			{
				Mouse_Pitch_Stop ++ ;
				if(Mouse_Pitch_Stop > 25)//��곤ʱ��ͣ����ֹͣ�ƶ�
				{
					Mouse_Gyro_Pitch = 0;
				}
			}
			else
			{
				Mouse_Pitch_Stop = 0;
			}
			Cloud_Angle_Target[YAW][GYRO]   = RampInc_float( &Mouse_Gyro_Yaw, Cloud_Angle_Target[YAW][GYRO], Slope_Mouse_Yaw );
			Cloud_Angle_Target[PITCH][MECH] = RampInc_float( &Mouse_Gyro_Pitch, Cloud_Angle_Target[PITCH][MECH], Slope_Mouse_Pitch );
		}	
	}
}

/**
  * @brief  �ֶ����ģʽ
  * @param  void
  * @retval void
  * @attention �췽���ɫ,��������ɫ,ͣ��3�뼤����,������Ҫ�ؼ���
  *  5��������ȴ,�������1��2,��糵�������2��5,�ھ�70cm,�⾶80cm
  *  Ctrl+F���룬��һ���Ҽ�ѡ��Բ�ģ��ڶ����Ҽ�ѡ�����Ұ뾶
  *  �������Ҽ�ѡ�����¸߶ȣ����ֵ���WASD΢��
  *  ������Ϻ�WASD��ʾ�������ң���ʱ������̧ͷ��ͷ��̧ͷ����
  *  ѡ��Բ�ĺ�������ƣ���������ϸ��
  *  ѡ��뾶��������������ƣ���������ϸ��
  */
//�л���־
bool Manual_Switch_Right = 1;//�����ж��Ҽ���Ӧ

float Manual_Centre_Yaw, Manual_Centre_Pitch;//Բ��
float Manual_Radius;//�뾶
float Manual_High;//�߶�
//int Manual_Step = 0;//��һ��ȷ��Բ�ģ��ڶ���ȷ���뾶��������WASDȷ��λ��
void GIMBAL_MANUAL_Mode_Ctrl(void)
{
	if(!IF_MOUSE_PRESSED_RIGH)//�Ҽ��ɿ�
	{
		Manual_Switch_Right = 1;//�����л�����
	}
	
	/*- 0 ���ֽ���ǰ�ǶȲ��� -*/
	if(Manual_Step == CONFIRM_BEGIN)//�ս����ֶ����,�Ƕȶ�ס����
	{
		Cloud_Angle_Target[YAW][MECH]   = Cloud_Angle_Measure[YAW][MECH];
		Cloud_Angle_Target[PITCH][MECH] = Cloud_Angle_Measure[PITCH][MECH];
		Manual_Step = CONFIRM_CENTRE;//������һ��ȷ��Բ��
	}
	
	/*- 1 ȷ��Բ�� -*/
	if(Manual_Step == CONFIRM_CENTRE)//����Բ��
	{
		//���ֵ�
		Cloud_Angle_Target[YAW][MECH]   += MOUSE_X_MOVE_SPEED * (kKey_Gyro_Yaw/5)*YAW_POSITION;
		Cloud_Angle_Target[PITCH][MECH] += MOUSE_Y_MOVE_SPEED * (kKey_Mech_Pitch/5);
		//����΢��
		if(IF_KEY_PRESSED_W)//��
		{
			Cloud_Angle_Target[PITCH][MECH] -= 0.05f;
		}
		else if(IF_KEY_PRESSED_S)//��
		{
			Cloud_Angle_Target[PITCH][MECH] += 0.05f;
		}
		else if(IF_KEY_PRESSED_A)//��
		{
			Cloud_Angle_Target[YAW][MECH]   += 0.05f*YAW_POSITION;
		}
		else if(IF_KEY_PRESSED_D)//��
		{
			Cloud_Angle_Target[YAW][MECH]   -= 0.05f*YAW_POSITION;
		}
	}
	
	if( IF_MOUSE_PRESSED_RIGH && Manual_Step == CONFIRM_CENTRE
			&& Manual_Switch_Right == 1 )
	{
		//ȷ��Բ��
		Manual_Centre_Yaw   = Cloud_Angle_Measure[YAW][MECH];
		Manual_Centre_Pitch = Cloud_Angle_Measure[PITCH][MECH];
		Cloud_Angle_Target[YAW][MECH] = Manual_Centre_Yaw + 100*YAW_POSITION;//��ǰ������������ֵ��뾶
		Manual_Step = CONFIRM_RADIUS;//������һ��ȷ�ϰ뾶
		Manual_Switch_Right = 0;
	}
	
	/*- 2 ȷ���뾶 -*/
	if(Manual_Step == CONFIRM_RADIUS)//���ڰ뾶
	{
		//���ֵ�
		Cloud_Angle_Target[YAW][MECH]   += MOUSE_X_MOVE_SPEED * (kKey_Gyro_Yaw/5)*YAW_POSITION;
		//����΢��
		if(IF_KEY_PRESSED_A)//��
		{
			Cloud_Angle_Target[YAW][MECH]   += 0.05f*YAW_POSITION;
		}
		else if(IF_KEY_PRESSED_D)//��
		{
			Cloud_Angle_Target[YAW][MECH]   -= 0.05f*YAW_POSITION;
		}
	}
	
	if( IF_MOUSE_PRESSED_RIGH && Manual_Step == CONFIRM_RADIUS
			&& Manual_Switch_Right == 1 )
	{
		//����뾶
		Manual_Radius = fabs(Cloud_Angle_Measure[YAW][MECH] - Manual_Centre_Yaw);
		Cloud_Angle_Target[PITCH][MECH] = Manual_Centre_Pitch - Manual_Radius;//��������ֵ��߶�
		Manual_Step = CONFIRM_HIGH;//������һ��ȷ��λ��
		Manual_Switch_Right = 0;
	}
	
	/*- 3 ȷ�����¸߶� -*/
	if(Manual_Step == CONFIRM_HIGH)//���ڸ߶�
	{
		//���ֵ�
		Cloud_Angle_Target[YAW][MECH] = Manual_Centre_Yaw;
		Cloud_Angle_Target[PITCH][MECH] += MOUSE_Y_MOVE_SPEED * (kKey_Mech_Pitch/5);
		
		//����΢��
		if(IF_KEY_PRESSED_W)//��
		{
			Cloud_Angle_Target[PITCH][MECH] -= 0.05f;
		}
		else if(IF_KEY_PRESSED_S)//��
		{
			Cloud_Angle_Target[PITCH][MECH] += 0.05f;
		}
	}
	
	if( IF_MOUSE_PRESSED_RIGH && Manual_Step == CONFIRM_HIGH
			&& Manual_Switch_Right == 1 )
	{
		Manual_High = fabs(Cloud_Angle_Measure[PITCH][MECH] - Manual_Centre_Pitch);
		Manual_Step = CONFIRM_LOCATION;//������һ��ȷ��λ��
		Manual_Switch_Right = 0;
	}
	
	/*- 4 ��ʽ��ʼ�ִ�ע����̨���� -*/
	if(Manual_Step == CONFIRM_LOCATION)
	{
		//���ֵ�̧ͷ����
		Manual_Pitch_Comp -= MOUSE_Y_MOVE_SPEED * (kKey_Mech_Pitch/10);
	}		
	
	if(Manual_Step == CONFIRM_LOCATION
			&& IF_KEY_PRESSED_W)//��
	{
		Cloud_Angle_Target[YAW][MECH]   = Manual_Centre_Yaw;
		Cloud_Angle_Target[PITCH][MECH] = Manual_Centre_Pitch - Manual_High + (-Manual_Pitch_Comp);
	}
	else if(Manual_Step == CONFIRM_LOCATION
				&& IF_KEY_PRESSED_S)//��
	{
		Cloud_Angle_Target[YAW][MECH]   = Manual_Centre_Yaw;
		Cloud_Angle_Target[PITCH][MECH] = Manual_Centre_Pitch + Manual_High + (-Manual_Pitch_Comp);
	}
	else if(Manual_Step == CONFIRM_LOCATION
				&& IF_KEY_PRESSED_A)//��
	{
		Cloud_Angle_Target[YAW][MECH]   = Manual_Centre_Yaw + Manual_Radius*YAW_POSITION;
		Cloud_Angle_Target[PITCH][MECH] = Manual_Centre_Pitch + (-Manual_Pitch_Comp);
	}
	else if(Manual_Step == CONFIRM_LOCATION
				&& IF_KEY_PRESSED_D)//��
	{
		Cloud_Angle_Target[YAW][MECH]   = Manual_Centre_Yaw - Manual_Radius*YAW_POSITION;
		Cloud_Angle_Target[PITCH][MECH] = Manual_Centre_Pitch + (-Manual_Pitch_Comp);
	}
	else if(Manual_Step == CONFIRM_LOCATION )//��������
	{
		Cloud_Angle_Target[YAW][MECH]   = Manual_Centre_Yaw;
		Cloud_Angle_Target[PITCH][MECH] = Manual_Centre_Pitch;
	}
}



/************************��̨����ֵ���¼�����**********************/

/**
  * @brief  ������̨��е�Ƕ�,can1�ж��е���
  * @param  void
  * @retval void
  * @attention 
  */
void GIMBAL_UpdateAngle( char eAxis, int16_t angle )
{
	if (eAxis == PITCH)
	{
		angleMotorPit = angle;
		Cloud_Angle_Measure[PITCH][MECH]  = angleMotorPit;
	}
	else if (eAxis == YAW)
	{
		angleMotorYaw = angle;
		Cloud_Angle_Measure[YAW][MECH]  = angleMotorYaw;
	}
}

/**
  * @brief  ������̨��̬,500HZ,loop�е���
  * @param  void
  * @retval void
  * @attention �Ƕ��ʶȷŴ�
  */
int js_ang_p = 0;
int js_pal_p = 0;
void GIMBAL_UpdatePalstance(void)
{		
	//��ȡ������  �Ƕ�   ���ٶ�   
	mpu_dmp_get_data( &angleMpuRoll, &angleMpuPitch, &angleMpuYaw );
	MPU_Get_Gyroscope( &palstanceMpuPitch, &palstanceMpuRoll, &palstanceMpuYaw );
	
	#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
		//�������ǽǶȷŴ�,���Ŵ���������ģʽ�ڻ�PҪ���ܴ�,�᲻�õ�
		Cloud_Angle_Measure[PITCH][GYRO]  =  angleMpuPitch * 20;
	
		//���ٶȸ���
		Cloud_Palstance_Measure[PITCH][MECH] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][MECH]   = -(palstanceMpuYaw  + PALST_COMPS_YAW)*YAW_POSITION;
		
		Cloud_Palstance_Measure[PITCH][GYRO] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][GYRO]   = -(palstanceMpuYaw  + PALST_COMPS_YAW);
		
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE	//���ط���
		//�������ǽǶȷŴ�,���Ŵ���������ģʽ�ڻ�PҪ���ܴ�,�᲻�õ�
		Cloud_Angle_Measure[PITCH][GYRO]  =  angleMpuPitch * 20;
	
		//���ٶȸ���
		Cloud_Palstance_Measure[PITCH][MECH] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][MECH]   = -(palstanceMpuYaw  + PALST_COMPS_YAW)*YAW_POSITION;
		
		Cloud_Palstance_Measure[PITCH][GYRO] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][GYRO]   = -(palstanceMpuYaw  + PALST_COMPS_YAW);
		
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
		//�������ǽǶȷŴ�,���Ŵ���������ģʽ�ڻ�PҪ���ܴ�,�᲻�õ�
		Cloud_Angle_Measure[PITCH][GYRO]  =  angleMpuPitch * 20;
	
		//���ٶȸ���
		Cloud_Palstance_Measure[PITCH][MECH] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][MECH]   = -(palstanceMpuYaw  + PALST_COMPS_YAW)*YAW_POSITION;
		
		Cloud_Palstance_Measure[PITCH][GYRO] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][GYRO]   = -(palstanceMpuYaw  + PALST_COMPS_YAW);
		
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
		//�������ǽǶȷŴ�,���Ŵ���������ģʽ�ڻ�PҪ���ܴ�,�᲻�õ�
		Cloud_Angle_Measure[PITCH][GYRO]  =  angleMpuPitch * 20;
	
		//���ٶȸ���
		Cloud_Palstance_Measure[PITCH][MECH] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][MECH]   = -(palstanceMpuYaw  + PALST_COMPS_YAW)*YAW_POSITION;
		
		Cloud_Palstance_Measure[PITCH][GYRO] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][GYRO]   = -(palstanceMpuYaw  + PALST_COMPS_YAW);
	
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
		//�������ǽǶȷŴ�,���Ŵ���������ģʽ�ڻ�PҪ���ܴ�,�᲻�õ�
		Cloud_Angle_Measure[PITCH][GYRO]  =  angleMpuPitch * 20;
	
		//���ٶȸ���
		Cloud_Palstance_Measure[PITCH][MECH] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][MECH]   = -(palstanceMpuYaw  + PALST_COMPS_YAW)*YAW_POSITION;
		
		Cloud_Palstance_Measure[PITCH][GYRO] = (palstanceMpuPitch + PALST_COMPS_PITCH);
		Cloud_Palstance_Measure[YAW][GYRO]   = -(palstanceMpuYaw  + PALST_COMPS_YAW);
	
	#endif
	
	Cloud_Angle_Measure[YAW][GYRO] = Gimbal_Yaw_Gryo_AngleSum(&Yaw_Gyro_Angle , (angleMpuYaw * 20));
	
	js_ang_p = angleMpuPitch * 1000;
	js_pal_p = palstanceMpuPitch * 1000;
}

/**
  * @brief  ���͵���ֵ��CAN1
  * @param  void
  * @retval void
  * @attention ע��һ���Ƿ����ڻ�����
  */
float gc_y = 0;
void GIMBAL_CanbusCtrlMotors(void)
{
	float fMotorOutput[2] = {0};
	
	#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
		//ע��һ���Ƿ����ڻ�����,ע�ⷽ��(������)
		fMotorOutput[PITCH] = -pidTermPit[INNER]*YAW_POSITION;
		if(modeGimbal == CLOUD_MECH_MODE)
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][MECH]*YAW_POSITION;
		}
		else
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][GYRO]*YAW_POSITION;
		}
	
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE
		//ע��һ���Ƿ����ڻ�����,ע�ⷽ��(������)
		fMotorOutput[PITCH] = -pidTermPit[INNER]*YAW_POSITION;
		if(modeGimbal == CLOUD_MECH_MODE)
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][MECH]*YAW_POSITION;
		}
		else
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][GYRO]*YAW_POSITION;
		}
	
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
		//ע��һ���Ƿ����ڻ�����,ע�ⷽ��(������)
		fMotorOutput[PITCH] = -pidTermPit[INNER]*YAW_POSITION;
		if(modeGimbal == CLOUD_MECH_MODE)
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][MECH]*YAW_POSITION;
		}
		else
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][GYRO]*YAW_POSITION;
		};
	
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
		//ע��һ���Ƿ����ڻ�����,ע�ⷽ��(������)
		fMotorOutput[PITCH] = -pidTermPit[INNER]*YAW_POSITION;
		if(modeGimbal == CLOUD_MECH_MODE)
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][MECH]*YAW_POSITION;
		}
		else
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][GYRO]*YAW_POSITION;
		}
	
	#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
		//ע��һ���Ƿ����ڻ�����,ע�ⷽ��(������)
		fMotorOutput[PITCH] = -pidTermPit[INNER]*YAW_POSITION;
		if(modeGimbal == CLOUD_MECH_MODE)
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][MECH]*YAW_POSITION;
		}
		else
		{
			fMotorOutput[YAW]   = -pidTermYaw[INNER][GYRO]*YAW_POSITION;
		}
		
	#endif
	gc_y = 	fMotorOutput[PITCH] ;
	CAN1_Cloud_QueueSend(fMotorOutput);   
}

/*****************************��̨λ��PID����***********************************/

/**
  * @brief  pid����
  * @param  void
  * @retval void
  * @attention �˴����ܸı�Ŀ��Ƕ�,ֻ���������޷��͵���PID���㺯��
  */
void GIMBAL_PositionLoop(void)
{
	if (modeGimbal == CLOUD_MECH_MODE)//��еģʽ
	{
		//pitch
		Cloud_Angle_Target[PITCH][GYRO] = Cloud_Angle_Measure[PITCH][GYRO];
		//pitch�Ƕ�����
		Cloud_Angle_Target[PITCH][MECH] = constrain_float( Cloud_Angle_Target[PITCH][MECH], Mech_Min_Pitch, Mech_Max_Pitch );
		vPitch_Mech_PositionLoop();
		
		//yaw
		Cloud_Angle_Target[YAW][GYRO] = Cloud_Angle_Measure[YAW][GYRO];//��еģʽ��ʵʱ��¼�����ǽǶ�,��ֹ�л�ģʽʱ˦ͷ
		//yaw�Ƕ�����
		#if	YAW_POSITION == YAW_UP
			Cloud_Angle_Target[YAW][MECH] = constrain_float( Cloud_Angle_Target[YAW][MECH], Mech_Min_Yaw, Mech_Max_Yaw );
		#else
			Cloud_Angle_Target[YAW][MECH] = constrain_float( Cloud_Angle_Target[YAW][MECH], Mech_Max_Yaw,  Mech_Min_Yaw);
		#endif
		vYaw_Mech_PositionLoop();
	}
	
	else if(modeGimbal == CLOUD_GYRO_MODE)//������ģʽ
	{
		//pitch
		Cloud_Angle_Target[PITCH][GYRO] = Cloud_Angle_Measure[PITCH][GYRO];
		//pitch�Ƕ�����
		Cloud_Angle_Target[PITCH][MECH] = constrain_float( Cloud_Angle_Target[PITCH][MECH], Mech_Min_Pitch, Mech_Max_Pitch );
		vPitch_Mech_PositionLoop();//������ģʽ��PITCH�õ����ǻ�еģʽ
		
		
		//yaw
		Cloud_Angle_Target[YAW][MECH] = Cloud_Angle_Measure[YAW][MECH];//������ģʽ��ʵʱ��¼��е�Ƕ�,��ֹ�л�ģʽʱ˦ͷ
		
		vYaw_Gyro_PositionLoop();//YAW������ģʽ���
	}
}


/**
  * @brief  pitch��еģʽ
  * @param  void
  * @retval void
  * @attention ����PID�����
  */
int js_pid_p = 0;
void vPitch_Mech_PositionLoop(void)
{
	//�Ƕ����
	Cloud_Angle_Error[PITCH][MECH] = Cloud_Angle_Target[PITCH][MECH] - Cloud_Angle_Measure[PITCH][MECH];
	//�����п������˲�,������Ƶ�ͷ��ȶ���
	Cloud_Angle_Error[PITCH][MECH] = KalmanFilter(&Gimbal_Pitch_Mech_Error_Kalman, Cloud_Angle_Error[PITCH][MECH]);
	//�⻷���
	pTermPit[OUTER] = Cloud_Angle_Error[PITCH][MECH] * Cloud_Angle_kpid[PITCH][MECH][KP];
	pidTermPit[OUTER] = pTermPit[OUTER];
	pidTermPit[OUTER] = constrain_float(pidTermPit[OUTER],-PID_Outter_Max,PID_Outter_Max);
	
	//���ٶ����
	Cloud_Palstance_Error[PITCH][MECH] = pidTermPit[OUTER] - Cloud_Palstance_Measure[PITCH][MECH];
	//�ڻ����
	pTermPit[INNER] = Cloud_Palstance_Error[PITCH][MECH] * Cloud_Palstance_kpid[PITCH][MECH][KP];
	iTermPit[INNER] += Cloud_Palstance_Error[PITCH][MECH] * Cloud_Palstance_kpid[PITCH][MECH][KI] * 0.002f;
	iTermPit[INNER] = constrain_float(iTermPit[INNER], -PID_Iterm_Max, PID_Iterm_Max);

	pidTermPit[INNER] = pTermPit[INNER] + iTermPit[INNER];
	pidTermPit[INNER] = constrain_float(pidTermPit[INNER], -PID_Out_Max, PID_Out_Max);
	js_pid_p = pidTermPit[INNER]*1000;
}

/**
  * @brief  pitch������ģʽ
  * @param  void
  * @retval void
  * @attention ����PID�����
  */
void vPitch_Gyro_PositionLoop(void)
{
	//�Ƕ����
	Cloud_Angle_Error[PITCH][GYRO] = Cloud_Angle_Target[PITCH][GYRO] - Cloud_Angle_Measure[PITCH][GYRO];
	//�����п������˲�,������Ƶ�ͷ��ȶ���
	Cloud_Angle_Error[PITCH][GYRO] = KalmanFilter(&Gimbal_Pitch_Gyro_Error_Kalman, Cloud_Angle_Error[PITCH][GYRO]);
	//�⻷���
	pTermPit[OUTER] = Cloud_Angle_Error[PITCH][GYRO] * Cloud_Angle_kpid[PITCH][GYRO][KP];
	pidTermPit[OUTER] = pTermPit[OUTER];
	pidTermPit[OUTER] = constrain_float(pidTermPit[OUTER],-PID_Outter_Max,PID_Outter_Max);
	
	//���ٶ����
	Cloud_Palstance_Error[PITCH][GYRO] = pidTermPit[OUTER] - Cloud_Palstance_Measure[PITCH][GYRO];
	//�ڻ����
	pTermPit[INNER] = Cloud_Palstance_Error[PITCH][GYRO] * Cloud_Palstance_kpid[PITCH][GYRO][KP];
	iTermPit[INNER] += Cloud_Palstance_Error[PITCH][GYRO] * Cloud_Palstance_kpid[PITCH][GYRO][KI];
	pidTermPit[INNER] = constrain_float(pidTermPit[INNER], -PID_Out_Max, PID_Out_Max);
}

/**
  * @brief  yaw��еģʽ
  * @param  void
  * @retval void
  * @attention ����PID�����
  */
void vYaw_Mech_PositionLoop(void)
{
	//�Ƕ����
	Cloud_Angle_Error[YAW][MECH] = Cloud_Angle_Target[YAW][MECH] - Cloud_Angle_Measure[YAW][MECH];
	//�����п������˲�,������Ƶ�ͷ��ȶ���
	Cloud_Angle_Error[YAW][MECH] = KalmanFilter(&Gimbal_Yaw_Mech_Error_Kalman, Cloud_Angle_Error[YAW][MECH]);
	//�⻷���
	pTermYaw[OUTER][MECH] = Cloud_Angle_Error[YAW][MECH] * Cloud_Angle_kpid[YAW][MECH][KP];
	pidTermYaw[OUTER][MECH] = pTermYaw[OUTER][MECH];
	pidTermYaw[OUTER][MECH] = constrain_float(pidTermYaw[OUTER][MECH],-PID_Outter_Max,PID_Outter_Max);
	
	//���ٶ����
	Cloud_Palstance_Error[YAW][MECH] = pidTermYaw[OUTER][MECH] - Cloud_Palstance_Measure[YAW][MECH];
	//�ڻ����
	pTermYaw[INNER][MECH]  = Cloud_Palstance_Error[YAW][MECH] * Cloud_Palstance_kpid[YAW][MECH][KP];
	iTermYaw[INNER][MECH] += Cloud_Palstance_Error[YAW][MECH] * Cloud_Palstance_kpid[YAW][MECH][KI] * 0.002f;
	iTermYaw[INNER][MECH]  = constrain_float(iTermYaw[INNER][MECH], -PID_Iterm_Max, PID_Iterm_Max);
	
	pidTermYaw[INNER][MECH] = pTermYaw[INNER][MECH] + iTermYaw[INNER][MECH];
	pidTermYaw[INNER][MECH] = constrain_float(pidTermYaw[INNER][MECH], -PID_Out_Max, PID_Out_Max);
}

/**
  * @brief  yaw������ģʽ
  * @param  void
  * @retval void
  * @attention ����PID�����
  */
void vYaw_Gyro_PositionLoop(void)
{
	//�Ƕ����
	Cloud_Angle_Error[YAW][GYRO] = Cloud_Angle_Target[YAW][GYRO] - Cloud_Angle_Measure[YAW][GYRO];

	//�����п������˲�,������Ƶ�ͷ��ȶ���
	Cloud_Angle_Error[YAW][GYRO] = KalmanFilter(&Gimbal_Yaw_Gyro_Error_Kalman, Cloud_Angle_Error[YAW][GYRO]);

	//�⻷���
	pTermYaw[OUTER][GYRO] = Cloud_Angle_Error[YAW][GYRO] * Cloud_Angle_kpid[YAW][GYRO][KP];
	pidTermYaw[OUTER][GYRO] = pTermYaw[OUTER][GYRO];
	pidTermYaw[OUTER][GYRO] = constrain_float(pidTermYaw[OUTER][GYRO],-PID_Outter_Max,PID_Outter_Max);
	
	//���ٶ����
	Cloud_Palstance_Error[YAW][GYRO] = pidTermYaw[OUTER][GYRO] - Cloud_Palstance_Measure[YAW][GYRO];
	//�ڻ����
	pTermYaw[INNER][GYRO]  = Cloud_Palstance_Error[YAW][GYRO] * Cloud_Palstance_kpid[YAW][GYRO][KP];
	iTermYaw[INNER][GYRO] += Cloud_Palstance_Error[YAW][GYRO] * Cloud_Palstance_kpid[YAW][GYRO][KI] * 0.002f;
	iTermYaw[INNER][GYRO]  = constrain_float(iTermYaw[INNER][GYRO], -PID_Iterm_Max, PID_Iterm_Max);
	
	pidTermYaw[INNER][GYRO] = pTermYaw[INNER][GYRO] + iTermYaw[INNER][GYRO];
	pidTermYaw[INNER][GYRO] = constrain_float(pidTermYaw[INNER][GYRO], -PID_Out_Max, PID_Out_Max)*YAW_POSITION;
}

/****************************��������***********************************************/

/**
  * @brief �ٽ�ֵ�ṹ���ʼ��
  * @param  critical:�ٽ�ֵ�ṹ��ָ��
  *    get:��ǰ��ȡ���ĽǶȣ������ǽǻ��е�Ƕȣ�
  * @retval void
  */
void Critical_Handle_Init(Critical_t *critical, float get)
{
	
	critical->AngleSum = get;//0;
	critical->CurAngle = get;
	critical->LastAngle = get;
	
	Cloud_Angle_Target[YAW][GYRO] = get; 

}

float Gimbal_Yaw_Gryo_AngleSum(Critical_t *critical, float get)
{
	critical->CurAngle = get - critical->LastAngle;	//��ǰ�����ǽǶȼ�ȥ��һ�ζ�ȡ�������ǽǶȣ���Ϊ��Ĳο�����ֵ
	critical->LastAngle = get;
	/*  �ٽ紦����֤ÿ�ξ�����㷴���Ƕȶ���������  */
	if(critical->CurAngle < -3600)		//ע��˴��������ǽǶȷŴ����ٽ�ֵ
		critical->CurAngle += 7200;
	if(critical->CurAngle > 3600)
		critical->CurAngle -= 7200;
	critical->AngleSum += critical->CurAngle;	
	
	return critical->AngleSum;				//�����µķ����Ƕȣ���ΪPID�㷨�ķ���ֵ
}

/**
  * @brief  ����YAWƫ�����ĽǶ�,���̸���ģʽ��
  * @param  void
  * @retval sAngleError,ƫ��Ƕ�ֵ,CAN�����Ļ�е�Ƕ�
  */
int16_t GIMBAL_GetOffsetAngle(void)
{
	int16_t sAngleError = 0;

	sAngleError = (Cloud_Angle_Measure[YAW][MECH] - Mech_Mid_Yaw)*YAW_POSITION;


	//���㴦��,ͳһ���ӻ�
	if (sAngleError > 8192 / 2)
	{
		return (sAngleError - 8192) ;
	}
	else if (sAngleError < -8192 / 2)
	{
		return (sAngleError + 8192);
	}
	else
	{
		return  sAngleError;
	}
}

/**
  * @brief  �ȴ�Pitch��ˮƽ
  * @param  void
  * @retval 1�ص�ˮƽλ��,0δ�ص�ˮƽλ��
  * @attention �Ƕ�С��50����Ϊ�ص�ˮƽ
  */
uint8_t GIMBAL_IfPitchLevel(void)
{	  
	if ( fabs( Cloud_Angle_Measure[PITCH][MECH] - Mech_Mid_Pitch ) <= 50 )
	{
        return 1;
	}
		
	return 0;
}

/**
  * @brief  PITCḨͷ�ж�
  * @param  void
  * @retval �Ƿ�̧ͷ
  * @attention ����Ħ���ֿ���
  */
uint8_t GIMBAL_IfPitchHigh(void)
{
	if (fabs(Cloud_Angle_Measure[PITCH][MECH] - CLOUD_FRIC_PIT_UP) <= 20)
	{
		return 1;
	}

	return 0;
}

/**
  * @brief  �Ƿ��ڿ�����ģʽ
  * @param  void
  * @retval TRUE/FALSE
  * @attention ����ֹͣŤ��ģʽ��45��Ե�ģʽ
  */
bool GIMBAL_IfGIMBAL_LEVEL(void)
{
	if (actGimbal == GIMBAL_LEVEL)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/**
  * @brief  �Ƿ������ģʽ
  * @param  void
  * @retval TRUE����  FALSE�ر�
  * @attention 
  */
bool GIMBAL_IfBuffHit(void)
{
    if (actGimbal == GIMBAL_BUFF || actGimbal == GIMBAL_SM_BUFF|| actGimbal == GIMBAL_MANUAL)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  �Ƿ�������ģʽ
  * @param  void
  * @retval TRUE����  FALSE�ر�
  * @attention 
  */
bool GIMBAL_If_Big_Buff(void)
{
    if (actGimbal == GIMBAL_BUFF)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  �Ƿ�����С��ģʽ
  * @param  void
  * @retval TRUE����  FALSE�ر�
  * @attention 
  */
bool GIMBAL_If_Small_Buff(void)
{
    if (actGimbal == GIMBAL_SM_BUFF)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  �Ƿ�������ģʽ
  * @param  void
  * @retval TRUE����  FALSE�ر�
  * @attention 
  */
bool GIMBAL_If_Base(void)
{
    if (actGimbal == GIMBAL_BASE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  �Ƿ����ֶ����ģʽ
  * @param  void
  * @retval TRUE����  FALSE�ر�
  * @attention 
  */
bool GIMBAL_IfManulHit(void)
{
    if (actGimbal == GIMBAL_MANUAL)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  �Ƿ�������
  * @param  void
  * @retval TRUE����  FALSE�ر�
  * @attention 
  */
bool GIMBAL_IfAutoHit(void)
{
    if(actGimbal == GIMBAL_AUTO)//����Ҽ�����
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  ����yaw��Ԥ���Ƿ��Ѿ�����
  * @param  void
  * @retval TRUE����  FALSE�ر�
  * @attention 
  */
bool GIMBAL_IfAuto_MobPre_Yaw(void)
{
    if(actGimbal == GIMBAL_AUTO)//����Ҽ�����
	{
		return Mobility_Prediction_Yaw;//TRUE/FALSE
	}
	else//û�����鲻������Ԥ��
	{
		return FALSE;
	}
}

/**
  * @brief  yaw�Ὺ��Ԥ���ʱ����̨�Ƿ�λ
  * @param  void
  * @retval TRUE��λ�ɴ�   FALSEû��λ��ֹ��
  * @attention ���Ҹ����ӳ٣�����ʱ�ǵ����㷴��;�ֹʱ���ӳ�
  */
bool GIMBAL_MOBPRE_YAW_FIRE(void)
{
	return Mobi_Pre_Yaw_Fire;
}

/**
  * @brief  ���yaw�Ƿ��ƶ���λ
  * @param  void
  * @retval TRUE��λ�ɴ�   FALSEû��λ��ֹ��
  * @attention 
  */
float debug_y_ready = 30;
float debug_pix_y = 0;
bool GIMBAL_BUFF_YAW_READY(void)
{
	debug_pix_y = fabs(gb_yaw_angle_gim + Buff_Yaw_Comp_Gimbal);
	if( (fabs(gb_yaw_posit_error) < debug_y_ready) 
			&& (VisionRecvData.yaw_angle != 0) 
				&& fabs(gb_yaw_angle_gim + Buff_Yaw_Comp_Gimbal) <= 35 )//(VisionRecvData.identify_buff == TRUE) )//ʶ����Ŀ��
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  ���pitch�Ƿ��ƶ���λ
  * @param  void
  * @retval TRUE��λ�ɴ�   FALSEû��λ��ֹ��
  * @attention 
  */
float debug_p_ready = 30;
float debug_pix_p = 0;
bool GIMBAL_BUFF_PITCH_READY(void)
{
	debug_pix_p = fabs(gb_pitch_angle_gim + Buff_Pitch_Comp_Gimbal);
	if( (fabs(gb_pitch_posit_error) < debug_p_ready)
			&& (VisionRecvData.pitch_angle != 0) 
				&& fabs(gb_pitch_angle_gim + Buff_Pitch_Comp_Gimbal) <= 35)//(VisionRecvData.identify_buff == TRUE) )//ʶ����Ŀ��
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  �Ƿ��������ڱ�
  * @param  void
  * @retval TRUE   FALSE
  * @attention ����̧ͷ�Ƕ�̫������Ϊ�ڴ��ڱ�
  */
bool GIMBAL_AUTO_PITCH_SB(void)
{
	if( Cloud_Angle_Measure[PITCH][MECH] - Mech_Min_Pitch <= down_sb_pitch/*300*/ 
			|| IF_KEY_PRESSED_G)//̧ͷ�ӽ���λ
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  �Ƿ����еȾ��������ڱ�,�Ӵ�Ԥ��
  * @param  void
  * @retval TRUE   FALSE
  * @attention ����̧ͷ�Ƕ�̫������Ϊ�ڴ��ڱ�
  */
float pitch_sb_error = 0;
bool GIMBAL_AUTO_PITCH_SB_SK(void)
{
	pitch_sb_error = Cloud_Angle_Measure[PITCH][MECH] - Mech_Min_Pitch;
	if( (Cloud_Angle_Measure[PITCH][MECH] - Mech_Min_Pitch <= down_sb_pitch/*450*//*550*/)
			&& (Cloud_Angle_Measure[PITCH][MECH] - Mech_Min_Pitch > up_sb_pitch) )//̧ͷ�ӽ���λ
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/**
  * @brief  ��̨̧ͷ�Ƕ�
  * @param  void
  * @retval ŷ����
  * @attention ����͵�Ϊ0
  */
float GIMBAL_PITCH_Judge_Angle(void)
{
	float angle_pitch = 180;
	
	//����ŷ����
	angle_pitch = -(Cloud_Angle_Measure[PITCH][MECH] - Mech_Max_Pitch)*1;///8192*360.f;
	
	return angle_pitch;
}

/**
  * @brief  ��̨����Ƕ�
  * @param  void
  * @retval pitch_mech_angle
  * @attention 
  */
int Base_Angle_Measure(void)
{
	return (int)Cloud_Angle_Measure[PITCH][MECH];
}

