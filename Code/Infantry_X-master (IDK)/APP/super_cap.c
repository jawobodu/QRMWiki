#include "super_cap.h"

#include "control.h"
#include "adda.h"
#include "remote.h"
#include "judge.h"
#include "Task_Chassis.h"

/*************************************************************************/

typedef struct
{
	float P;
	float I;
	float D;
	float Pterm;
	float Iterm;
	float Dterm;
	float PIDterm;
	float looptime;
}exPower_t;


float getCapNowVol;
bool chargeFlag,capFlag;//����־λ �ŵ��־λ
exPower_t Power;

extern float Speed_Target_All;
extern RC_Ctl_t            RC_Ctl;
//extern extPowerHeatData_t  PowerHeatData;
extern bool Key_Flag;
extern uint8_t Climb_Mode;
/*********************/
#define charging    1 
#define notcharging 0

#define cap_giveout     1
#define cap_notgiveout  0

float getCapNowVol = 0;
float Max_Vol = 23.8;
float Min_Vol = 13.f;

/**
  * @brief  �������ݿ���
  * @param  void
  * @retval void
  * @attention  
  */
float Chass_Realtime_Power;//ʵʱ����
float Chass_Realtime_RemainEnergy;//ʵʱʣ�ཹ����
void CAP_Ctrl(void)
{
	static int32_t judgDataError_Time = 0;
	
	Chass_Realtime_RemainEnergy = JUDGE_fGetRemainEnergy();//ʣ�ཹ����
	Chass_Realtime_Power = JUDGE_fGetChassisPower();//��ȡʵʱ����
	if(SYSTEM_GetSystemState() == SYSTEM_STARTING)
	{
		SuperCap_PID_Parameter_Init();
		Dac1_Set_Vol(0);
		CAP_Charge_Off; 
		capFlag = cap_notgiveout;
	}
	else
	{
		if(JUDGE_sGetDataState() == FALSE)//����ϵͳ���ݲ�����
		{
			judgDataError_Time++;
		}
		else
		{
			judgDataError_Time = 0;
		}
		
		if(judgDataError_Time <= 250)//����ϵͳ���ݿ���
		{
			if (SYSTEM_GetRemoteMode() == RC) //ң��ģʽ
			{
				SuperCap_In();
			}
			else               //����ģʽ
			{
				if(IF_KEY_PRESSED_SHIFT)//��סshift�ŵ�
				{
					SuperCap_Out();
				}
				else
				{
					SuperCap_In();
				}
			}
		}
		else
		{
			chargeFlag = notcharging;
			capFlag = cap_notgiveout;
		}
	}
	
	if(chargeFlag == charging)
	{
		SuperCap_Power_PID();
	}
	else if(chargeFlag == notcharging)
	{
		Power.PIDterm = 0;
	}

	if(Chass_Realtime_RemainEnergy < 50)
	{
		chargeFlag = notcharging;//��ֹ�ߵ��˿�Ѫ
	}
	Dac1_Set_Vol(Power.PIDterm);
	
	if(Get_Realvoltage()>=Max_Vol)
	{
		CAP_Charge_Off;
	}
}

/**
  * @brief  �������ݺ�����ʼ��
  * @param  void
  * @retval void
  */
void SuperCap_PID_Parameter_Init(void)
{
	Power.P = 15;  Power.I = 10;   Power.D = 0; 
	Power.Pterm = 10;
	Power.Iterm = 5;
	Power.looptime = 0.02f;
}

/**
  * @brief  ��������ʧ�ر���
  * @param  void
  * @retval void
  * @attention  
  */
void Super_Cap_StopCtrl(void)
{
	Power.PIDterm=0;
	CAP_OUT_Off;
	CAP_Charge_Off;
}

/**
  * @brief  �������ݹ��ʱջ�
  * @param  void
  * @retval void
  */
extern uint8_t yunsuflag;
float cap_radio;
float debugmaxpower=75;
float realtimePower,realtimePowerError,realtimeLastPowerError;
float debugsum=8000,debugnumtoadd=10;

