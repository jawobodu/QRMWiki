#include "friction.h"

#include "control.h"
#include "pwm.h"
#include "friction.h"
#include "remote.h"
#include "Task_Gimbal.h"
#include "laser.h"
#include "judge.h"
#include "vision.h"
#include "magazine.h"

/*****************����Ƶ���뽵�����٣����������Ħ�������***********************/

//���ļ��кܶ಻̫����ĵط�,ֻ��Ϊ��������
/**************************Ħ���ֿ���***********************************/


//[0, 13~13.5, 17.5~18, 22~23, ���ݵ���ѡ��, �ڱ�27]
//Ħ�����ٶ�ѡ��,Ӱ���ӵ��ٶ�,ֻ�ǲ���,ϵ��Ҫ���涨
#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
/*snail*/
//	float Friction_PWM_Output[6]     = {0, 450, 450, 550, 550, 600};//�ر�  ����  ����  ����  ��,���650���ٶ�25.5m/s
//550->23.5~25.5   450->20~21

///*С�۷�*/
	float Friction_PWM_Output[6]     = {0, 480, 550, 650, 730/*730*/, 730};//�ر�  ����  ����  ����  ��  �ڱ�
//600->23.7~24.1    630->24~25   550->20~21    450->12.5~13.5
//710����������ǽǼ��ٶȶ�ȡ	

/*��ӯխ��40A*/
//	float Friction_PWM_Output[6]     = {0, 500, 600, 700, 710, 730};//�ر�  ����  ����  ����  ��,���650���ٶ�25.5m/s
////600->21    650->22~23   700->23.5~24.5    750->25~27

#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE
	float Friction_PWM_Output[6]     = {0, 450, 550, 650, 720/*685*//*690*/, 730};//�ر�  ����  ����  ����  ��
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
	float Friction_PWM_Output[6]     = {0, 455, 510, 598/*570*/, 695, 685};//�ر�  ����  ����  ����  ��  �ڱ�
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE    //600��
	float Friction_PWM_Output[6]     = {0, 465, 520, 592, 715/*675*/, 675};//�ر�  ����  ����  ����  ��  �ڱ�
	//���27 �ڱ�25.5
	
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
	float Friction_PWM_Output[6]     = {0, 460, 505, 583, 695, 685};//�ر�  ����  ����  ����  ��  �ڱ�
	
#endif

//Ħ���ֲ�ͬpwm�¶�Ӧ����������ֵ(����),��ñ�ʵ��ֵ��5
uint16_t Friction_PWM_HeatInc[5] = {0,  20,  26,  34,  36};//����ʱ��㶨���ٶ�,������Ը���


//ң��ģʽ�µ�һЩ��־λ
#define    FRIC_STEP0    0
#define    FRIC_STEP1    1
#define    FRIC_STEP2    2

//�ٶ�ѡ��
#define FRI_OFF  	0
#define FRI_LOW  	1		//Z������
#define FRI_MID  	2		//B������
#define FRI_HIGH 	3		//���
#define FRI_MAD  	4		//�������
#define FRI_SENTRY  5		//�ڱ�����

//�ٶȵȼ�ѡ��
uint16_t Fric_Speed_Level;

//ң��ģʽ�µĿ�����־λ
uint8_t Friction_Switch = 0;//�뵯�ֿ����ж�����

//Ħ����Ŀ���ٶ�
float Friction_Speed_Target;

//Ħ���ֵȼ�Ŀ��ת��
float Frict_Speed_Level_Target;

//Ħ����ʵ������ٶ�,������б�����
float Friction_Speed_Real;




/**
  * @brief  Ħ����ʧ�ر���
  * @param  void
  * @retval void
  * @attention ������СPWM���,����ᱨ��
  */
void FRICTION_StopMotor(void)
{
	Friction_Speed_Target = 0;

	if (Friction_Speed_Real > 0)
	{
		Friction_Speed_Real -= 1;//3
	}

	if (Friction_Speed_Real < 0)
	{
		Friction_Speed_Real = 0;
	}

	TIM4_FrictionPwmOutp( Friction_Speed_Real, Friction_Speed_Real );
}

/************************Ħ�����ܿ���*****************************/

/**
  * @brief  Ħ���ֿ��ƺ���
  * @param  void
  * @retval void
  * @attention Ħ����ֹͣת��,����ر�,Ħ���ִӹرյ�����,��̨̧ͷ����
  */
