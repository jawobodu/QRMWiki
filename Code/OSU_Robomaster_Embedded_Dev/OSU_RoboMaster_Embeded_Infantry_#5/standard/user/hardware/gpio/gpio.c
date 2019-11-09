/**
  ****************************(C) COPYRIGHT 2019 OSU-RM-TEAM****************************
  * @file       gpio.c/h
  * @brief      �Զ���GPIO��PWM�ӿ�
	*	@note 			SYSCLK = HSE_VALUE/PLL_M*PLL_N/PLL_P = 12/6*180/2 = 180MHz
	*	@note 			ABP1_CLK = 180MHz
	* @note 			ABP2_CLK = 90MHz
	* @note 			��Prescaler����0ʱĬ��Ϊ1��Prescaler������1ʱ����ôTIM1��TIM8-TIM11��ʱ��ΪAPB2��ʱ�ӵ�������TIM2-TIM7��TIM12-TIM14��ʱ��ΪAPB1��ʱ�ӵ�����     
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     May-14-2019     OSU-RM              1. ���
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 OSU-RM-TEAM****************************
  */
	
#include "stm32f4xx.h"
#include "gpio.h"

//����ṹ�����
GPIO_InitTypeDef GPIO_InitStructure;
TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;

//��ʼ�������Զ���GPIO
void User_GPIO_Init(User_GPIO_X* gpio_x)
	{
	//�ж�GPIO�˿�
	if(gpio_x->GPIO_ID==I1)
	{
		gpio_x->GPIOX=GPIOF;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOF;
		gpio_x->GPIO_Pin_x=GPIO_Pin_1;
	}
	else if(gpio_x->GPIO_ID==I2)
	{
		gpio_x->GPIOX=GPIOF;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOF;
		gpio_x->GPIO_Pin_x=GPIO_Pin_0;
	}
	else if(gpio_x->GPIO_ID==J1)
	{
		gpio_x->GPIOX=GPIOE;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOE;
		gpio_x->GPIO_Pin_x=GPIO_Pin_5;
	}
	else if(gpio_x->GPIO_ID==J2)
	{
		gpio_x->GPIOX=GPIOE;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOE;
		gpio_x->GPIO_Pin_x=GPIO_Pin_4;
	}
	else if(gpio_x->GPIO_ID==K1)
	{
		gpio_x->GPIOX=GPIOE;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOE;
		gpio_x->GPIO_Pin_x=GPIO_Pin_6;
	}
	else if(gpio_x->GPIO_ID==K2)
	{
		gpio_x->GPIOX=GPIOE;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOE;
		gpio_x->GPIO_Pin_x=GPIO_Pin_12;
	}
	else if(gpio_x->GPIO_ID==L1)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_2;
	}
	else if(gpio_x->GPIO_ID==L2)
	{
		gpio_x->GPIOX=GPIOB;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOB;
		gpio_x->GPIO_Pin_x=GPIO_Pin_0;
	}
	else if(gpio_x->GPIO_ID==M1)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_3;
	}
	else if(gpio_x->GPIO_ID==M2)
	{
		gpio_x->GPIOX=GPIOB;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOB;
		gpio_x->GPIO_Pin_x=GPIO_Pin_1;
	}
	else if(gpio_x->GPIO_ID==N1)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_4;
	}
	else if(gpio_x->GPIO_ID==N2)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_0;
	}
	else if(gpio_x->GPIO_ID==O1)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_5;
	}
	else if(gpio_x->GPIO_ID==O2)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_1;
	}
	else if(gpio_x->GPIO_ID==P1)
	{
		gpio_x->GPIOX=GPIOA;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOA;
		gpio_x->GPIO_Pin_x=GPIO_Pin_5;
	}
	else if(gpio_x->GPIO_ID==P2)
	{
		gpio_x->GPIOX=GPIOA;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOA;
		gpio_x->GPIO_Pin_x=GPIO_Pin_4;
	}
