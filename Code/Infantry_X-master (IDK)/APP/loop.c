#include "loop.h"




//��ѭ��
void Loop(void)
{	
	static uint32_t currentTime = 0;
	
	static uint32_t ulLoopTime_STOPMOTOR = 0;//ʧ�ر���
	
	static uint32_t loopTime_1ms = 0;
	static uint32_t loopTime_2ms = 0;
	static uint32_t loopTime_4ms = 0;
	static uint32_t loopTime_10ms = 0;
	static uint32_t loopTime_50ms = 0;
	static uint32_t loopTime_100ms = 0;

	
	currentTime = micros();	//��ȡ��ǰϵͳʱ��
	
	//ʧ�ر���,ͨ����ȡϵͳʱ���봮��2�ж��л�ȡ��ϵͳʱ�����Ƚ�
	if (currentTime >= REMOTE_ulGetLostTime( ))//ң��ʧ��,��ʧ���Ļ�Ӧ��һֱ����С�ڵ�״̬(ʱ���ΪREMOTE_LOST_TIME)
	{
		if (currentTime >= ulLoopTime_STOPMOTOR)
		{
			ulLoopTime_STOPMOTOR = currentTime + 4000;//4ms
			
			SYSTEM_OutCtrlProtect( );//ʧ�ر���
		}
		//�˳�ѭ��
		return;
	}
	else if (REMOTE_IfDataError() == TRUE      //ң�ؽ������ݳ���
		       || REMOTE_IfKeyReset() == TRUE )     //�ֶ�����
	{	
		//����оƬ
		delay_ms(1);
		__set_FAULTMASK(1);
		NVIC_SystemReset();			
	}

	
	
	/**********************����ѭ��******************************/
	
	if((int32_t)(currentTime - loopTime_1ms) >= 0)  
	{			
		loopTime_1ms = currentTime + 1000;	//1ms
		
		GIMBAL_UpdatePalstance();//������̨��ֵ
		Vision_UpdatePalstance();//�����Ӿ���ֵ
		SYSTEM_UpdateRemoteMode();//����ң��/����ģʽ
		SYSTEM_UpdateSystemState();//����ϵͳ״̬,����������״̬
		
		REVOLVER_Ctrl();//���̵������,�ϸ�Ŵ�ִ��,���򿨵�ʱ���жϲ�׼
	}
	
	if((int32_t)(currentTime - loopTime_2ms) >= 0)  
	{			
		loopTime_2ms = currentTime + 2000;	//2ms

		CHASSIS_Ctrl();		//���̿���
		GIMBAL_Ctrl();		//��̨����
	}
	
	if((int32_t)(currentTime - loopTime_4ms) >= 0)  
	{			
		loopTime_4ms = currentTime + 4000;	//4ms
		
		Judge_Read_Data(Judge_Buffer);		//��ȡ����ϵͳ����	
	}
	
	if((int32_t)(currentTime - loopTime_10ms) >= 0)  
	{			
		loopTime_10ms = currentTime + 10000;	//10ms

//		Magazine_Ctrl();		//���ֿ���
		FRICTION_Ctrl();//Ħ���ֿ���
	}
	
	if((int32_t)(currentTime - loopTime_50ms) >= 0) 
	{
		loopTime_50ms = currentTime + 50000;	 //50ms	
		
//		Tempeture_PID();//����������¶�Ư������
	}	
	
	if((int32_t)(currentTime - loopTime_100ms) >= 0)  
	{			
		loopTime_100ms = currentTime + 100000;	//100ms
//		JUDGE_Show_Data();//�û������ϴ�,10Hz�ٷ������ٶ�
		Vision_Ctrl();//�Ӿ�,ָ�����
	}

}