float fric_rc_debug = 300;
void FRICTION_Ctrl( void )
{
//	uint8_t level;//��ǰ�����˵ȼ�
	
	if(SYSTEM_GetSystemState( ) == SYSTEM_STARTING)
	{
		Fric_Speed_Level = FRI_OFF;
		Friction_Speed_Target = 0;
		Friction_Speed_Real   = 0;
		Laser_Off;
	}
	else
	{
		if (SYSTEM_GetRemoteMode( ) == RC)//ң��ģʽ
		{
			Fric_Speed_Level = FRI_LOW;//ң��ģʽ�µ��ٶ�ѡ�񣬵����٣������¼���ⵯ
			
			if (FRIC_RcSwitch( ) == TRUE)//�ж�״̬�л�,�����ֿ����߼���ͬ
			{	//�л�Ϊ��
				if (Friction_Speed_Target > Friction_PWM_Output[FRI_OFF])
				{
					Friction_Speed_Target = Friction_PWM_Output[FRI_OFF];	
				}
				else//�л�Ϊ��
				{
					Friction_Speed_Target = Friction_PWM_Output[Fric_Speed_Level];//Ħ����Ŀ��ֵ����0,��־��ң��ģʽ�¿���,����pitchҪ̧ͷ
				}
			}
			else
			{
				if(Friction_Speed_Target > Friction_PWM_Output[Fric_Speed_Level])
				{
					Friction_Speed_Target = Friction_PWM_Output[Fric_Speed_Level];
				}
			}
		}
		else				//����ģʽ,�ɵ�����
		{
//			level = JUDGE_ucGetRobotLevel();//��ȡ�ȼ�
			FRIC_KeyLevel_Ctrl();
		}
	}
	
	//Ħ�������б��,ע��Ҫ��̧ͷ���ܽ���б��
	Friction_Ramp();
	
	//���⿪��
	if(Friction_Speed_Real > 0 && GIMBAL_IfBuffHit() == FALSE && Magazine_IfOpen() == FALSE)//��ʵ���������һ��
	{
		Laser_On;
	}
	else
	{
		Laser_Off;
		//Laser_On;
	}
	
	TIM4_FrictionPwmOutp(Friction_Speed_Real, Friction_Speed_Real);
}

/**
  * @brief  Ħ����ң�ؿ���
  * @param  void
  * @retval �Ƿ�ת����ǰ״̬
  * @attention �����ֿ����߼���ͬ
  */
