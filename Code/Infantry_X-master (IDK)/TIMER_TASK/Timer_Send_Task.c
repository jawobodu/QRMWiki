#include "Timer_Send_Task.h"


/* ���� */
QueueHandle_t	CAN1_Queue;					//CAN1��Ϣ���о��
QueueHandle_t	CAN2_Queue;					//CAN2��Ϣ���о��

TimerHandle_t	CAN1_Timer_Handle; 			//���ڶ�ʱ�����					
TimerHandle_t	CAN2_Timer_Handle; 			//���ڶ�ʱ�����

/**
  * @brief  �����ʱ�ص�����
  * @param  void
  * @retval void
  * @attention can���н��õȴ�������delay
  */
void CAN1_Timer_Callback( TimerHandle_t xTimer )
{
	CanTxMsg    SendCanTxMsg;

	while(xQueueReceive(CAN1_Queue, &SendCanTxMsg, 0))//���ն�����Ϣ,��ֹ�ȴ�������
	{
//        do
//		{
//			//CAN��������
//			if(CAN1->ESR)
//			{
//				CAN1->MCR |= 0x02;
//				CAN1->MCR &= 0xFD;
//			}
//		}while(!(CAN1->TSR & 0x1C000000));
		
		CAN_Transmit(CAN1, &SendCanTxMsg);//����Ŀ��ֵ
    }
}

/**
  * @brief  �����ʱ�ص�����
  * @param  void
  * @retval void
  * @attention can���н��õȴ�������delay
  */
void CAN2_Timer_Callback( TimerHandle_t xTimer )
{
	CanTxMsg    SendCanTxMsg;

	while(xQueueReceive(CAN2_Queue, &SendCanTxMsg, 0))//���ն�����Ϣ,��ֹ�ȴ�������
	{
//        do
//		{
//			//CAN��������
//			if(CAN2->ESR)
//			{
//				CAN2->MCR |= 0x02;
//				CAN2->MCR &= 0xFD;
//			}
//		}while(!(CAN2->TSR & 0x1C000000));
		
		CAN_Transmit(CAN2, &SendCanTxMsg);//����Ŀ��ֵ
    }
}

void Timer_Send_Create(void)
{
	taskENTER_CRITICAL(); 	//�����ٽ���
	
	/*--------------------------���ݽ���--------------------------*/	

	//����can1���ն���
		//can1���յ��ı��Ĵ���ڴ˶�����
	CAN1_Queue = xQueueCreate( 128, sizeof(CanTxMsg));//���ɱ���64��CanTxMsg
	
	//����can2���ն���
		//can2���յ��ı��Ĵ���ڴ˶�����
	CAN2_Queue = xQueueCreate( 128, sizeof(CanTxMsg));//
	
	//����can1���Ͷ�ʱ��
	 CAN1_Timer_Handle=xTimerCreate((const char*	)"CAN1_Timer",
									 (TickType_t 	)TIME_STAMP_2MS,//2ms
									 (UBaseType_t	)pdTRUE, 		//����ִ��
									 (void *		)0,				//���һ���0
									 (TimerCallbackFunction_t)CAN1_Timer_Callback);//�ص�����
	
	//����CAN1��ʱ��,���ҽ��ܿ���һ�Σ����������������򲻻ᷢ����
	if( CAN1_Timer_Handle != NULL )
	{
		xTimerStart(CAN1_Timer_Handle,0);//���ȴ�
	}
									 
	//����can2���Ͷ�ʱ��
	 CAN2_Timer_Handle=xTimerCreate((const char*	)"CAN2_Timer",
									 (TickType_t 	)TIME_STAMP_1MS,//1ms
									 (UBaseType_t	)pdTRUE, 		//����ִ��
									 (void *		)1,				//���һ���0
									 (TimerCallbackFunction_t)CAN2_Timer_Callback);//�ص�����
	
			
									 
	//����CAN2��ʱ��,���ҽ��ܿ���һ�Σ����������������򲻻ᷢ����
	if( CAN2_Timer_Handle != NULL )
	{
		xTimerStart(CAN2_Timer_Handle,0);//���ȴ�
	}	
	
	taskEXIT_CRITICAL();	//�˳��ٽ���
}
