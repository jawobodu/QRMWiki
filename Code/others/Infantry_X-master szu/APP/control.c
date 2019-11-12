#include "control.h"

#include "remote.h"
#include "laser.h"
#include "Task_Chassis.h"
#include "Task_Gimbal.h"
#include "Task_Revolver.h"

#include "magazine.h"
#include "friction.h"
#include "super_cap.h"
#include "adda.h"

/**********************ϵͳ�����жϼ�����******************/


//����ģʽ
eRemoteMode remoteMode = RC;

//ϵͳ״̬
eSystemState systemState = SYSTEM_STARTING;


/**
  * @brief  ϵͳ����
  * @param  void
  * @retval void
  * @attention 
  */
void SYSTEM_Reset( void )
{
	systemState = SYSTEM_STARTING;
}

/**
  * @brief  ʧ�ر���
  * @param  void
  * @retval void
  * @attention 
  */
void SYSTEM_OutCtrlProtect(void)
{
    SYSTEM_Reset();//ϵͳ�ָ�������״̬
	REMOTE_vResetData();//ң�����ݻָ���Ĭ��״̬
	
	
	Laser_Off;//�����
	CHASSIS_StopMotor();//���̹�
	GIMBAL_StopMotor();//��̨��
	Magazine_StopCtrl();//���ֹͣת��
	REVOLVER_StopMotor();//����ֹͣת��
	FRICTION_StopMotor();//Ħ���ֹ�
	Super_Cap_StopCtrl();//���ݹرճ�ŵ�
}

/**
  * @brief  ����ϵͳ״̬
  * @param  void
  * @retval void
  * @attention 1kHz,��LOOPѭ������
  */
void SYSTEM_UpdateSystemState(void)
{
	static uint32_t  ulInitCnt  =  0;
	
	if (systemState == SYSTEM_STARTING)
	{
		ulInitCnt++;

		if (ulInitCnt > 2500)//������ʱ,1ms*2k=2s,Ϊ�˸�MPU����ʱ��
		{
			ulInitCnt = 0;

			systemState = SYSTEM_RUNNING;//�������,ת������ͨģʽ
		}
	}
}

/**
  * @brief  ����ģʽѡ��
  * @param  void
  * @retval void
  * @attention ���̻����,���ڴ��Զ���ģʽѡ��ʽ,1msִ��һ��
  */
void SYSTEM_UpdateRemoteMode( void )
{ 
    if (IF_RC_SW2_UP)
	{
		remoteMode = KEY;
	}
	
	else
	{
		remoteMode = RC;
	}
}


//���ؿ���ģʽ
eRemoteMode SYSTEM_GetRemoteMode( )
{
	return remoteMode;
}

//����ϵͳ״̬
eSystemState SYSTEM_GetSystemState( )
{
	return systemState;
}




