#include "Task_Revolver.h"


#include "can2.h"
#include "control.h"
#include "remote.h"
#include "Task_Gimbal.h"

#include "judge.h"
#include "friction.h"
#include "vision.h"
#include "super_cap.h"
#include "led.h"


/*****************����Ƶ���뽵�����٣����������Ħ�������***********************/


#define REVOL_SPEED_RATIO   2160       //�����һ��תһȦ,2160ת��ת��,60*36,����Ƶ�ٳ��Բ��̸����Ϳɵ���Ӧ��Ƶ�µ�ת��

/*       �����ͺ�Ԥ����       */
#define SEVEN_REVOLVER    0    	//7����
#define EIGHT_REVOLVER    1		//8����
#define TEN_REVOLVER      2		//10����
#define TWELVE_REVOLVER   3		//12����

#define REVOLVER_CHOOSE  TWELVE_REVOLVER	//ѡ�����ͺ�

#if REVOLVER_CHOOSE == EIGHT_REVOLVER			
	#define		REVOL_SPEED_GRID      8				//���̸���
	#define 	AN_BULLET         (36864.0f)		//�����ӵ����λ������ֵ

#elif REVOLVER_CHOOSE == SEVEN_REVOLVER
	#define 	REVOL_SPEED_GRID      7				//���̸���
	#define   	AN_BULLET     	  (42130.2857f)		//�����ӵ����λ������ֵ

#elif REVOLVER_CHOOSE == TEN_REVOLVER
	#define 	REVOL_SPEED_GRID  	  10			//���̸���
	#define    	AN_BULLET     	  (29491.2f)		//�����ӵ����λ������ֵ

#elif REVOLVER_CHOOSE == TWELVE_REVOLVER
	#define 	REVOL_SPEED_GRID      12			//���̸���
	#define    	AN_BULLET         (24576.0f)		//�����ӵ����λ������ֵ

#endif	

/******����,�����߼�����̨����*********/

//���̵��ģʽ,λ�û����ٶȻ�
//#define    REVOL_LOOP_POSI     0
//#define    REVOL_LOOP_SPEED    1
//uint16_t   Revolver_mode;//����ģʽѡ��
//���̵��ģʽ,λ�û����ٶȻ�
typedef enum
{
	REVOL_POSI_MODE  = 0,
	REVOL_SPEED_MODE = 1,
}eRevolverCtrlMode;
eRevolverCtrlMode Revolver_mode;

typedef enum
{
	SHOOT_NORMAL       =  0,//���ģʽѡ��,Ĭ�ϲ���
	SHOOT_SINGLE       =  1,//����
	SHOOT_TRIPLE       =  2,//������
	SHOOT_HIGHTF_LOWS  =  3,//����Ƶ������
	SHOOT_MIDF_HIGHTS  =  4,//����Ƶ������
	SHOOT_BUFF         =  5,//���ģʽ
	SHOOT_AUTO         =  6,//�����Զ����
}eShootAction;
eShootAction actShoot;
#define    REVOL_CAN_OPEN    350  //Ħ����ʵ���ٶȳ������ֵ��������ת��,����Ħ������СĿ���ٶ����ı�


/*******************���̲���**********************/
//���̲����ٶ�
int16_t Revolver_Speed_Measure;

//���̲����Ƕ�
int16_t Revolver_Angle_Measure;

//�����ٶ����
float Revolver_Speed_Error;

//���̽Ƕ����
float Revolver_Angle_Error[2];//  inner/outer

//�ۼƺ�
float Revolver_Angle_Measure_Sum;//���̲����Ƕ��ۼƺ�,����λ��PID
int16_t Revolver_Angle_Measure_Prev;//�ϴ�ʣ�µ��ۼӺͽǶ�,����Ȧ�������ж�

//����Ŀ��Ƕ�
float  Revolver_Angle_Target;

//����Ŀ��Ƕ��ۼƺ�,����λ��PID����
float  Revolver_Angle_Target_Sum;
float  Revolver_Buff_Target_Sum;//���ģʽ�µ�Ŀ��ֵ�ݴ棬��ʱĿ��Ƕ���б�´���
float  Revolver_Buff_Ramp = AN_BULLET/40;//40msתһ��,һ�����ܳ���50ms

//����Ŀ��ת��
float  Revolver_Speed_Target;//ת�ٹ������׿���,������ת����6000

//���̵�������,������ʱ��
float Revolver_Final_Output;

/****************��Ƶ����******************/
#define SHOOT_LEFT_TIME_MAX  150	//��������л����

//�����ٶȻ���Ƶ
int16_t Revolver_Freq;

//λ�û�������,ʵʱ�ɱ�,��ֵԽСλ�û�������Խ��
uint32_t Shoot_Interval = 0;

//����������Ӧ��ģʽ�л�ʱ��ʱ���óɵ�ǰʱ��
uint32_t  Revol_Posit_RespondTime = 0;

////���������
//uint32_t Shoot_Buff_Interval = TIME_STAMP_400MS;

////�����Զ�����������
//uint32_t Shoot_Auto_Interval = TIME_STAMP_1000MS;

/*****************PID����*****************/
float pTermRevolSpeed, iTermRevolSpeed;	
float pTermRevolAngle[2], iTermRevolAngle[2];//  inner/outer
float Revolver_Speed_kpid[3];//	kp/ki/kd
float Revolver_Angle_kpid[2][3];//  inner/outer    kp/ki/kd


/**********�޷�*************/
//��������޷�
float Revolver_Output_Max;
float iTermRevolSpeedMax;//�ٶȻ�΢���޷�
float iTermRevolPosiMax;//λ�û�΢���޷�



/********���**********/
//�����ӵ���,��һ�¼�һ��,��һ�ż�һ��
int16_t Key_ShootNum;//����������
int16_t ShootNum_Allow = 0;//��ʣ���ŵ����Դ�
//int16_t Residue_ShootNum = 0;//ʣ��ɷ�����
uint16_t Residue_Heat;//ʣ���������,�������ƿ���
uint16_t Shoot_HeatLimit;//��ǰ�ȼ������������
uint16_t Shoot_HeatIncSpeed;//��ǰĦ����ת���µ����ӵ���������ֵ

/************����************/
#define Stuck_Revol_PIDTerm   4000      //PID����������������Ϊ�п��ܿ���
#define Stuck_Speed_Low       60       //�����ٶȵ��������,����Ϊ�п��ܿ���

#define Stuck_SpeedPID_Time   100       //�ٶ����� ms��С,PID����  ms����
#define Stuck_TurnBack_Time   100       //��תʱ��,ʱ��Խ������Խ��
uint32_t Stuck_Speed_Sum = 0;//���㿨������,�ٶȻ�
uint32_t Stuck_Posit_Sum = 0;//���㿨������,λ�û�

portTickType posishoot_time;//�����ʱ����

/*����*/
uint8_t Revol_Switch_Left = 0;
u8 	 Revol_Key_Left_Change = 0;

uint8_t First_Into_Buff = FALSE;
uint8_t Buff_Shoot_Begin = FALSE;
bool 	buff_fire = 0;
bool	buff_change_fire = 0;
/************************************************************************************/
/************************************************************************************/

uint8_t revol_remot_change = TRUE;
void Task_Revolver(void *pvParameters)
{
	portTickType currentTime;	
	
	for(;;)
	{	
		currentTime = xTaskGetTickCount();//��ǰϵͳʱ��
		CAP_Ctrl();//���ݳ�ŵ���ƺ���,ʱ������⣬���Զ�������
		
		if (SYSTEM_GetSystemState() == SYSTEM_STARTING)
		{
			REVOLVER_Rest();
			REVOLVER_InitArgument();
		}
		else
		{
			if (SYSTEM_GetRemoteMode() == RC)
			{
				REVOLVER_Rc_Ctrl();		
			}
			else
			{
				REVOLVER_Key_Ctrl();
				revol_remot_change = TRUE;
			}
		}
		
		//��������
		if(Revolver_Heat_Limit() == FALSE //һ��Ҫ�����ٶȡ�λ�ÿ���֮ǰ
			&& GIMBAL_IfBuffHit() != TRUE 
				&& GIMBAL_IfManulHit() != TRUE)//���ʱ��������
		{
			REVOLVER_Rest();//��������,�������������Ҳ��������
		}
		
		if(Revolver_mode == REVOL_SPEED_MODE)
		{
			REVOL_SpeedLoop();
		}
		else if(Revolver_mode == REVOL_POSI_MODE)
		{
			REVOL_PositionLoop();
		}
		
		if(Fric_GetSpeedReal() > REVOL_CAN_OPEN)//Ħ���ֿ���
		{
			REVOLVER_CANbusCtrlMotor();
		}
		else
		{
			Revolver_Speed_Target = 0;//Ħ���ֹر�,���̲�����
			Revolver_Angle_Rest();//Ħ���ֹرգ��������ڼ�Ĵ�ָ��
			REVOL_SpeedLoop();
			REVOLVER_CANbusCtrlMotor();
		}

		vTaskDelayUntil(&currentTime, TIME_STAMP_1MS);//������ʱ
	}
}
	


/********************************************************************************/
/**
  * @brief  ����ʧ�ر���
  * @param  void
  * @retval void
  * @attention ���������0
  */
void REVOLVER_StopMotor(void)
{
	Revolver_Final_Output = 0;
	CAN2_Revolver_QueueSend(Revolver_Final_Output);
}

/**
  * @brief  ���̲�����ʼ��
  * @param  void
  * @retval void
  * @attention 
  */
void REVOLVER_InitArgument(void)
{
	/* Ŀ��ֵ */
	Revolver_Final_Output = 0;
	Revolver_Speed_Target = 0;
	
	/* PID���� */
	  //�ٶȻ�
	Revolver_Speed_kpid[KP] = 15;
	Revolver_Speed_kpid[KI] = 0;
	Revolver_Speed_kpid[KD] = 0;
	  //λ�û�
	Revolver_Angle_kpid[OUTER][KP] = 0.4;
	Revolver_Angle_kpid[OUTER][KI] = 0;
	Revolver_Angle_kpid[OUTER][KD] = 0;
	Revolver_Angle_kpid[INNER][KP] = 6;
	Revolver_Angle_kpid[INNER][KI] = 0;
	Revolver_Angle_kpid[INNER][KD] = 0;
	
	/* �޷� */
	iTermRevolSpeedMax  = 250;
	iTermRevolPosiMax   = 2500;
	Revolver_Output_Max = 9999;
	
	/* ��� */
	Key_ShootNum    = 0;
	Shoot_HeatLimit = 240;//������ʼ��
	Revolver_Freq   = 0;//��Ƶ��ʼ��
	
	/* λ�û�Ŀ��Ƕ� */
	Revolver_Angle_Target_Sum = Revolver_Angle_Measure;//������0,�����ϵ�ᷴת
	Revolver_Buff_Target_Sum  = Revolver_Angle_Measure;
}

/**
  * @brief  ��������
  * @param  void
  * @retval void
  * @attention ǹ�ڳ���������
  */
void REVOLVER_Rest(void)
{
	Key_ShootNum = 0;//λ�û���������
	Revolver_Speed_Target = 0;//�ٶȻ�ֹͣת��
	
	//�ٶȻ�λ������
	Revolver_Angle_Target_Sum   = Revolver_Angle_Measure;//λ�û�Ŀ��Ƕ�����
	Revolver_Angle_Measure_Sum  = Revolver_Angle_Measure;//λ�û�ת���Ƕ�����
	Revolver_Angle_Measure_Prev = Revolver_Angle_Measure;//�ϴ�λ������
	Revolver_Buff_Target_Sum 	= Revolver_Angle_Measure;
	
	//PID��������
	iTermRevolSpeed = 0;
	iTermRevolAngle[INNER] = 0;
}

/**
  * @brief  ���̽Ƕ�����
  * @param  void
  * @retval void
  * @attention ģʽ�л�ʱ��,��ֹ�´��л�ȥ��ͻȻ��һ��
  */
void Revolver_Angle_Rest(void)
{
	Key_ShootNum = 0;
	Revolver_Angle_Target_Sum   = Revolver_Angle_Measure;//λ�û�Ŀ��Ƕ�����
	Revolver_Angle_Measure_Sum  = Revolver_Angle_Measure;//λ�û�ת���Ƕ�����
	Revolver_Angle_Measure_Prev = Revolver_Angle_Measure;//�ϴ�λ������
	Revolver_Buff_Target_Sum 	= Revolver_Angle_Target_Sum;
}

/**
  * @brief  ���̵�ң��ģʽ
  * @param  void
  * @retval void
  * @attention ң�����ٶȻ�
  */
