#ifndef GPIO_H
#define GPIO_H

#include "main.h"
#include "stm32f4xx_gpio.h"
typedef enum
{
	I1=1,//PF1	��ֵΪ1������NULL=0��ͻ
	I2,	//PF0
	J1,	//PE5
	J2,	//PE4
	K1,	//PE6
	K2,	//PE12
	L1,	//PC2
	L2,	//PB0
	M1,	//PC3
	M2,	//PB1
	N1,	//PC4
	N2,	//PC0
	O1,	//PC5
	O2,	//PC1
	P1,	//PA5
	P2,	//PA4
//	Q1, //PF10����17mm΢������ռ��
	Q2,//PI9
//	R1, GND
//	R2,	+5V
} GPIO_ID_E;

typedef enum
{
	A=1,//PI0 	TIM5_CH4	��ֵΪ1������NULL=0��ͻ
	B,	//PH12	TIM5_CH3
	C,	//PH11	TIM5_CH2
	D,	//PH10	TIM5_CH1
	E,	//PD15	TIM4_CH4
	F,	//PD14	TIM4_CH3
	G,	//PD13	TIM4_CH2
	H,	//PD12	TIM4_CH1
	
	S,	//PA0		TIM2_CH1		
	T,	//PA1		TIM2_CH2
	U,	//PA2		TIM2_CH3
	V,	//PA3		TIM2_CH4
	W,	//PI5		TIM8_CH1
	X,	//PI6		TIM8_CH2
	Y,	//PI7		TIM8_CH3
	Z,	//PI2		TIM8_CH4
} PWM_ID_E;

typedef struct
{
	GPIO_ID_E GPIO_ID;//GPIO��������ĸ
	GPIO_TypeDef * GPIOX;//GPIO����
	uint16_t GPIO_Pin_x;//GPIO�ܽ�
	uint32_t RCC_AHB1Periph_GPIOX;//GPIO�˿�AHB����
} User_GPIO_X;

typedef struct
{
	PWM_ID_E PWM_ID;//PWM��������ĸ
	GPIO_TypeDef * GPIOX;//GPIO����
	TIM_TypeDef * TIMx;//��ʱ��
	uint16_t GPIO_Pin_x;//GPIO�ܽ�
	uint32_t RCC_AHB1Periph_GPIOX;//GPIO�˿�AHB����
	uint32_t RCC_APBxPeriph_TIMx;//GPIOʱ��APB����
	uint32_t CHANNEL;//GPIOʱ��ͨ��
	uint8_t GPIO_PinSourcex;
	uint8_t GPIO_AF_TIMx;//GPIO���ü�ʱ��
} User_PWM_X;

extern void User_GPIO_Init(User_GPIO_X* gpio_x);//��ʼ���Զ���GPIO�˿ں͹ܽ�
extern void User_PWM_Init(User_PWM_X* pwm_x);//��ʼ���Զ���PWM�˿ڣ��ܽź�ʱ��
extern void Set_User_GPIO(User_GPIO_X* gpio_x, FunctionalState Status);//�����Զ���GPIO״̬
extern void Set_User_PWM(User_PWM_X* pwm_x, uint32_t pulse_width);//�����Զ���PWM����
extern User_GPIO_X user_gpio;
extern User_PWM_X user_pwm;
#endif
