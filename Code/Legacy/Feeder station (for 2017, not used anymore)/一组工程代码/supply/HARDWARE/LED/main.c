#include "stm32f4xx.h"
#include "main.h"
#include "delay.h"
#include "can1.h"
#include "can2.h"
#include "variable_init.h"
#include "TIM2.h"
#include "duty.h"
#include "pid.h"
#include "usart1.h"
#include "value_allinit.h"
#include "pon_limit.h"
#include "photoelectric_init.h"
#include "TIM8_pwm.h"
#include "solenoid_value.h"
#include "TIM3.h"


//�ش�ʱ�����ã� stm32f4xx.h��98�е�HSE_VALUE 25000000�ĳ�12000000
//��system_stm32f4xx.c��155�е�PLL_M ��Ϊ 12
//��������ԭ���ǺͰ������ⲿ������ƥ��


int main()
{
	SystemInit();
	power_gpio_Configuration();
	CAN1_Configuration();
	CAN2_Configuration();
	Tim2_Configuration();
	Tim3_Configuration();
	USART1_Configuration(100000);
	PID_InitALL();
	pon_limit_Init();
	photoelectric_init();
  TIM8_PWM_Configuration();  //�����ʼ��
	
	//������ʱ��
	delay_us(5000);
	
  //���̸�λ���������
//	chassis_Motor_tar[0] = 100;
//	chassis_Motor_tar[1] = 100;
//	chassis_Motor_tar[2] = 100;
//	chassis_Motor_tar[3] = 100;	
//	externmoto_tar[0] = 100;
//	externmoto_tar[1] = 100;
//	externmoto_tar[2] = 100;
//	externmoto_tar[3] = 100;
//	pickmotor_extern_tar = 100;
//	pickmotor_rotate_tar = 100;
//	chassis_position_reset();
	
	//��������ѭ����
	while(1)
	{
    Loop();//�ڶԻ� ������ϣ��׶Ի�
	}
	
}


//void chassis_position_reset()
//{
//  while(1)
//	{
//		pon_detect_circle();
//		if(Tim_circle_flag == 1) 
//	  {
//			if(key[0] == 0)
//			{
//        externmoto_tar[0] = 0;
//			}
//			if(key[1] == 0)
//			{
//        externmoto_tar[1] = 0;
//			}
//			if(key[2] == 0)
//			{
//        externmoto_tar[2] = 0;
//			}
//			if(key[3] == 0)
//			{
//        externmoto_tar[3] = 0;
//	  	Send_motorvalue_CAN2(  PID_Calc(&PidDataSpeed_chassis_1 , chassis_Motor0x201[1] , chassis_Motor_tar[0]) , 
//														 PID_Calc(&PidDataSpeed_chassis_2 , chassis_Motor0x202[1] , chassis_Motor_tar[1]) ,
//														 PID_Calc(&PidDataSpeed_chassis_3 , chassis_Motor0x203[1] , chassis_Motor_tar[2]) ,
//														 PID_Calc(&PidDataSpeed_chassis_4 , chassis_Motor0x204[1] , chassis_Motor_tar[3]));
//						}

//			Send_motorvalue_CAN1_4(PID_Calc(&PidDataSpeed_extern_1 , extern_Motor0x201[1] , externmoto_tar[0]) ,
//														 PID_Calc(&PidDataSpeed_extern_2 , extern_Motor0x202[1] , externmoto_tar[1]) ,
//														 PID_Calc(&PidDataSpeed_extern_3 , extern_Motor0x203[1] , externmoto_tar[2]) ,
//														 PID_Calc(&PidDataSpeed_extern_4 , extern_Motor0x204[1] , externmoto_tar[3]));

//			Send_motorvalue_CAN1_2(PID_Calc(&PidDataSpeed_pick_rotate_1 , pick_rotateMotor_1[1] , pickmotor1_rotate_tar) ,
//														 PID_Calc(&PidDataSpeed_pick_rotate_2 , pick_rotateMotor_2[1] , pickmotor2_rotate_tar) ,
//														 0 , 0);
////			
////      if(externmoto_tar[0] == 0 && externmoto_tar[1] == 0 && externmoto_tar[2] == 0 && externmoto_tar[3] == 0)
////			{
////		    chassis_reset_flag = 1;
////			}
//			
//			if(chassis_reset_flag)
//			{
//				break;
//			}
//			
//		}
//	}
//	
//}



 