void REVOLVER_Rc_Ctrl(void)
{
	/**************������*******************/
//	Revolver_mode = REVOL_SPEED_MODE;
//	
//	if(IF_RC_SW1_DOWN)//sw1�´�
//	{
//		if (IF_RC_SW2_DOWN)//��еģʽ
//		{
//			Revolver_Freq = 10;//��Ƶѡ��
//			//�ٶȻ�ת������
//			Revolver_Speed_Target = REVOL_SPEED_RATIO/REVOL_SPEED_GRID*Revolver_Freq;
//		}
//		else if (IF_RC_SW2_MID)//������ģʽ
//		{
//			Revolver_Freq = 14;//��Ƶѡ��
//			//�ٶȻ�ת������
//			Revolver_Speed_Target = REVOL_SPEED_RATIO/REVOL_SPEED_GRID*Revolver_Freq;
//		}
//		else	//����ģʽ,�ٶȻ�������
//		{
//			Revolver_Speed_Target = 0;
//			Revolver_Freq = 0;
//		}
//	}
//	else	//ң��ģʽ�رղ���
//	{
//		Revolver_Speed_Target = 0;
//		Revolver_Freq = 0;
//	}

//	REVOL_SpeedStuck();//�����жϼ���ת
	/********************************************/
	
	/*******************�����**********************/
	
	if(IF_RC_SW2_MID)//��еģʽ�µ�����������Ե���
	{
		Revolver_mode = REVOL_POSI_MODE;//λ�û�
		if(REVOLVER_Rc_Switch() == TRUE)
		{
			Key_ShootNum++;//��һ��
		}
		
		if(revol_remot_change == TRUE)//�մӼ���ģʽ�л���������շ�������
		{
			revol_remot_change = FALSE;
			Revolver_Angle_Rest();//��ֹͻȻ�Ӽ����е�ң�ؿ�ת
		}
		
		if(Key_ShootNum != 0)
		{
			Key_ShootNum--;
			Revolver_Buff_Target_Sum += AN_BULLET;
		}
		
		if(Revolver_Angle_Target_Sum != Revolver_Buff_Target_Sum)//����ת��ȥ
		{
			Revolver_Angle_Target_Sum = RAMP_float(Revolver_Buff_Target_Sum, Revolver_Angle_Target_Sum, Revolver_Buff_Ramp);
		}
		
		REVOL_PositStuck();//�����жϼ���ת
	}
	/**************������*******************/
	else if(IF_RC_SW2_DOWN)//������ģʽ
	{
		Revolver_mode = REVOL_SPEED_MODE;//�ٶȻ�
		
		if(IF_RC_SW1_DOWN)//sw1�´�
		{
			Revolver_Freq = 20;//��Ƶѡ��
			//�ٶȻ�ת������
			Revolver_Speed_Target = REVOL_SPEED_RATIO/REVOL_SPEED_GRID*Revolver_Freq;
		}
		else	//ң��ģʽ�رղ���
		{
			Revolver_Speed_Target = 0;
			Revolver_Freq = 0;
		}

		REVOL_SpeedStuck();//�����жϼ���ת
	}
	/********************************************/
}

/**
  * @brief  ����ң�ش�
  * @param  void
  * @retval void
  * @attention 
  */