bool FRIC_RcSwitch( void )
{
	if (IF_RC_SW2_DOWN)
	{
		if (IF_RC_SW1_UP)
		{
			if (Friction_Switch == FRIC_STEP1)
			{
				Friction_Switch = FRIC_STEP2;
			}
			else if (Friction_Switch == FRIC_STEP2)
			{
				Friction_Switch = FRIC_STEP0;
			}
		}
		else
		{
			Friction_Switch = FRIC_STEP1;
		}
	}
	else
	{
		Friction_Switch = FRIC_STEP0;
	}


	if (Friction_Switch == FRIC_STEP2)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  Ħ���ֵȼ�����,����ר��,���ݵȼ��Զ���������
  * @param  ��ǰ�ȼ�
  * @retval void
  * @attention ����ģʽ�²���Ħ����
  */
float debug_fric = 450;//760;//650;
float Fric_Dist_Far = 3;//���ڴ˾�����������
float Fric_Dist_Near = 1.2;//С�ڴ˾����������
void FRIC_KeyLevel_Ctrl(void)
{
	static int16_t fric_auto_delay = 0;//��ֹ����Ƶ������
	float fric_percent = 1;
	float Fric_Dist_Now = 5;
	
	Fric_Dist_Now = VisionRecvData.distance/100;
	
	if (GIMBAL_IfBuffHit( ) == TRUE)
	{
		Fric_Speed_Level = FRI_MAD;//���ģʽ,�������
		Frict_Speed_Level_Target = Friction_PWM_Output[Fric_Speed_Level];//����ѡ��
	}
	else if (IF_MOUSE_PRESSED_LEFT && GIMBAL_IfAutoHit() == FALSE)//�����飬�������
	{
		Fric_Speed_Level = FRI_HIGH;//һ���ø�����
		Frict_Speed_Level_Target = Friction_PWM_Output[Fric_Speed_Level];//����ѡ��
	}
	else if (IF_KEY_PRESSED_Z)//�Ƽ�����
	{
		Fric_Speed_Level = FRI_LOW;				
		Frict_Speed_Level_Target = Friction_PWM_Output[Fric_Speed_Level];//����ѡ��		
	}
	else if (IF_KEY_PRESSED_B)
	{
		Fric_Speed_Level = FRI_MID;//����Ƶ������ģʽ,�Ƽ��ⲫ��
		Frict_Speed_Level_Target = Friction_PWM_Output[Fric_Speed_Level];//����ѡ��
	}
	else if (GIMBAL_IfAutoHit() == TRUE)//����ģʽ
	{
		if(VisionRecvData.identify_target == TRUE)
		{
			fric_auto_delay = 0;
			
			fric_percent = (Fric_Dist_Now - Fric_Dist_Near) / (Fric_Dist_Far - Fric_Dist_Near);
			fric_percent = constrain_float(fric_percent, 0, 1);
			
			Frict_Speed_Level_Target = Friction_PWM_Output[FRI_LOW] 
										+ (Friction_PWM_Output[FRI_HIGH] - Friction_PWM_Output[FRI_LOW]) * fric_percent;
		}
		else//ûʶ�𵽣���һ��ʱ������л��ٶ�
		{
			fric_auto_delay++;
			if(fric_auto_delay >= 20)//����200msûʶ��
			{
				fric_auto_delay = 200;//��ֹ���
				Fric_Speed_Level = FRI_HIGH;//һ���ø�����
				Frict_Speed_Level_Target = Friction_PWM_Output[Fric_Speed_Level];//����ѡ��
			}
		}
		
		if(GIMBAL_AUTO_PITCH_SB() == TRUE)//������ڱ����������
		{
			Fric_Speed_Level = FRI_SENTRY;
			Frict_Speed_Level_Target = Friction_PWM_Output[Fric_Speed_Level];//����ѡ��
		}
	}
	else//��ֹ����
	{
		Fric_Speed_Level = FRI_HIGH;	
		Frict_Speed_Level_Target = Friction_PWM_Output[Fric_Speed_Level];//����ѡ��
	}

	if( IF_MOUSE_PRESSED_LEFT || Friction_Speed_Target>0 )//�����м����������BUG����̧ͷ���Զ���ǹ
	{
		//Friction_Speed_Target = debug_fric;
		Friction_Speed_Target = Frict_Speed_Level_Target;
	}
}


/***********Ħ����������̨̧ͷ�жϺ���*************/

/**
  * @brief  Ħ�����Ƿ��Ѿ�����
  * @param  void
  * @retval TRUE�ѿ���   FALSEδ����
  * @attention 
  */
uint8_t FRIC_IfWait( void )
{
    if (Friction_Speed_Target > 0)//ͨ���ı�Ħ����Ŀ��ֵ����־ң��ģʽ��pitch�Ƿ�Ҫ̧ͷ
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  Ħ�����Ƿ����ڿ���
  * @param  void
  * @retval TRUE���ڿ���   FALSE������Ŀ���ٶ�
  * @attention ���ڿ�����׼�ı���̨PITCH
  */
uint8_t FRIC_IfOpen( void )
{
	static  uint8_t  status = FALSE;
	static uint32_t  ulWait = 0;

	static portTickType ulCurrent = 0;

	ulCurrent = xTaskGetTickCount( );
	
    if (Friction_Speed_Real > 0)
	{
		if (ulCurrent >= ulWait + 1500)//̧ͷʱ��,1.5S
		{
			status = TRUE;
		}
	}
	else
	{
		ulWait = ulCurrent;
		
		status = FALSE;
	}
	
	return status;
}

/*************Ħ���ָ�������****************/

/**
  * @brief  Ħ�������б��
  * @param  void
  * @retval void
  * @attention 
  */
void Friction_Ramp(void)
{
	if (Friction_Speed_Real < Friction_Speed_Target)//����
	{
		Friction_Speed_Real += 5;
		if(Friction_Speed_Real > Friction_Speed_Target)
		{
			Friction_Speed_Real = Friction_Speed_Target;
		}
	}
	else if (Friction_Speed_Real > Friction_Speed_Target)//�ر�
	{
		Friction_Speed_Real -= 5;
	}
	
	if (Friction_Speed_Real < 0)
	{
		Friction_Speed_Real = 0;
	}
}

/**
  * @brief  ��ǰPWM��Ӧ�����ӵ���������ֵ(����)
  * @param  void
  * @retval ��ǰ��������ֵ
  * @attention ��������42mm,�����ڿͻ���������ʾ
  */
uint16_t Fric_GetHeatInc(void)
{
	if(GIMBAL_IfAutoHit() == TRUE && !IF_KEY_PRESSED_Z && !IF_KEY_PRESSED_B)
	{
		return JUDGE_usGetSpeedHeat17()*1.4f;
	}
	else
	{
		return Friction_PWM_HeatInc[Fric_Speed_Level];
	}
}

/**
  * @brief  ��ȡ��ǰĦ����PWM���ֵ
  * @param  void
  * @retval ʵ��PWMֵ
  * @attention ������ֹĦ�����ٶȹ��͵�����²��̵�ת��
  */
float Fric_GetSpeedReal(void)
{
	return Friction_Speed_Real;
}


