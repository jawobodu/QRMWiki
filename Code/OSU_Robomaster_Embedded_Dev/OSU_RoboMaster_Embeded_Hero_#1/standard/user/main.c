/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       main.c/h
  * @brief      stm32��ʼ���Լ���ʼ����freeRTOS��h�ļ��������ȫ�ֺ궨���Լ�
  *             typedef һЩ������������
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. ���
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2016 DJI****************************
  */
#include "main.h"

#include "stm32f4xx.h"

#include "adc.h"
#include "buzzer.h"
#include "can.h"
#include "delay.h"
#include "flash.h"
#include "fric.h"
#include "laser.h"
#include "led.h"
#include "power_ctrl.h"
#include "rc.h"
#include "rng.h"
#include "sys.h"
#include "timer.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "calibrate_task.h"
#include "remote_control.h"
#include "start_task.h"


#include "gpio.h"

void BSP_init(void);
User_GPIO_X user_gpio;
User_PWM_X user_pwm;

int main(void)
{
    BSP_init();
    delay_ms(100);
    startTask();
    vTaskStartScheduler();
    while (1)
    {
        ;
    }
}

//�ĸ�24v ��� ���ο��� ��� 709us
#define POWER_CTRL_ONE_BY_ONE_TIME 709

void BSP_init(void)
{
    //�ж��� 4
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    //��ʼ���δ�ʱ��
    delay_init(configTICK_RATE_HZ);
    //��ˮ�ƣ����̵Ƴ�ʼ��
    led_configuration();
    //stm32 �����¶ȴ�������ʼ��
    temperature_ADC_init();
#if GIMBAL_MOTOR_6020_CAN_LOSE_SLOVE
    //stm32 �������������ʼ��
    RNG_init();
#endif
    //24������ƿ� ��ʼ��
    power_ctrl_configuration();

		
///////////////////////////////////////////////////////////////////////////////
		//��ʼ������GPIO
		GPIO_ID_E GPIO_ID_LIST[17]={I1,I2,J1,J2,K1,K2,L1,L2,M1,M2,N1,N2,O1,O2,P1,P2,Q2};//������Ҫ��ʼ�����õĶ˿�
		int i;
		for (i=0;i<17;i++)
		{	
			if(GPIO_ID_LIST[i]==NULL)//���˿�δָ��
			{
				;//��ʲôҲ����������
			}
			else
			{
				user_gpio.GPIO_ID=GPIO_ID_LIST[i];//����GPIO_IDΪָ���˿�
				User_GPIO_Init(&user_gpio);//��ʼ������GPIO
			}
		}
///////////////////////////////////////////////////////////////////////////////
		
///////////////////////////////////////////////////////////////////////////////
		//��ʼ������PWM
		PWM_ID_E PWM_ID_LIST[16]={A,B,C,D,E,F,G,H,S,T,U,V,W,X,Y,Z};//������Ҫ��ʼ�����õĶ˿�
		int k;
		for (k=0;k<16;k++)
		{	
			if(PWM_ID_LIST[k]==NULL)//���˿�δָ��
			{
				;//��ʲôҲ����������
			}
			else
			{
				user_pwm.PWM_ID=PWM_ID_LIST[k];//����PWM_IDΪָ���˿�
				User_PWM_Init(&user_pwm);//��ʼ������PWM
			}
		}
////////////////////////////////////////////////////////////////////////////////

		//17mmĦ���ֵ��PWM��ʼ��
    fric_PWM_configuration();
    //��������ʼ��
    buzzer_init(30000, 90);
    //����IO��ʼ��
    laser_configuration();
    //��ʱ��6 ��ʼ��
    TIM6_Init(60000, 90);
    //CAN�ӿڳ�ʼ��
    CAN1_mode_init(CAN_SJW_1tq, CAN_BS2_2tq, CAN_BS1_6tq, 5, CAN_Mode_Normal);//CAN1-����ģʽCAN_Mode_Normal���ػ�ģʽCAN_Mode_LoopBack
    CAN2_mode_init(CAN_SJW_1tq, CAN_BS2_2tq, CAN_BS1_6tq, 5, CAN_Mode_Normal);//CAN2-����ģʽCAN_Mode_Normal���ػ�ģʽCAN_Mode_LoopBack

    //24v ��� �����ϵ�
    for (uint8_t i = POWER1_CTRL_SWITCH; i < POWER4_CTRL_SWITCH + 1; i++)
    {
        power_ctrl_on(i);
        delay_us(POWER_CTRL_ONE_BY_ONE_TIME);
    }
    //ң������ʼ��
    remote_control_init();
    //flash��ȡ��������У׼ֵ�Żض�Ӧ����
    cali_param_init();
}