////��17mm΢������ռ��
//	else if(gpio_x->GPIO_ID==Q1)
//	{
//		gpio_x->GPIOX=GPIOF;
//		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOF;
//		gpio_x->GPIO_Pin_x=GPIO_Pin_10;
//	}
	else if(gpio_x->GPIO_ID==Q2)
	{
		gpio_x->GPIOX=GPIOI;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOI;
		gpio_x->GPIO_Pin_x=GPIO_Pin_9;
	}
	//����GPIO��Ӧ�˿�ʱ��
  RCC_AHB1PeriphClockCmd(gpio_x->RCC_AHB1Periph_GPIOX, ENABLE);
	//����GPIO�ܽ�
  GPIO_InitStructure.GPIO_Pin = gpio_x->GPIO_Pin_x;
	//����GPIOģʽ
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	//��ʼ���ṹ��
  GPIO_Init(gpio_x->GPIOX, &GPIO_InitStructure);
	
	//����GPIO�ܽ�
	GPIO_InitStructure.GPIO_Pin = gpio_x->GPIO_Pin_x;
	//��ʼ���ṹ��
  GPIO_Init(gpio_x->GPIOX, &GPIO_InitStructure);
}

//��ʼ�������Զ���PWM
void User_PWM_Init(User_PWM_X* pwm_x)
{
		//�ж�PWM�ӿ�
		if(pwm_x->PWM_ID==A)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOI;
			pwm_x->GPIOX=GPIOI;
			pwm_x->GPIO_Pin_x=GPIO_Pin_0;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource0;
			
			pwm_x->TIMx=TIM5;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM5;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM5;
			pwm_x->CHANNEL=4;
		}
		else if(pwm_x->PWM_ID==B)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOH;
			pwm_x->GPIOX=GPIOH;
			pwm_x->GPIO_Pin_x=GPIO_Pin_12;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource12;
			
			pwm_x->TIMx=TIM5;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM5;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM5;
			pwm_x->CHANNEL=3;
		}
		else if(pwm_x->PWM_ID==C)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOH;
			pwm_x->GPIOX=GPIOH;
			pwm_x->GPIO_Pin_x=GPIO_Pin_11;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource11;
			
			pwm_x->TIMx=TIM5;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM5;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM5;
			pwm_x->CHANNEL=2;
		}
		else if(pwm_x->PWM_ID==D)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOH;
			pwm_x->GPIOX=GPIOH;
			pwm_x->GPIO_Pin_x=GPIO_Pin_10;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource10;
			
			pwm_x->TIMx=TIM5;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM5;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM5;
			pwm_x->CHANNEL=1;
		}
		else if(pwm_x->PWM_ID==E)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOD;
			pwm_x->GPIOX=GPIOD;
			pwm_x->GPIO_Pin_x=GPIO_Pin_15;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource15;
			
			pwm_x->TIMx=TIM4;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM4;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM4;
			pwm_x->CHANNEL=4;
		}
		else if(pwm_x->PWM_ID==F)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOD;
			pwm_x->GPIOX=GPIOD;
			pwm_x->GPIO_Pin_x=GPIO_Pin_14;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource14;
			
			pwm_x->TIMx=TIM4;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM4;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM4;
			pwm_x->CHANNEL=3;
		}
		else if(pwm_x->PWM_ID==G)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOD;
			pwm_x->GPIOX=GPIOD;
			pwm_x->GPIO_Pin_x=GPIO_Pin_13;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource13;
			
			pwm_x->TIMx=TIM4;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM4;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM4;
			pwm_x->CHANNEL=2;
		}
		else if(pwm_x->PWM_ID==H)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOD;
			pwm_x->GPIOX=GPIOD;
			pwm_x->GPIO_Pin_x=GPIO_Pin_12;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource12;
			
			pwm_x->TIMx=TIM4;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM4;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM4;
			pwm_x->CHANNEL=1;
		}
		else if(pwm_x->PWM_ID==S)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOA;
			pwm_x->GPIOX=GPIOA;
			pwm_x->GPIO_Pin_x=GPIO_Pin_0;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource0;
			
			pwm_x->TIMx=TIM2;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM2;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM2;
			pwm_x->CHANNEL=1;
		}
		else if(pwm_x->PWM_ID==T)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOA;
			pwm_x->GPIOX=GPIOA;
			pwm_x->GPIO_Pin_x=GPIO_Pin_1;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource1;
			
			pwm_x->TIMx=TIM2;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM2;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM2;
			pwm_x->CHANNEL=2;
		}
		else if(pwm_x->PWM_ID==U)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOA;
			pwm_x->GPIOX=GPIOA;
			pwm_x->GPIO_Pin_x=GPIO_Pin_2;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource2;
			
			pwm_x->TIMx=TIM2;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM2;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM2;
			pwm_x->CHANNEL=3;
		}
		else if(pwm_x->PWM_ID==V)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOA;
			pwm_x->GPIOX=GPIOA;
			pwm_x->GPIO_Pin_x=GPIO_Pin_3;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource3;
			
			pwm_x->TIMx=TIM2;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM2;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB1Periph_TIM2;
			pwm_x->CHANNEL=4;
		}
		else if(pwm_x->PWM_ID==W)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOI;
			pwm_x->GPIOX=GPIOI;
			pwm_x->GPIO_Pin_x=GPIO_Pin_5;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource5;
			
			pwm_x->TIMx=TIM8;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM8;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB2Periph_TIM8;
			pwm_x->CHANNEL=1;
		}
		else if(pwm_x->PWM_ID==X)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOI;
			pwm_x->GPIOX=GPIOI;
			pwm_x->GPIO_Pin_x=GPIO_Pin_6;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource6;
			
			pwm_x->TIMx=TIM8;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM8;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB2Periph_TIM8;
			pwm_x->CHANNEL=2;
		}
		else if(pwm_x->PWM_ID==Y)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOI;
			pwm_x->GPIOX=GPIOI;
			pwm_x->GPIO_Pin_x=GPIO_Pin_7;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource7;
			
			pwm_x->TIMx=TIM8;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM8;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB2Periph_TIM8;
			pwm_x->CHANNEL=3;
		}
		else if(pwm_x->PWM_ID==Z)
		{	
			pwm_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOI;
			pwm_x->GPIOX=GPIOI;
			pwm_x->GPIO_Pin_x=GPIO_Pin_2;
			pwm_x->GPIO_PinSourcex=GPIO_PinSource2;
			
			pwm_x->TIMx=TIM8;
			pwm_x->GPIO_AF_TIMx=GPIO_AF_TIM8;
			pwm_x->RCC_APBxPeriph_TIMx=RCC_APB2Periph_TIM8;
			pwm_x->CHANNEL=4;
		}
		
		//����GPIO��Ӧ�˿�ʱ��
    RCC_AHB1PeriphClockCmd(pwm_x->RCC_AHB1Periph_GPIOX, ENABLE); 
		//����GPIO���ö�Ӧ�˿ڣ��ܽź�ӳ��ʱ��
		GPIO_PinAFConfig(pwm_x->GPIOX, pwm_x->GPIO_PinSourcex, pwm_x->GPIO_AF_TIMx);
		//����GPIO����
    GPIO_InitStructure.GPIO_Pin = pwm_x->GPIO_Pin_x;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		//��ʼ��GPIO�ṹ��
	  GPIO_Init(pwm_x->GPIOX, &GPIO_InitStructure);
		
		//���ü�ʱ��
		if(pwm_x->TIMx==TIM2||pwm_x->TIMx==TIM3||pwm_x->TIMx==TIM4||pwm_x->TIMx==TIM5||pwm_x->TIMx==TIM6||pwm_x->TIMx==TIM7||pwm_x->TIMx==TIM12||pwm_x->TIMx==TIM13||pwm_x->TIMx==TIM14)
		{
			RCC_APB1PeriphClockCmd(pwm_x->RCC_APBxPeriph_TIMx, ENABLE);//APB1����ʱ��=SYSCLK/1
		}
		else if(pwm_x->TIMx==TIM1||pwm_x->TIMx==TIM8||pwm_x->TIMx==TIM9||pwm_x->TIMx==TIM10||pwm_x->TIMx==TIM11)
		{
			RCC_APB2PeriphClockCmd(pwm_x->RCC_APBxPeriph_TIMx, ENABLE);//APB2����ʱ��=SYSCLK/2
		}
