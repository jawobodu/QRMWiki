#include "magazine.h"

#include "control.h"
#include "remote.h"
#include "judge.h"
#include "Task_Revolver.h"
#include "Task_Gimbal.h"

/* ����pwm --> 23~127��Χ�ɶ� */ 


/*--------------------------------------------------------*/
//���ֿ��ؽǶȶ�Ӧ��PWMֵ
#if		INFANTRY_DEBUG_ID == DEBUG_ID_ZERO
			#define Magazine_Close_Angle   53
			#define Magazine_Open_Angle    75//105
			
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_ONE
			#define Magazine_Close_Angle   25
			#define Magazine_Open_Angle    63
			
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_TWO
			#define Magazine_Close_Angle   88
			#define Magazine_Open_Angle    50
			
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_THREE
			#define Magazine_Close_Angle   88
			#define Magazine_Open_Angle    50
			
#elif	INFANTRY_DEBUG_ID == DEBUG_ID_FOUR
			#define Magazine_Close_Angle   89
			#define Magazine_Open_Angle    52
			
#endif
/*--------------------------------------------------------*/

//���ֿ��ر�־λ
#define MAGA_STEP0    0		//ʧ�ܱ�־
#define MAGA_STEP1    1		//SW1��λ��־
#define MAGA_STEP2    2		//���ֿ��ر�־

#define MAGA_KEY_CLOSE    0		//���ּ���ģʽ���ر�־λ
#define MAGA_KEY_OPEN     1		//���ּ���ģʽ���ر�־λ
uint8_t	Magazine_Switch = 0;//����ң��ģʽ���ر�־λת��
uint8_t Magazine_Key_Switch = 0;//���ּ���ģʽ���ر�־λת��

int16_t Magazine_Target;//PWMĿ��ֵ
int16_t Magazine_Actual;//PWM��ʵֵ
int16_t Magazine_ServoRamp = 100;//����б��,���Ʊ仯�ٶ�,ʵ�����ûʲô��


u8 Maga_Switch_R = 1;
u8 Maga_Key_R_Change = 0;
u8 Maga_Times = 0;


bool Senty_Run_Flag = FALSE;

/**
  * @brief  ���ֶ��ֹͣת��
  * @param  void
  * @retval void
  * @attention ʧ�ر���,��0���ֲ���,��ʱ���ֶ��򿪵���
  */
void Magazine_StopCtrl(void)
{
	Magazine_Servo(0);
}

/******************���ֿ���,loopѭ������*************************/

/**
  * @brief  ���ֶ������
  * @param  void
  * @retval void
  * @attention 
  */
uint8_t maga_remot_change = TRUE;
void Magazine_Ctrl(void)
{
	if (SYSTEM_GetSystemState() == SYSTEM_STARTING)//����Ƕ�ϵͳ��ʼ��
	{
		//�Ƕȳ�ʼ��,ʹĿ��ֵ�����ֵ��Ϊ�ر�ֵ
		Magazine_Target = Magazine_Close_Angle;
		Magazine_Actual = Magazine_Close_Angle;
		Maga_Switch_R = 1;
		Maga_Key_R_Change = 0;
		Maga_Times = 0;
	}
	else
	{
		if (Magezine_Rc_Switch( ) == TRUE)//�ж��Ƿ�Ҫ���ָı䵱ǰ״̬
		{
			//�ı䵱ǰ״̬���ж�
			if (Magazine_Target == Magazine_Open_Angle)//���ֿ����˴�ʼ�ս���
			{
				Magazine_Target = Magazine_Close_Angle;//��֮ǰ��,�����ڹر�
			}
			else			//�ڵ��ֹر�֮��˴�ʼ�ս���
			{
				Magazine_Target = Magazine_Open_Angle;//��֮ǰ�ر�,�����ڴ�
			}
		}
				
		if (SYSTEM_GetRemoteMode() == KEY)
		{
			Magazine_Key_Ctrl();
		}
		else
		{
			Magazine_Key_Switch = MAGA_KEY_CLOSE;
			Maga_Switch_R = 1;
			Maga_Key_R_Change = 0;
			Maga_Times = 0;
			maga_remot_change = TRUE;//����л���ң��ģʽ
		}
	}
	

	//ʹ���ʵ��ֵ�𲽱ƽ�Ŀ��ֵ,б�����
	if (Magazine_Actual < Magazine_Target)
	{
		Magazine_Actual += Magazine_ServoRamp;
		Magazine_Actual = constrain_int16_t( Magazine_Actual, Magazine_Actual, Magazine_Target );
	}
	else if (Magazine_Actual > Magazine_Target)
	{
		Magazine_Actual -= Magazine_ServoRamp;
		Magazine_Actual = constrain_int16_t( Magazine_Actual, Magazine_Target, Magazine_Actual );
	}
	

	Magazine_Servo(Magazine_Actual);
	
	
	/*********************/
	if(IF_KEY_PRESSED_X && IF_KEY_PRESSED_G)
	{
		Senty_Run_Flag = TRUE;
	}
}

/**
  * @brief  ң��ģʽ,�ж��Ƿ��´���״̬ת��ָ��,����һ��֮�����̱��FALSE
  * @param  void
  * @retval �Ƿ��´��˸ı�״̬��ָ��
  * @attention �߼��ϸ���,�ú�����
  */