float last_target_all;
static portTickType remainspeedtime=0;
uint8_t just_a_flag=0;
uint8_t yunsuflag=0;
void SuperCap_Power_PID(void)
{	
	float Speed_Target_All = 0;
	Speed_Target_All = Chassis_All_Speed_Target();
	
	last_target_all = Speed_Target_All;
		
	if( last_target_all == Speed_Target_All && Speed_Target_All!=0 && just_a_flag == 0 )
	{
		remainspeedtime = xTaskGetTickCount();
		just_a_flag = 1;
	}
	else if( last_target_all == Speed_Target_All && Speed_Target_All!=0 && just_a_flag == 1 )
	{
		if( xTaskGetTickCount() - remainspeedtime > 200)//���ٱ���һ��
		{
			just_a_flag = 2;
		}
	}
	else if( last_target_all == Speed_Target_All && Speed_Target_All!=0 && just_a_flag == 2 )
	{
		yunsuflag = 1;
	}
	else 
	{
		just_a_flag = 0;
		yunsuflag = 0;
	}
		 
		 
	//�����ֹ�Ļ�
	if(Speed_Target_All == 0)
	{
		cap_radio = 1;
		Power.P = 15;
		Power.I = 10;
	}
	if(!IF_KEY_PRESSED_SHIFT)
	{
		//�˶���ʱ��
		if(yunsuflag == 0 && Speed_Target_All!=0 )//������
		{
//			cap_radio=debugsum/(float)(Speed_Target_All+debugnumtoadd);
//			cap_radio=constrain(cap_radio,0,1);
			cap_radio = 1;
			Power.P = 15;
			Power.I = 10;
		}
		else if(yunsuflag == 1 && Speed_Target_All!=0 )//�����˶�
		{
//			cap_radio=debugsum/(float)(Speed_Target_All+debugnumtoadd) * 2;
//			cap_radio=constrain(cap_radio,0,1);
			cap_radio = 1;
			Power.P = 11;
			Power.I = 8;
		}
	}
	else if(IF_KEY_PRESSED_SHIFT)
	{
		cap_radio = 1;
		Power.P = 15;
		Power.I = 10;
	}
		
	realtimePower 				 = Chass_Realtime_Power;
	//P
	realtimePowerError		 = debugmaxpower - realtimePower;
	Power.Pterm 	 = realtimePowerError * Power.P;
	//I
	Power.Iterm 	+= realtimePowerError * Power.I * Power.looptime;
	Power.Iterm	 = constrain_float(Power.Iterm,-2000,+2000);

	//PID
	Power.PIDterm = (Power.Pterm + Power.Iterm ) *cap_radio;		
	

	if(Chass_Realtime_RemainEnergy<60)
	{
		Power.PIDterm=0;
		Power.Iterm = 0;
		chargeFlag = notcharging;
	}		
	Power.PIDterm= constrain_float(Power.PIDterm/3.f,0,1000);
	Power.PIDterm = 1270 + Power.PIDterm/2;
	Power.PIDterm = constrain_float(Power.PIDterm, 1270, 1505);//1570);
}


/**
  * @brief  ������
  * @param  void
  * @retval void
  */
void Super_Charging_Control(void)
{
	getCapNowVol = Get_Realvoltage();

	 if(chargeFlag == charging && getCapNowVol<Max_Vol)
	{
		CAP_Charge_On;	 
	}
	else if(chargeFlag == notcharging || getCapNowVol>=Max_Vol)
	{
		CAP_Charge_Off; 
	}
}


/**
  * @brief  �ŵ����
  * @param  void
  * @retval void
  */
void SuperCap_Giveout_Control(void)
{
	if(capFlag == cap_giveout)
	{
		CAP_OUT_On;	 
	}
	else if(capFlag == cap_notgiveout)
	{
		CAP_OUT_Off; 
	}
}

/**
  * @brief  �ŵ�
  * @param  void
  * @retval void
  */

void SuperCap_Out(void)
{
//	chargeFlag = notcharging;
	chargeFlag = charging;
	getCapNowVol = Get_Realvoltage();
	if(getCapNowVol > Min_Vol)
	{
		capFlag = cap_giveout;
	}
	else
	{
		capFlag = cap_notgiveout;
	}
}


/**
  * @brief  ���
  * @param  void
  * @retval void
  */
void SuperCap_In(void)
{
	chargeFlag = charging;
	capFlag 	 = cap_notgiveout;
}


/**
  * @brief  ���������Ƿ�ŵ�
  * @param  void
  * @retval true false
  * @attention  ��ѹ���ͽ�ֹ�ŵ�
  */

bool Cap_Out_Can_Open(void)
{
	getCapNowVol = Get_Realvoltage();
	
	if(KEY_PRESSED_OFFSET_SHIFT && capFlag == cap_giveout && getCapNowVol > Min_Vol + 1.5f)
	{
		return TRUE;//���Էŵ�
	}
	else
	{
		return FALSE;//��ֹ�ŵ�
	}
}