//		//����Timer
//		RCC_APB1PeriphResetCmd(pwm_x->RCC_APBxPeriph_TIMx, ENABLE);
//    RCC_APB1PeriphResetCmd(pwm_x->RCC_APBxPeriph_TIMx, DISABLE);
		
		//����Timer����
		TIM_TimeBaseInitStructure.TIM_Period = 2000-1;//2000=500Hz, 20000=50Hz
		if(pwm_x->TIMx==TIM2||pwm_x->TIMx==TIM3||pwm_x->TIMx==TIM4||pwm_x->TIMx==TIM5||pwm_x->TIMx==TIM6||pwm_x->TIMx==TIM7||pwm_x->TIMx==TIM12||pwm_x->TIMx==TIM13||pwm_x->TIMx==TIM14)
		{
			TIM_TimeBaseInitStructure.TIM_Prescaler = 90-1;//2*180M/Period/Prescaler=1/Frequency
		}
		else if(pwm_x->TIMx==TIM1||pwm_x->TIMx==TIM8||pwm_x->TIMx==TIM9||pwm_x->TIMx==TIM10||pwm_x->TIMx==TIM11)
		{
			TIM_TimeBaseInitStructure.TIM_Prescaler = 180-1;
		}

    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���
		TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//ʱ�ӷ�Ƶ Divided by 1
		//��ʼ��Timer
    TIM_TimeBaseInit(pwm_x->TIMx, &TIM_TimeBaseInitStructure);
		
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//����PWMģʽ 
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//�������
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//�������ͨ����ƽ����
		TIM_OCInitStructure.TIM_Pulse = 1000;// ����ռ�ձȴ�С
		
		if(pwm_x->TIMx==TIM1||pwm_x->TIMx==TIM8)
		{
		//Only valid to TIM1 or TIM8
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;// �������
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;// �������ͨ����ƽ��������
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;// ���ͨ�����е�ƽ��������
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;// �������ͨ�����е�ƽ��������
		}
		else
		{
			;//����
		}

		//��ʼ��Timer��ӦChannel
		if(pwm_x->CHANNEL==1)
		{
			TIM_OC1Init(pwm_x->TIMx, &TIM_OCInitStructure);
			//����Timer��Channel�ϵ�Preload�Ĵ���
			TIM_OC1PreloadConfig(pwm_x->TIMx, TIM_OCPreload_Enable);
		}
		else if (pwm_x->CHANNEL==2)
		{
			TIM_OC2Init(pwm_x->TIMx, &TIM_OCInitStructure);
			//����Timer��Channel�ϵ�Preload�Ĵ���
			TIM_OC2PreloadConfig(pwm_x->TIMx, TIM_OCPreload_Enable);
		}
		else if (pwm_x->CHANNEL==3)
		{
			TIM_OC3Init(pwm_x->TIMx, &TIM_OCInitStructure);
			//����Timer��Channel�ϵ�Preload�Ĵ���
			TIM_OC3PreloadConfig(pwm_x->TIMx, TIM_OCPreload_Enable);
		}
		else if (pwm_x->CHANNEL==4)
		{
			TIM_OC4Init(pwm_x->TIMx, &TIM_OCInitStructure);
			//����Timer��Channel�ϵ�Preload�Ĵ���
			TIM_OC4PreloadConfig(pwm_x->TIMx, TIM_OCPreload_Enable);
		}		

		//����Timer��Preload�Ĵ���
		TIM_ARRPreloadConfig(pwm_x->TIMx, ENABLE);

		
		if(pwm_x->TIMx==TIM1||pwm_x->TIMx==TIM8)
		{
		//Only valid to TIM1 or TIM8
    TIM_CtrlPWMOutputs(pwm_x->TIMx, ENABLE);
		}
		else
		{
			;//����
		}
		
		//����Timer
    TIM_Cmd(pwm_x->TIMx, ENABLE);
		//��Timer��ӦChannel����������Ϊ1000��ֹͣ���
		if(pwm_x->CHANNEL==1)
		{
			TIM_SetCompare1(pwm_x->TIMx, 1000);
		}
		else if (pwm_x->CHANNEL==2)
		{
			TIM_SetCompare2(pwm_x->TIMx, 1000);
		}
		else if (pwm_x->CHANNEL==3)
		{
			TIM_SetCompare3(pwm_x->TIMx, 1000);
		}
		else if (pwm_x->CHANNEL==4)
		{
			TIM_SetCompare4(pwm_x->TIMx, 1000);
		}		
}

