#include "main.h"

u8 SysMode=SYS_PREPARESTATE;
u8 ControlMode=MC_NORMAL;
/**
  * @brief ģʽ�л�
  * @param None
  * @retval None
  */
void WorkStateFSM(u32 sys)
{
	
	if (SysMode!=SYS_STOPSTATE&&SysMode!=SYS_CALISTATE)
	{
		if (GetRcMode()==RC_KEY_STOP)
		{
			SysMode=SYS_STOPSTATE;
			return;
		}
		if (ParamSavedFlag!=1)
		{
			SysMode=SYS_CALISTATE;
			return;
		}
		if (sys<SYS_PREPARETIME)
		{
			SysMode=SYS_PREPARESTATE;
			return;
		}

		SysMode=SYS_NORMALSTATE;
	}
	if (SysMode==SYS_STOPSTATE)
	{
		if (GetRcMode()!=RC_KEY_STOP)
		{
			//Soft Reset
			NVIC_SystemReset();							
			return;
		}
	}
}

/**
  * @brief ���ص�ǰģʽ
  * @param None
  * @retval ��ǰģʽ
  */
u8 GetWSCurrent(void)
{
	return SysMode;
}
