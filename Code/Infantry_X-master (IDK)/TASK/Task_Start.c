#include "Task_Start.h"

#include "main.h"
#include "Timer_Send_Task.h"
#include "Task_Chassis.h"
#include "Task_Gimbal.h"
#include "Task_Revolver.h"

#include "remote.h"
#include "control.h"
#include "magazine.h"
#include "friction.h"
#include "MPU_Temperature.h"
#include "super_cap.h"
#include "judge.h"
#include "usart5.h"
#include "led.h"


//���񴴽�
#define START_TASK_PRIO  11  				//�������ȼ�
TaskHandle_t StartTask_Handler; 			//������
void Start_Task(void *pvParameters); 		//������

/*ʧ�ر���*/
#define TASK_Control_Protect_PRIO 2  		//�������ȼ�,ʧ��������
TaskHandle_t  Task_Control_Protect_Handler; //������
void Task_Control_Protect(void *pvParameters);

/*����*/
#define TASK_Chassis_PRIO 3  				//�������ȼ�
TaskHandle_t  Task_Chassis_Handler; 		//������

/*��̨*/
#define TASK_Gimbal_PRIO 4 					//�������ȼ�
TaskHandle_t  Task_Gimbal_Handler; 			//������

/*����*/
#define TASK_REVOLVER_PRIO 5  				//�������ȼ�
TaskHandle_t  Task_Revolver_Handler; 		//������

#define TASK_10ms_PRIO 6  					//�������ȼ�
TaskHandle_t  Task_10ms_Handler; 			//������
void Task_10ms(void *pvParameters); 		//������

#define TASK_500ms_PRIO 10  					//�������ȼ�
TaskHandle_t  Task_500ms_Handler; 			//������
void Task_500ms(void *pvParameters); 		//������

#define TASK_OUTCTRL 7  					//�������ȼ�
TaskHandle_t  Task_OutCtl_Handler; 			//������
void Task_OutCtl(void *pvParameters); 		//������

/***********************************************************/
void App_Task_Create(void)
{
	xTaskCreate((TaskFunction_t )Start_Task, 			//������
				(const char* )"Start_Task", 			//��������
				(uint16_t )STK_SIZE_128, 				//�����ջ��С
				(void* )NULL, 							//���ݸ��������Ĳ���
				(UBaseType_t )START_TASK_PRIO, 			//�������ȼ�
				(TaskHandle_t* )&StartTask_Handler); 	//������

}


/**
  * @brief  ������ʼ����������
  * @param  void
  * @retval void
  * @attention ���������ڴ˴���
  */