//�����Զ���GPIO״̬
void Set_User_GPIO(User_GPIO_X* gpio_x, FunctionalState Status)
{	
	//�ж�GPIO�˿�
	if(gpio_x->GPIO_ID==I1)
	{
		gpio_x->GPIOX=GPIOF;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOF;
		gpio_x->GPIO_Pin_x=GPIO_Pin_1;
	}
	else if(gpio_x->GPIO_ID==I2)
	{
		gpio_x->GPIOX=GPIOF;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOF;
		gpio_x->GPIO_Pin_x=GPIO_Pin_0;
	}
	else if(gpio_x->GPIO_ID==J1)
	{
		gpio_x->GPIOX=GPIOE;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOE;
		gpio_x->GPIO_Pin_x=GPIO_Pin_5;
	}
	else if(gpio_x->GPIO_ID==J2)
	{
		gpio_x->GPIOX=GPIOE;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOE;
		gpio_x->GPIO_Pin_x=GPIO_Pin_4;
	}
	else if(gpio_x->GPIO_ID==K1)
	{
		gpio_x->GPIOX=GPIOE;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOE;
		gpio_x->GPIO_Pin_x=GPIO_Pin_6;
	}
	else if(gpio_x->GPIO_ID==K2)
	{
		gpio_x->GPIOX=GPIOE;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOE;
		gpio_x->GPIO_Pin_x=GPIO_Pin_12;
	}
	else if(gpio_x->GPIO_ID==L1)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_2;
	}
	else if(gpio_x->GPIO_ID==L2)
	{
		gpio_x->GPIOX=GPIOB;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOB;
		gpio_x->GPIO_Pin_x=GPIO_Pin_0;
	}
	else if(gpio_x->GPIO_ID==M1)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_3;
	}
	else if(gpio_x->GPIO_ID==M2)
	{
		gpio_x->GPIOX=GPIOB;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOB;
		gpio_x->GPIO_Pin_x=GPIO_Pin_1;
	}
	else if(gpio_x->GPIO_ID==N1)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_4;
	}
	else if(gpio_x->GPIO_ID==N2)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_0;
	}
	else if(gpio_x->GPIO_ID==O1)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_5;
	}
	else if(gpio_x->GPIO_ID==O2)
	{
		gpio_x->GPIOX=GPIOC;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOC;
		gpio_x->GPIO_Pin_x=GPIO_Pin_1;
	}
	else if(gpio_x->GPIO_ID==P1)
	{
		gpio_x->GPIOX=GPIOA;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOA;
		gpio_x->GPIO_Pin_x=GPIO_Pin_5;
	}
	else if(gpio_x->GPIO_ID==P2)
	{
		gpio_x->GPIOX=GPIOA;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOA;
		gpio_x->GPIO_Pin_x=GPIO_Pin_4;
	}
