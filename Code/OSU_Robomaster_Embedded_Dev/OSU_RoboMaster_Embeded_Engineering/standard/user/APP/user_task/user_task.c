/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       user_task.c/h
  * @brief      *һ����ͨ������������豸�޴����̵�1Hz��˸,Ȼ���ȡ��̬��*
  *             ���̳�������GPIO��PWM�����������
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

#include "User_Task.h"
#include "main.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "led.h"

#include "Detect_Task.h"
#include "INS_Task.h"


//�Զ���
#include "gimbal_task.h"
#include "filter.h"
#include "CAN_Receive.h"
#include "chassis_task.h"
#include "gpio.h"

//�����λTrigger
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "core_cmFunc.h"


#define user_is_error() toe_is_error(errorListLength)

#if INCLUDE_uxTaskGetStackHighWaterMark
uint32_t UserTaskStack;
#endif

//��̬�� ��λ��
fp32 angle_degree[3] = {0.0f, 0.0f, 0.0f};


//�����λTrigger
extern void SoftReset(void)
{
	__set_FAULTMASK(1);//�ر������жϣ��Է���λ�����
	NVIC_SystemReset();//��λ
}





void UserTask(void *pvParameters)
{

		const volatile fp32 *angle;
		//��ȡ��̬��ָ��
		angle = get_INS_angle_point();
	
    while (1)
    {

        //��̬�� ��rad ��� �ȣ����������̬�ǵĵ�λΪ�ȣ������ط�����̬�ǣ���λ��Ϊ����
        angle_degree[0] = (*(angle + INS_YAW_ADDRESS_OFFSET)) * 57.3f;
        angle_degree[1] = (*(angle + INS_PITCH_ADDRESS_OFFSET)) * 57.3f;
        angle_degree[2] = (*(angle + INS_ROLL_ADDRESS_OFFSET)) * 57.3f;

        if (!user_is_error())
        {
            led_green_on();
        }
				
				//����GPIO
				GPIO_ID_E GPIO_ID_LIST[17]={I1,I2,J1,J2,K1,K2,L1,L2,M1,M2,N1,N2,O1,O2,P1,P2,Q2};//������Ҫ�����Ķ˿�
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
					Set_User_GPIO(&user_gpio,ENABLE);//ENABLE����GPIO�˿�
					}
				}
								
				vTaskDelay(1);//ÿ��1msѭ��һ��
//        led_green_off();
//        vTaskDelay(500);
#if INCLUDE_uxTaskGetStackHighWaterMark
        UserTaskStack = uxTaskGetStackHighWaterMark(NULL);
#endif
    }
}