bool Magezine_Rc_Switch(void)
{
	if (IF_RC_SW2_MID)//ң��ģʽ
	{
		if (IF_RC_SW1_UP)//������������1
		{
			if (Magazine_Switch == MAGA_STEP1)//������������2
			{
				Magazine_Switch = MAGA_STEP2;
			}
			else if (Magazine_Switch == MAGA_STEP2)//���ֹر�
			{
				Magazine_Switch = MAGA_STEP0;//�ж���ϵ
			}
		}
		else		//��־SW1�Ƿ��и�λ�����,�ڸ�λ������²����ٴν���STERP2
		{
			Magazine_Switch = MAGA_STEP1;//����SW1���´α任֮ǰһֱ������
		}
	}
	else//s2�����м�,�������ֿ���
	{
		Magazine_Switch = MAGA_STEP0;//������Ħ���ֿ���Ҳ�������л��ɼ���ģʽ
	}
	
	
	if (Magazine_Switch == MAGA_STEP2)
	{
		return TRUE;//ֻ��SW1���±任��ʱ���ΪTRUE
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  ����ģʽ
  * @param  void
  * @retval void
  * @attention 
  */
void Magazine_Key_Ctrl(void)
{
	static uint32_t ulTimePressS   = 0;
//	static uint32_t ulTimeOpen     = 0;
	       portTickType ulTimeCurrent  = 0;
	static uint32_t PressR_Gap = 0;//�ص�������£�R��һ��֮��ʱ�䲻�ٰ�һ������Դ˴�
	
	if(maga_remot_change == TRUE)//�մ�ң��ģʽ�й���
	{
		Magazine_Key_Switch = MAGA_KEY_CLOSE;
		Magazine_Target = Magazine_Close_Angle;
		maga_remot_change = FALSE;
	}
	
	ulTimeCurrent = xTaskGetTickCount();
	
	switch (Magazine_Key_Switch)
	{
		case MAGA_KEY_CLOSE:	
			if(!IF_KEY_PRESSED_R)//R�ɿ�
			{
				Maga_Switch_R = 1;
				if(ulTimeCurrent - PressR_Gap > TIME_STAMP_500MS)//500ms��û����R
				{
					Maga_Times = 0;//���¼Ǵ�
				}
			}
			
			if (IF_KEY_PRESSED_R && Maga_Switch_R == 1
					&& GIMBAL_IfBuffHit() != TRUE)//R����
			{
				PressR_Gap = ulTimeCurrent;//��¼����ʱ��
				Maga_Switch_R = 0;	
				Maga_Times++;	
			}	
			
			if(Maga_Times >= 2)//500ms������2��
			{
				Magazine_Key_Switch = MAGA_KEY_OPEN;//������
				Magazine_Target = Magazine_Open_Angle;
				if(JUDGE_usGetShootNum()>0)
				{
					JUDGE_ShootNum_Clear();//����������
					Revol_Angle_Clear();//���̽Ƕ�����
				}
				ulTimePressS = ulTimeCurrent;
			}
			else
			{
				Magazine_Target = Magazine_Close_Angle;
			}
		break;
				
		case MAGA_KEY_OPEN:	
			if(!IF_KEY_PRESSED_R)//R�ɿ�
			{
				Maga_Switch_R = 1;
			}
			
			if (!IF_KEY_PRESSED_S)
			{
				ulTimePressS = ulTimeCurrent;//ˢ��S���µ�ʱ��
			}
			
			if ( ulTimeCurrent - ulTimePressS >  (TIME_STAMP_500MS + TIME_STAMP_300MS)  //����S����800ms
						|| IF_KEY_PRESSED_Q || IF_KEY_PRESSED_E || IF_KEY_PRESSED_V )	
			{
				Magazine_Key_Switch = MAGA_KEY_CLOSE;
			}
			else
			{
				Magazine_Target = Magazine_Open_Angle;
			}
			Maga_Times = 0;
		break;		
	}
	
}

/**************���ּ���ģʽ����С����****************/

/**
  * @brief  ���ֶ���ŷ�
  * @param  Ŀ��PWMֵ
  * @retval void
  * @attention 28��С
  */
void Magazine_Servo(int16_t pwm)
{
	pwm = abs(pwm);

	TIM1->CCR2 = pwm;
}


/*******************���ָ�������*************************/

/**
  * @brief  �����Ƿ��Ѿ������
  * @param  void
  * @retval TRUE��,FALSEδ��
  * @attention 
  */
bool Magazine_IfOpen(void)
{
	if (Magazine_Actual == Magazine_Open_Angle)//��ʵ������ж�
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
  * @brief  �����Ƿ����ڴ�
  * @param  void
  * @retval TRUE����,falseδ��
  * @attention 
  */
bool Magazine_IfWait(void)
{
	if (Magazine_Target == Magazine_Open_Angle)//��Ŀ������ж�
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/****************�����ڱ������жϣ���ˢ���ʣ���ֹ��ⲻ������*********************/

bool Senty_Run(void)
{
	return Senty_Run_Flag;
}

void Senty_Run_Clean_Flag(void)
{
	Senty_Run_Flag = FALSE;
}