////��17mm΢������ռ��
//	else if(gpio_x->GPIO_ID==Q1)
//	{
//		gpio_x->GPIOX=GPIOF;
//		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOF;
//		gpio_x->GPIO_Pin_x=GPIO_Pin_10;
//	}
	else if(gpio_x->GPIO_ID==Q2)
	{
		gpio_x->GPIOX=GPIOI;
		gpio_x->RCC_AHB1Periph_GPIOX=RCC_AHB1Periph_GPIOI;
		gpio_x->GPIO_Pin_x=GPIO_Pin_9;
	}
	
	//ENABLE���ã�DISABLE����������
	if (Status==ENABLE)
	{
		GPIO_SetBits(gpio_x->GPIOX, gpio_x->GPIO_Pin_x);
	}
	else if (Status==DISABLE)
	{
		GPIO_ResetBits(gpio_x->GPIOX, gpio_x->GPIO_Pin_x);
	}
	else
	{
		GPIO_ResetBits(gpio_x->GPIOX, gpio_x->GPIO_Pin_x);
	}
}

//�����Զ���PWM����
void Set_User_PWM(User_PWM_X* pwm_x, uint32_t pulse_width)
{
			//�ж�PWM�ӿ�
		if(pwm_x->PWM_ID==A)
		{	
			pwm_x->TIMx=TIM5;
			pwm_x->CHANNEL=4;
		}
		else if(pwm_x->PWM_ID==B)
		{	
			pwm_x->TIMx=TIM5;
			pwm_x->CHANNEL=3;
		}
		else if(pwm_x->PWM_ID==C)
		{	
			pwm_x->TIMx=TIM5;
			pwm_x->CHANNEL=2;
		}
		else if(pwm_x->PWM_ID==D)
		{	
			pwm_x->TIMx=TIM5;
			pwm_x->CHANNEL=1;
		}
		else if(pwm_x->PWM_ID==E)
		{	
			pwm_x->TIMx=TIM4;
			pwm_x->CHANNEL=4;
		}
		else if(pwm_x->PWM_ID==F)
		{	
			pwm_x->TIMx=TIM4;
			pwm_x->CHANNEL=3;
		}
		else if(pwm_x->PWM_ID==G)
		{	
			pwm_x->TIMx=TIM4;
			pwm_x->CHANNEL=2;
		}
		else if(pwm_x->PWM_ID==H)
		{	
			pwm_x->TIMx=TIM4;
			pwm_x->CHANNEL=1;
		}
		else if(pwm_x->PWM_ID==S)
		{	
			pwm_x->TIMx=TIM2;
			pwm_x->CHANNEL=1;
		}
		else if(pwm_x->PWM_ID==T)
		{	
			pwm_x->TIMx=TIM2;
			pwm_x->CHANNEL=2;
		}
		else if(pwm_x->PWM_ID==U)
		{	
			pwm_x->TIMx=TIM2;
			pwm_x->CHANNEL=3;
		}
		else if(pwm_x->PWM_ID==V)
		{	
			pwm_x->TIMx=TIM2;
			pwm_x->CHANNEL=4;
		}
		else if(pwm_x->PWM_ID==W)
		{	
			pwm_x->TIMx=TIM8;
			pwm_x->CHANNEL=1;
		}
		else if(pwm_x->PWM_ID==X)
		{	
			pwm_x->TIMx=TIM8;
			pwm_x->CHANNEL=2;
		}
		else if(pwm_x->PWM_ID==Y)
		{	
			pwm_x->TIMx=TIM8;
			pwm_x->CHANNEL=3;
		}
		else if(pwm_x->PWM_ID==Z)
		{	
			pwm_x->TIMx=TIM8;
			pwm_x->CHANNEL=4;
		}
		//��Timer��ӦChannel����������
		if(pwm_x->CHANNEL==1)
		{
			TIM_SetCompare1(pwm_x->TIMx, pulse_width);
		}
		else if (pwm_x->CHANNEL==2)
		{
			TIM_SetCompare2(pwm_x->TIMx, pulse_width);
		}
		else if (pwm_x->CHANNEL==3)
		{
			TIM_SetCompare3(pwm_x->TIMx, pulse_width);
		}
		else if (pwm_x->CHANNEL==4)
		{
			TIM_SetCompare4(pwm_x->TIMx, pulse_width);
		}		
}