void Start_Task(void *pvParameters)
{
		
	taskENTER_CRITICAL(); 	//�����ٽ���
								 
	/*------------------------------------------------*/
	//���� ���� ����ʵʱ���񣬾�����ʱ
	xTaskCreate((TaskFunction_t )Task_Chassis,
				(const char* )"Task_Chassis",
				(uint16_t )STK_SIZE_128,
				(void* )NULL,
				(UBaseType_t )TASK_Chassis_PRIO, 
				(TaskHandle_t* )&Task_Chassis_Handler);
				
	//���� ��̨ ����ʵʱ���񣬾�����ʱ
	xTaskCreate((TaskFunction_t )Task_Gimbal,
				(const char* )"Task_Gimbal",
				(uint16_t )STK_SIZE_128,
				(void* )NULL,
				(UBaseType_t )TASK_Gimbal_PRIO,
				(TaskHandle_t* )&Task_Gimbal_Handler);
				
	//���� ���� ����ʵʱ���񣬾�����ʱ
	xTaskCreate((TaskFunction_t )Task_Revolver,
				(const char* )"Task_Revolver",
				(uint16_t )STK_SIZE_128,
				(void* )NULL,
				(UBaseType_t )TASK_REVOLVER_PRIO,
				(TaskHandle_t* )&Task_Revolver_Handler);
				
	//���� 10ms ����
	xTaskCreate((TaskFunction_t )Task_10ms,
				(const char* )"Task_10ms",
				(uint16_t )STK_SIZE_128,
				(void* )NULL,
				(UBaseType_t )TASK_10ms_PRIO,
				(TaskHandle_t* )&Task_10ms_Handler);
				
	//���� 500ms ����
	xTaskCreate((TaskFunction_t )Task_500ms,
				(const char* )"Task_500ms",
				(uint16_t )STK_SIZE_128,
				(void* )NULL,
				(UBaseType_t )TASK_500ms_PRIO,
				(TaskHandle_t* )&Task_500ms_Handler);
				
	//����ʧ�ر�������
	xTaskCreate((TaskFunction_t )Task_Control_Protect,
				(const char* )"Task_Control_Protect",
				(uint16_t )STK_SIZE_128,
				(void* )NULL,
				(UBaseType_t )TASK_Control_Protect_PRIO,
				(TaskHandle_t* )&Task_Control_Protect_Handler);
				
	//���� ʧ�ؿ��� ����
	xTaskCreate((TaskFunction_t )Task_OutCtl,
				(const char* )"Task_OutCtl",
				(uint16_t )STK_SIZE_128,
				(void* )NULL,
				(UBaseType_t )TASK_OUTCTRL,
				(TaskHandle_t* )&Task_OutCtl_Handler);

				
	vTaskDelay(500);
	vTaskSuspend(StartTask_Handler); 					//ɾ����ʼ����			

	taskEXIT_CRITICAL(); 								//�˳��ٽ���		
}

//ÿ10msִ��һ��������
void Task_10ms(void *pvParameters)
{
	for(;;)
	{	
		vTaskDelay(TIME_STAMP_10MS);				//10ms
//���벿��		
		Magazine_Ctrl();		//���ֿ���
		FRICTION_Ctrl();//Ħ���ֿ���	
		
		Super_Charging_Control();
		SuperCap_Giveout_Control();		
	}
}

//ÿ500msִ��һ��������
void Task_500ms(void *pvParameters)
{
	for(;;)
	{	
		vTaskDelay(TIME_STAMP_200MS);				//500ms
//���벿��		
		Send_to_Teammate();
	}
}

//ʧ�ؿ��������������ÿ4msִ��һ�Σ�����ӳ�
void Task_OutCtl(void *pvParameters)
{
	for(;;)
	{	
		vTaskDelay(TIME_STAMP_4MS);				//4ms
//���벿��		
		SYSTEM_OutCtrlProtect( );//ʧ�ر���
	}
}

//ÿ50msִ��һ��������
void Task_Control_Protect(void *pvParameters)
{
	static portTickType currentTime;	
	
	for(;;)
	{	
		//���벿��	
		currentTime = xTaskGetTickCount();	//��ȡ��ǰϵͳʱ��	
		
//		Tempeture_PID();//����������¶�Ư������
		Green_Off;
//����vTaskSuspend�����ǲ����ۼƵģ���ʹ��ε���vTaskSuspend ()������һ���������Ҳֻ�����һ��vTaskResume ()��������ʹ���������������״̬��
		if(currentTime >= REMOTE_ulGetLostTime( ) || REMOTE_IfDataError() == TRUE )//ң��ʧ��ʱ��̫��
		{
			vTaskSuspend(Task_Chassis_Handler);		//���������
			vTaskSuspend(Task_Gimbal_Handler);
			vTaskSuspend(Task_Revolver_Handler);
			vTaskSuspend(Task_10ms_Handler);
			
			vTaskResume(Task_OutCtl_Handler);//���ʧ�ر�����������
		}
		else 
		{
			vTaskResume(Task_Chassis_Handler);		//�ָ�����
			vTaskResume(Task_Gimbal_Handler);
			vTaskResume(Task_Revolver_Handler);
			vTaskResume(Task_10ms_Handler);
			
			vTaskSuspend(Task_OutCtl_Handler);//����ʧ�ر�����������
		}
		vTaskDelayUntil(&currentTime, TIME_STAMP_50MS);//������ʱ50ms
	}
}