#define REVOL_STEP0    0		//ʧ�ܱ�־
#define REVOL_STEP1    1		//SW1��λ��־
#define REVOL_STEP2    2		//���ֿ��ر�־
uint8_t	Revolver_Switch = 0;//����ң��ģʽ���ر�־λת��
bool REVOLVER_Rc_Switch(void)
{
//	if (IF_RC_SW2_MID || IF_RC_SW2_DOWN)//��е��������ģʽ
	if (IF_RC_SW2_MID)//��еģʽ
	{
		if (IF_RC_SW1_DOWN)
		{
			if (Revolver_Switch == REVOL_STEP1)
			{
				Revolver_Switch = REVOL_STEP2;
			}
			else if (Revolver_Switch == REVOL_STEP2)
			{
				Revolver_Switch = REVOL_STEP0;
			}
		}
		else		
		{
			Revolver_Switch = REVOL_STEP1;
		}
	}
	else
	{
		Revolver_Switch = REVOL_STEP0;
	}
	
	if (Revolver_Switch == REVOL_STEP2)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*******����ģʽ************/

/**
  * @brief  ���̵ļ���ģʽ
  * @param  void
  * @retval void
  * @attention ������λ�û�����
  */
void REVOLVER_Key_Ctrl(void)
{
	Revolver_mode = REVOL_POSI_MODE;//��ֹ������,Ĭ��λ�û�
	
	SHOOT_NORMAL_Ctrl();//ȷ�����ģʽ
	
	/*- ȷ�������������ģʽ -*/
	switch(actShoot)
	{
		case SHOOT_NORMAL:
			//���ģʽѡ��,Ĭ�ϲ���
			SHOOT_NORMAL_Ctrl();
		break;
		
		case SHOOT_SINGLE:
			//��һ���������,��������
			SHOOT_SINGLE_Ctrl();
		break;
		
		case SHOOT_TRIPLE:
			//��������
			SHOOT_TRIPLE_Ctrl();
//			SHOOT_MIDF_HIGHTS_Ctrl();
		break;
		
		case SHOOT_HIGHTF_LOWS:
			//B����Ƶ
			SHOOT_HIGHTF_LOWS_Ctrl();
		break;
		
		case SHOOT_MIDF_HIGHTS:
			//Z�Ƽ�
			SHOOT_MIDF_HIGHTS_Ctrl();
		break;
		
		case SHOOT_BUFF:
			//����Զ���
			Revolver_mode = REVOL_POSI_MODE;
		
			#if 	BUFF_CAM_TYPE == BUFF_CAM_CHAS
				SHOOT_BUFF_Ctrl();
		
			#elif 	BUFF_CAM_TYPE == BUFF_CAM_GIMB
				SHOOT_BUFF_Ctrl_Gimbal();

			#endif
		break;
		
		case SHOOT_AUTO:
			//�Ҽ�����ʱ�Զ���
//			SHOOT_AUTO_Ctrl();
		break;		
	}
	
	/*- ��ʼ����,������ -*/
	if(Revolver_mode == REVOL_SPEED_MODE && Fric_GetSpeedReal() > REVOL_CAN_OPEN)
	{
		REVOLVER_KeySpeedCtrl();
	}
	else if(Revolver_mode == REVOL_POSI_MODE && Fric_GetSpeedReal() > REVOL_CAN_OPEN)
	{
		REVOLVER_KeyPosiCtrl();
	}
}

/************************���̼���ģʽ����ģʽС����****************************/
/**
  * @brief  ����ģʽ�·���ģʽѡ��
  * @param  void
  * @retval void
  * @attention  ��ͨģʽ�������,�Ҳ�����
  */
void SHOOT_NORMAL_Ctrl(void)
{
	static uint32_t shoot_left_time = 0;//�����������ʱ��,ʱ������л�������
	
	/*------ ���̧�����ܴ���һ�� -------*/
	if (IF_MOUSE_PRESSED_LEFT)
	{
		if (Revol_Switch_Left == 1)
		{
			Revol_Switch_Left = 2;
		}
		else if (Revol_Switch_Left == 2)
		{
			Revol_Switch_Left = 0;
		}
	}
	else if(!IF_MOUSE_PRESSED_LEFT)		
	{
		Revol_Switch_Left = 1;
		shoot_left_time = 0;//������¼�ʱ
	}
	/*------------------------------------*/
	
	if(IF_MOUSE_PRESSED_LEFT &&	shoot_left_time <= SHOOT_LEFT_TIME_MAX	//�������
			&& !IF_KEY_PRESSED_B && !IF_KEY_PRESSED_Z /*&& !GIMBAL_IfAutoHit()*/)
	{
		Revolver_mode = REVOL_POSI_MODE;//λ�û���
		shoot_left_time++;//�жϳ���,�л�
		actShoot = SHOOT_SINGLE;	
	}
	else if(IF_MOUSE_PRESSED_LEFT && shoot_left_time > SHOOT_LEFT_TIME_MAX	//��������200ms
				&& !IF_KEY_PRESSED_B && !IF_KEY_PRESSED_Z /*&& !GIMBAL_IfAutoHit()*/)
	{
		shoot_left_time++;
		Revolver_mode = REVOL_POSI_MODE;
		actShoot = SHOOT_TRIPLE;//����ģʽ
	}
	else if(IF_KEY_PRESSED_B	//����Ƶ������
				&& !IF_MOUSE_PRESSED_LEFT && !IF_KEY_PRESSED_Z /*&& !GIMBAL_IfAutoHit()*/)
	{
		Revolver_mode = REVOL_POSI_MODE;
		actShoot = SHOOT_HIGHTF_LOWS;
		shoot_left_time = 0;
	}
	else if(IF_KEY_PRESSED_Z	//����Ƶ��������,�Ƽ�ר��
				&& !IF_MOUSE_PRESSED_LEFT && !IF_KEY_PRESSED_B)
	{
		Revolver_mode = REVOL_POSI_MODE;
		actShoot = SHOOT_MIDF_HIGHTS;
		shoot_left_time = 0;
	}
//	else if(GIMBAL_IfAutoHit() == TRUE /*&& VisionRecvData.centre_lock==TRUE*/  //�Ӿ�˵���Դ���
//				 && !IF_KEY_PRESSED_Z && !IF_KEY_PRESSED_B)
//	{
//		Revolver_mode = REVOL_POSI_MODE;
//		actShoot = SHOOT_AUTO;
//		shoot_left_time = 0;
//	}
	else if(GIMBAL_IfBuffHit() == TRUE && GIMBAL_IfManulHit() == FALSE)//���ģʽ�ҷ��ֶ����ģʽ
	{
		Revolver_mode  = REVOL_POSI_MODE;
		actShoot = SHOOT_BUFF;
		shoot_left_time = 0;
	}
	else
	{
		actShoot = SHOOT_NORMAL;
		Shoot_Interval  = 0;//����������
		Revol_Posit_RespondTime = xTaskGetTickCount();//������Ӧ
		shoot_left_time = 0;
//		Revol_Angle_Clear();//ģʽ�л�ʱ�������
		Key_ShootNum = 0;
	}
	
	if(GIMBAL_IfBuffHit() == FALSE)//�˳��˴��ģʽ
	{
		First_Into_Buff = TRUE;	
		Buff_Shoot_Begin = FALSE;
		buff_fire = FALSE;
	}
}

/**
  * @brief  ��������
  * @param  void
  * @retval void
  * @attention  
  */
void SHOOT_SINGLE_Ctrl(void)
{
	portTickType  CurrentTime = 0;
	static uint32_t  RespondTime = 0;//��Ӧ�����ʱ
	
	CurrentTime = xTaskGetTickCount();
	
	Shoot_Interval = TIME_STAMP_1000MS/8;//���һ��8��
	
	if(RespondTime < CurrentTime
			&& Revol_Switch_Left == 2//�뵯�ֿ���ͬ��
				&& Key_ShootNum == 0)
	{
		RespondTime = CurrentTime + Shoot_Interval;
		Key_ShootNum++;
	}
}

/**
  * @brief  ��������
  * @param  void
  * @retval void
  * @attention  
  */
void SHOOT_TRIPLE_Ctrl(void)
{
/********************************************************/
//	static portTickType  CurrentTime = 0;
//	static uint32_t  RespondTime = 0;//��Ӧ�����ʱ
////	static uint32_t  Shoot_Interval_TRIPLE  =  0;//������������ʱ
//	
//	CurrentTime = xTaskGetTickCount();
//	
//	Shoot_Interval = TIME_STAMP_50MS;//��20��Ƶ��3����
//	
//	if(RespondTime < CurrentTime
//			&& Key_ShootNum == 0)
//	{
//		RespondTime = CurrentTime + TIME_STAMP_400MS;//��0.4��ȷ��һ������
//		
//		switch(JUDGE_ucGetRobotLevel())//�ȼ����������������µ�������
//		{
//			case 0://δ��������
//				Key_ShootNum = 3;//3����
//			break;
//			
//			case 1://1��
//				Key_ShootNum = 3;//3����
//			break;
//			
//			case 2://2��
//				Key_ShootNum = 4;//4����
//			break;
//			
//			case 3://3��
//				Key_ShootNum = 5;//5����
//			break;
//		}	
//	}
/********************************************************/


//	static portTickType  CurrentTime = 0;
//	static uint32_t  RespondTime = 0;//��Ӧ�����ʱ	
//	
//	CurrentTime = xTaskGetTickCount();
//	
//	if(JUDGE_usGetShootCold() <= 40)
//	{
//		Shoot_Interval = TIME_STAMP_1000MS/8;//ȷ����Ƶ
//	}
//	else if(JUDGE_usGetShootCold() <= 60 && JUDGE_usGetShootCold() > 40)
//	{
//		Shoot_Interval = TIME_STAMP_1000MS/10;//ȷ����Ƶ
//	}
//	else if(JUDGE_usGetShootCold() <= 80 && JUDGE_usGetShootCold() > 60)
//	{
//		Shoot_Interval = TIME_STAMP_1000MS/12;//ȷ����Ƶ
//	}
//	else if(JUDGE_usGetShootCold() >= 160)//ռ��ﱤ
//	{
//		Shoot_Interval = TIME_STAMP_1000MS/20;//ȷ����Ƶ
//	}
//	else
//	{
//		Shoot_Interval = TIME_STAMP_1000MS/8;//ȷ����Ƶ
//	}
//	
//	if(RespondTime < CurrentTime
//			&& Key_ShootNum == 0)
//	{
//		RespondTime = CurrentTime + Shoot_Interval;
//		Key_ShootNum++;
//	}

/*************************************************/
	Revolver_mode = REVOL_SPEED_MODE;

	if(JUDGE_usGetShootCold() <= 40)
	{
		Revolver_Freq = 8;//��Ƶѡ��
	}
	else if(JUDGE_usGetShootCold() <= 60 && JUDGE_usGetShootCold() > 40)
	{
		Revolver_Freq = 8;//10;//��Ƶѡ��
	}
	else if(JUDGE_usGetShootCold() <= 80 && JUDGE_usGetShootCold() > 60)
	{
		Revolver_Freq = 8;//12;//��Ƶѡ��
	}
	else if(JUDGE_usGetShootCold() >= 160)//ռ��ﱤ
	{
		Revolver_Freq = 14;//12;//��Ƶѡ��
	}
	else
	{
		Revolver_Freq = 8;//��Ƶѡ��
	}
	
	//�ٶȻ�ת������
	Revolver_Speed_Target = REVOL_SPEED_RATIO/REVOL_SPEED_GRID*Revolver_Freq;
}

/**
  * @brief  ����Ƶ�����ٿ���
  * @param  void
  * @retval void
  * @attention  
  */
void SHOOT_HIGHTF_LOWS_Ctrl(void)
{
	static portTickType  CurrentTime = 0;
	static uint32_t  RespondTime = 0;//��Ӧ�����ʱ	
	
	CurrentTime = xTaskGetTickCount();
	
	Shoot_Interval = TIME_STAMP_1000MS/20;//ȷ����Ƶ
	
	if(RespondTime < CurrentTime
			&& Key_ShootNum == 0)
	{
		RespondTime = CurrentTime + Shoot_Interval;
		Key_ShootNum++;
	}
}

/**
  * @brief  ����Ƶ�����ٿ���
  * @param  void
  * @retval void
  * @attention  
  */
void SHOOT_MIDF_HIGHTS_Ctrl(void)
{
	static portTickType  CurrentTime = 0;
	static uint32_t  RespondTime = 0;//��Ӧ�����ʱ	
	
	CurrentTime = xTaskGetTickCount();//��ǰϵͳʱ��
	
	Shoot_Interval = TIME_STAMP_1000MS/20;//ȷ����Ƶ
	
	if(RespondTime < CurrentTime
			&& Key_ShootNum == 0)
	{
		RespondTime = CurrentTime + Shoot_Interval;
		Key_ShootNum++;
	}
}

/**
  * @brief  �����������
  * @param  void
  * @retval void
  * @attention  
  */
void SHOOT_AUTO_Ctrl(void)
{
	static portTickType CurrentTime     = 0;
	static uint32_t RespondTime_Stop    = 0;//��Ӧ�����ʱ����ֹ
	static uint32_t RespondTime_MobiPre = 0;//��Ӧ�����ʱ���ƶ�Ԥ��
	CurrentTime = xTaskGetTickCount();

/***********************************************************************/
	if( GIMBAL_IfAuto_MobPre_Yaw() == TRUE)	//������Ԥ��			
	{
		Shoot_Interval = TIME_STAMP_1000MS/15;//TIME_STAMP_50MS;//ȷ����Ƶ
		if(GIMBAL_MOBPRE_YAW_FIRE()==TRUE				//�Լ���Ԥ�⵽��λ��
				&& RespondTime_MobiPre < CurrentTime
					&& Key_ShootNum == 0 
						&& IF_MOUSE_PRESSED_LEFT)//�������
		{
			RespondTime_MobiPre = CurrentTime + Shoot_Interval;
			Key_ShootNum ++;
		}
		else//������Ԥ�⵫Ԥ�ⲻ��λ����ֹ��
		{
			Key_ShootNum = 0;
		}
	}
	else if(GIMBAL_IfAuto_MobPre_Yaw() == FALSE)	//û��Ԥ��
	{
		Shoot_Interval = TIME_STAMP_1000MS/5;//ȷ����Ƶ
		if(GIMBAL_MOBPRE_YAW_FIRE()==TRUE		//�Լ���Ԥ�⵽��λ��
				&& RespondTime_Stop < CurrentTime
					&& Key_ShootNum == 0
						&& IF_MOUSE_PRESSED_LEFT)//�������				
		{
			RespondTime_Stop = CurrentTime + Shoot_Interval;//TIME_STAMP_500MS;//ÿ��0.5s������һ��		
//			Key_ShootNum = 3;
			Key_ShootNum ++;
		}
	}
}

/**
  * @brief  ����������,����������ͷλ�ڵ���
  * @param  void
  * @retval void
  * @attention  ÿ��500ms�ж�һ�ε�λ����λ����
  */
uint32_t buff_shoot_time = 500;
uint32_t New_Armor_Time = 0;//�ȶ�ʱ��
int buff_time = 750;//500;//1200;
int shootbuff_flag = 0;
void SHOOT_BUFF_Ctrl(void)
{
		   portTickType CurrentTime = 0;
	static uint32_t		RespondTime = 0;//��Ӧ�����ʱ
//	static uint32_t 	New_Armor_Time = 0;//�ȶ�ʱ��
	
	CurrentTime = xTaskGetTickCount();
	
	if(!IF_MOUSE_PRESSED_RIGH)//�Ҽ��ɿ��Զ���
	{
		//�ӵ��ɹ�ȥҪ600ms���ң�����600�����飬˵��û���У���һ��
		Shoot_Interval = buff_time;//TIME_STAMP_1000MS/1;//���һ��1����BUG,����REVOLVER_KeyPosiCtrl()���ʱ����
		
		if( Vision_If_Armor() == TRUE )//�ջ�װ�װ壬��һ��ʱ�������������
		{
			Vision_Clean_Ammor_Flag();//�ȴ��´θ���װ�װ�
			New_Armor_Time = 0;//װ���ȶ����¼�ʱ
			RespondTime = CurrentTime - 1;
			Revol_Posit_RespondTime = xTaskGetTickCount();//������Ӧ
		}

		if( VisionRecvData.identify_buff == TRUE || VisionRecvData.identify_buff == 2 )
		{
			New_Armor_Time++;
			if( New_Armor_Time > buff_shoot_time  //��װ�׺��ȶ�500ms�ſ�ʼ��
					&& RespondTime < CurrentTime
						&& Key_ShootNum == 0)
			{
				RespondTime = CurrentTime + Shoot_Interval;
				Key_ShootNum++;//��һ��
			}
		}
		else if(VisionRecvData.identify_buff == 0)//ûʶ��
		{
			New_Armor_Time = 0;//װ���ȶ����¼�ʱ
		}
	}
	else//��ס�Ҽ��ӹ��Զ���
	{
		/*------ ���̧�����ܴ���һ�� -------*/
		if (IF_MOUSE_PRESSED_LEFT)
		{
			if (Revol_Switch_Left == 1)
			{
				Revol_Switch_Left = 2;
			}
			else if (Revol_Switch_Left == 2)
			{
				Revol_Switch_Left = 0;
			}
		}
		else if(!IF_MOUSE_PRESSED_LEFT)		
		{
			Revol_Switch_Left = 1;
		}
		
		/*------------------------------------*/
		if(IF_MOUSE_PRESSED_LEFT)//�������
		{
			SHOOT_SINGLE_Ctrl();
		}
	}
}

/**
  * @brief  ���������ƣ�����ͷ����̨
  * @param  void
  * @retval void
  * @attention  ÿ��500ms�ж�һ�ε�λ����λ����
  */
uint32_t buff_lost_time = 0;//��֡��ʱ������ʱ�����Ϊ��֡
uint32_t buff_change_lost_time = 0;//��װ�׵�֡��ʱ
uint32_t buff_shoot_close = 0;
float buff_stamp = 800;//1100;//��0.8�벹һ��
float buff_lost_stamp = 200;//����200ms��ʧĿ��
float Armor_Change_Delay = 0;
float dy = 80;//1;//130;
void SHOOT_BUFF_Ctrl_Gimbal(void)
{
		   portTickType CurrentTime = 0;
	static uint32_t		RespondTime = 0;//��Ӧ�����ʱ
	static uint32_t lockon_time = 0;//�ȶ���׼һ��ʱ���ɻ���
	
	CurrentTime = xTaskGetTickCount();
	
	if(!IF_MOUSE_PRESSED_RIGH)//�Ҽ��ɿ��Զ���
	{	
		//��֡ͳ�ƣ���̫֡���ز���
		if(VisionRecvData.identify_buff == FALSE)//ûʶ��Ŀ��
		{
			buff_lost_time++;
			if(buff_lost_time > buff_lost_stamp)
			{
				buff_fire = FALSE;
			}
		}
		else
		{
			buff_lost_time = 0;
			buff_fire = TRUE;//���Կ���
		}
		
		Shoot_Interval = 200;//���������Ƶ,����̫���ֹ����
		
		//ʶ��Ŀ���ҽӽ�Ŀ��
		if( buff_fire == TRUE //�ǳ�ʱ���֡
				&& Vision_If_Armor() == FALSE)//��װ���л�
		{
//			buff_shoot_close = 0;//���¼���δ��׼Ŀ��ʱ��
			Armor_Change_Delay++;
			if(Armor_Change_Delay > 50)
			{
				if(GIMBAL_BUFF_YAW_READY() && GIMBAL_BUFF_PITCH_READY())
				{
					buff_change_lost_time = 0;//ˢ�µ�λ�ж�ʱ��
					buff_change_fire = TRUE;//�������л�����ȶ���ʱ
				}
				else
				{
					buff_change_lost_time++;//�ȶ���λ
					if(buff_change_lost_time > 50)//������֡50ms����Ϊû��λ
					{
						buff_change_fire = FALSE;//�����л��ȶ���ʱ
					}
				}
				
				if(buff_change_fire == TRUE)
				{
					lockon_time++;
				}
				else
				{
					lockon_time = 0;
				}
				
				if( RespondTime < CurrentTime
						&& Key_ShootNum == 0 
							&& lockon_time > dy//80
								&& (VisionRecvData.yaw_angle != 0 && VisionRecvData.pitch_angle != 0))//�ȶ���λ30ms
				{
					RespondTime = CurrentTime + buff_stamp;//Shoot_Interval;
					Key_ShootNum++;//��һ��
				}
			}
		}
		else//��ʱ���֡�����л���װ��
		{
			lockon_time = 0;
			buff_change_fire = FALSE;//�����л��ȶ���ʱ
			
			if( Vision_If_Armor() == TRUE )//�л�װ�װ�
			{
				Armor_Change_Delay = 0;
				Vision_Clean_Ammor_Flag();//�ȴ��´θ���װ�װ�
			}
			
			RespondTime = CurrentTime-1;//����ˢ�����ʱ��
			Key_ShootNum = 0;
			
//			buff_shoot_close++;
//			lockon_time = 0;
//			if(buff_shoot_close > 100)//����100msû�鵽Ŀ��
//			{
//				RespondTime = CurrentTime-1;//����ˢ�����ʱ��
//				Key_ShootNum = 0;
//			}
		}
	}
	else//��ס�Ҽ��ӹ��Զ���
	{
		/*------ ���̧�����ܴ���һ�� -------*/
		if (IF_MOUSE_PRESSED_LEFT)
		{
			if (Revol_Switch_Left == 1)
			{
				Revol_Switch_Left = 2;
			}
			else if (Revol_Switch_Left == 2)
			{
				Revol_Switch_Left = 0;
			}
		}
		else if(!IF_MOUSE_PRESSED_LEFT)		
		{
			Revol_Switch_Left = 1;
		}
		
		/*------------------------------------*/
		if(IF_MOUSE_PRESSED_LEFT)//�������
		{
			SHOOT_SINGLE_Ctrl();
		}
	}
}

/**
  * @brief  ����ģʽ�����ٶȻ�����
  * @param  void
  * @retval void
  * @attention �Ƽ�ģʽ������Ƶ,�����ǵü���һ����־λ��������Ħ���ֵ�������
  */
void REVOLVER_KeySpeedCtrl(void)
{
	REVOL_SpeedStuck();//�����жϼ���ת
}

/**
  * @brief  ����ģʽ����λ�û�����
  * @param  void
  * @retval void
  * @attention 
  */
void REVOLVER_KeyPosiCtrl(void)
{
	static portTickType  CurrentTime = 0;
//	static uint32_t  RespondTime = 0;//��Ӧ�����ʱ
	
	CurrentTime = xTaskGetTickCount();
	
	if(Key_ShootNum != 0 && Revol_Posit_RespondTime < CurrentTime)
	{
		Revol_Posit_RespondTime = CurrentTime + Shoot_Interval;
		Key_ShootNum--;//����������
		Revolver_Buff_Target_Sum += AN_BULLET;//����λ�ü�
		
		posishoot_time = xTaskGetTickCount();//����ָ���´�ʱ��ϵͳʱ��,���ڷ�����ʱ����
	}		
	
	if(Revolver_Angle_Target_Sum != Revolver_Buff_Target_Sum)//����ת��ȥ
	{
		Revolver_Angle_Target_Sum = RAMP_float(Revolver_Buff_Target_Sum, Revolver_Angle_Target_Sum, Revolver_Buff_Ramp);
	}
	
	REVOL_PositStuck();//�����жϼ���ת,��ǰ�������һ��
}

/**
  * @brief  ���Ͳ��̵���ֵ
  * @param  void
  * @retval void
  * @attention 
  */
void REVOLVER_CANbusCtrlMotor(void)
{	
	CAN2_Revolver_QueueSend(Revolver_Final_Output);
}


/*******************���̵�����ݸ���*********************/

/**
  * @brief  ��ȡ����Ƕ�
  * @param  CAN����
  * @retval void
  * @attention  CAN2�ж��е���
  */
void REVOLVER_UpdateMotorAngle(int16_t angle)
{
    Revolver_Angle_Measure = angle;
}

/**
  * @brief  ��ȡ���ת��
  * @param  CAN����
  * @retval void
  * @attention  CAN2�ж��е���
  */
void REVOLVER_UpdateMotorSpeed(int16_t speed)
{
	  Revolver_Speed_Measure = speed;
}

/**
  * @brief  ͳ��ת���Ƕ��ܺ�
  * @param  void
  * @retval void
  * @attention �л���ģʽ֮��ǵ����� 
  */
void REVOL_UpdateMotorAngleSum(void)
{		 
	//�ٽ�ֵ�жϷ�
	if (abs(Revolver_Angle_Measure - Revolver_Angle_Measure_Prev) > 4095)//ת����Ȧ
	{		
		//���β����Ƕ�С���ϴβ����Ƕ��ҹ��˰�Ȧ,��˵�����ι������
		if (Revolver_Angle_Measure < Revolver_Angle_Measure_Prev)//����Ȧ�ҹ����
		{
			//�Ѿ�ת����һȦ,���ۼ�ת�� 8191(һȦ) - �ϴ� + ����
			Revolver_Angle_Measure_Sum += 8191 - Revolver_Angle_Measure_Prev + Revolver_Angle_Measure;
		}
		else
		{
			//������һȦ
			Revolver_Angle_Measure_Sum -= 8191 - Revolver_Angle_Measure + Revolver_Angle_Measure_Prev;
		}
	}
	else      
	{
		//δ���ٽ�ֵ,�ۼ���ת���ĽǶȲ�
		Revolver_Angle_Measure_Sum += Revolver_Angle_Measure - Revolver_Angle_Measure_Prev;
	}

	//��¼��ʱ����Ƕ�,��һ�μ���ת���ǶȲ���,�����ж��Ƿ�ת��1Ȧ
	Revolver_Angle_Measure_Prev = Revolver_Angle_Measure;
}

/***********************PID����**********************/

/**
  * @brief  �ٶȻ�PID����
  * @param  void
  * @retval void
  * @attention  ң��ֻ���ٶȻ�
  */
void REVOL_SpeedLoop(void)
{  
	Revolver_Speed_Error = Revolver_Speed_Target - Revolver_Speed_Measure;

	//���͵���PID�㷨
	pTermRevolSpeed   = Revolver_Speed_Error * Revolver_Speed_kpid[KP];
	iTermRevolSpeed  += Revolver_Speed_Error * Revolver_Speed_kpid[KI];
	iTermRevolSpeed   = constrain( iTermRevolSpeed, -iTermRevolSpeedMax, iTermRevolSpeedMax );

	Revolver_Final_Output = constrain_float( pTermRevolSpeed + iTermRevolSpeed, -Revolver_Output_Max, +Revolver_Output_Max );
}

/**
  * @brief  λ�û�PID����
  * @param  void
  * @retval void
  * @attention  ����ģʽ
  */
void REVOL_PositionLoop(void)
{
	//��ȡת�����ܽǶ�ֵ
	REVOL_UpdateMotorAngleSum( );
	
	//�⻷����
	Revolver_Angle_Error[OUTER] = Revolver_Angle_Target_Sum - Revolver_Angle_Measure_Sum;
	pTermRevolAngle[OUTER] = Revolver_Angle_Error[OUTER] * Revolver_Angle_kpid[OUTER][KP];

	//�ڻ�����
	Revolver_Angle_Error[INNER]  =  pTermRevolAngle[OUTER] - Revolver_Speed_Measure;
	pTermRevolAngle[INNER]   = Revolver_Angle_Error[INNER] * Revolver_Angle_kpid[INNER][KP];		
	iTermRevolAngle[INNER]  += Revolver_Angle_Error[INNER] * Revolver_Angle_kpid[INNER][KI] * 0.001f;
	iTermRevolAngle[INNER]   = constrain_float( iTermRevolAngle[INNER], -iTermRevolPosiMax, iTermRevolPosiMax );

	Revolver_Final_Output = constrain_float( pTermRevolAngle[INNER] + iTermRevolAngle[INNER] , -Revolver_Output_Max, Revolver_Output_Max);

}


/*********************��Ƶ��������****************************/

/**
  * @brief  ǹ����������
  * @param  void
  * @retval �����Ƿ���
  * @attention  ����Ҫ����һ�²���,����ʣ��ɷ����������ջ�
  *             �����˫ǹ����˺���������
  */
bool Revolver_Heat_Limit(void)
{
	static uint16_t  usShootNumAllow  = 0;
	static uint16_t  usHeatBuffer     = 0;
	static bool_t  IfShootAllow  =  FALSE;

	static  uint16_t  usShootNumBuffer  = 0;
	static  portTickType  ulShootTimeRecord = 0;
	static  uint16_t  usShootHeatRecord = 0;
	static  uint16_t  usShootNumPrev    = 0;
	static  uint16_t  usHeatPrev        = 0;

	static  uint32_t  ulOfflineCnt      = 0;
			uint16_t  usHeatReal		= 0;
			uint16_t  usShootNumReal	= 0;
			uint16_t  usHeatOneShoot	= 30;
			uint16_t  usHeatLimit;
	static  uint32_t  ShootNumBuffer_Error = 0;//������ʱ�������Ϊ��������

	/* ��ȡ���� */
	usHeatReal = JUDGE_usGetRemoteHeat17();
	
	  
	  /* ���ߴ��� */
	if (usHeatReal == usHeatPrev)
	{
		
	}
	else
	{
		ulOfflineCnt = 0;
	}

	/* ��ȡ������� */
	usShootNumReal  =  JUDGE_usGetShootNum( );
	
	/* ֻҪ���˵������� */
	if (usShootNumReal > usShootNumPrev)
	{
		usShootNumBuffer  += usShootNumReal - usShootNumPrev;
		ulShootTimeRecord  = xTaskGetTickCount( );
		usShootHeatRecord  = usHeatReal;
	}
  	
	/* */
	usHeatOneShoot = Fric_GetHeatInc( );
	if(usHeatOneShoot <= 1)//��ֹ��������ֱ������򵯲��ᶯ
	{
		usHeatOneShoot = 30;
	}
	usHeatLimit    = JUDGE_usGetHeatLimit( );
	if(usHeatLimit <= 30)
	{
		usHeatLimit = 240;//��ֹ���ݳ���
	}

	/* ʣ������ */
	if (usHeatReal <= usHeatLimit)
	{
		usHeatBuffer = usHeatLimit - usHeatReal;
	}
	else
	{
		usHeatBuffer = 0;
	}

	if (usHeatBuffer > usHeatOneShoot)//ʣ���������ڴ�һ����������
	{
		/* ���ܴ�����ӵ���Ŀ */
		usShootNumAllow = (uint16_t)(usHeatBuffer / usHeatOneShoot);// - 1;
	}
	else
	{
		usShootNumAllow = 0;//ʣ���������ͣ�������
	}


	/**/
	if ( abs(Revolver_Speed_Measure) <= 100	
			&& xTaskGetTickCount( ) - ulShootTimeRecord > TIME_STAMP_30MS/*TIME_STAMP_100MS*/ 
				&& (usHeatReal > usShootHeatRecord + 3 || usHeatReal == 0) )
	{
		/* ���� */
		usShootNumBuffer = 0;
	}

	if (usShootNumAllow <= 1)
	{
		IfShootAllow = FALSE;
	}
	else if (usShootNumBuffer < usShootNumAllow)
	{
		IfShootAllow = TRUE;
	}
	else
	{
		IfShootAllow = FALSE;
	}
	
    /* ��¼��ʱ��������� */  
	usShootNumPrev  =  usShootNumReal;
	usHeatPrev 		=  usHeatReal;

	if(usShootNumBuffer > 6)//��ֹ��������ʱ�����������
	{
		ShootNumBuffer_Error++;
	}
	else
	{
		ShootNumBuffer_Error = 0;
	}
	
	if(ShootNumBuffer_Error > TIME_STAMP_1000MS*5)//����5�����6��
	{
		usShootNumBuffer = 0;
		ShootNumBuffer_Error = 0;
	}
	
	
	if (ulOfflineCnt < 100)
	{
		return  IfShootAllow;
	}
	else
	{
		return  TRUE;	
	}

}


/*****************************��������**************************************/

/**
  * @brief  �ٶȻ�ʽ��������
  * @param  void
  * @retval void
  * @attention  ��ס�ͷ�תn��
  */
void REVOL_SpeedStuck(void)
{
	static uint16_t  stuck_time    = 0;//������ʱ
	static uint16_t  turnback_time = 0;//��ת��ʱ
	static bool Revol_Speed_ifStuck = FALSE;//�����ж�

	if (Revol_Speed_ifStuck == TRUE)//��ȷ�Ͽ���,��ʼ��ת��ʱ
	{
		Revolver_Speed_Target = -4000;//��ת
		turnback_time++;//��תһ��ʱ��

		if (turnback_time > Stuck_TurnBack_Time)//��ת���
		{
			turnback_time  = 0;
			Revol_Speed_ifStuck = FALSE;//������ת
		}			
	}
	else
	{
		if ( abs(Revolver_Final_Output) >= Stuck_Revol_PIDTerm //PID�������
				&& abs(Revolver_Speed_Measure) <= Stuck_Speed_Low  )//�ٶȹ���
		{
			stuck_time++;//������ʱ
		}
		else
		{
			stuck_time = 0;//û�г�ʱ�俨��,��ʱ����
		}

		if (stuck_time > Stuck_SpeedPID_Time)//���˳���60ms
		{
			Stuck_Speed_Sum++;//��������,����е��Ĵ��ֵܵ�����
			stuck_time = 0;
			Revol_Speed_ifStuck = TRUE;//��ǿ��Խ��뵹ת��ʱ
		}
	}
}

/**
  * @brief  λ�û�ʽ��������
  * @param  void
  * @retval void
  * @attention  ��ס�ͷ�תn��
  */
void REVOL_PositStuck(void)
{
	static uint16_t  stuck_time      = 0;//������ʱ
	static uint16_t  turnback_time   = 0;//��ת��ʱ
	static bool Revol_Posit_ifStuck = FALSE;//�����ж�
	
	if (Revol_Posit_ifStuck == TRUE)//������ʼ��ת��ʱ
	{
		//�����������ж��Ƿ񿨵����ʱ������갴�µ�ָ�����
		Key_ShootNum = 0;
		
		turnback_time++;//��ת��ʱ,1msһ��
		if (turnback_time > Stuck_TurnBack_Time)//��תʱ�乻��
		{
			Revolver_Angle_Target_Sum = Revolver_Angle_Measure_Sum;//������ת,��ת�ر�����Ҫ��ת����λ��
			Revolver_Buff_Target_Sum = Revolver_Angle_Target_Sum;
			Revol_Posit_ifStuck = FALSE;//��Ϊ��ʱ���ٿ�����
			turnback_time = 0;//��תʱ������,Ϊ�´ε�ת��׼��	
		}
	}
	else
	{
		if ( abs(Revolver_Final_Output)  >= Stuck_Revol_PIDTerm //PID�������
				&& abs(Revolver_Speed_Measure) <= Stuck_Speed_Low  )//�ٶȹ���
		{
			stuck_time++;//ͳ�ƿ��˶೤ʱ��
		}
		else
		{
			stuck_time = 0;//������,ʱ������
		}
		
		if (stuck_time > Stuck_SpeedPID_Time)//��̫����,��ʾҪ��ת
		{
			//��ת���ܷ���Revol_Posit_ifStuck == TRUE��,����Ͳ��Ƕ�һ�ε�ת1/2����
			Revolver_Angle_Target_Sum = Revolver_Angle_Measure_Sum - AN_BULLET ;//��ת 1��  
			Revolver_Buff_Target_Sum = Revolver_Angle_Target_Sum;
			
			Stuck_Posit_Sum++;//��������,����е��Ĵ��ֵܵ�����
			stuck_time = 0;
			Revol_Posit_ifStuck = TRUE;//������ǵ�ת��ʱ����	
		}
	}
}


/***************���̸�������***************/

/**
  * @brief  ���̽Ƕ���������
  * @param  void
  * @retval void
  * @attention  ����ÿ����һ��ִ��һ��
  */
void Revol_Angle_Clear(void)
{
	Key_ShootNum = 0;//����ָ������
	
	//�ٶȻ�λ������
	Revolver_Angle_Target_Sum   = Revolver_Angle_Measure;//λ�û�Ŀ��Ƕ�����
	Revolver_Angle_Measure_Sum  = Revolver_Angle_Measure;//λ�û�ת���Ƕ�����
	Revolver_Angle_Measure_Prev = Revolver_Angle_Measure;//�ϴ�λ������
	Revolver_Buff_Target_Sum 	= Revolver_Angle_Target_Sum;
	
}

/**
  * @brief  ����ʱ���ȡ
  * @param  void
  * @retval λ�û�ʵʱָ��ʱ��
  * @attention  ���ڷ�����ʱ����
  */
portTickType REVOL_uiGetRevolTime(void)
{
	return posishoot_time;
}
