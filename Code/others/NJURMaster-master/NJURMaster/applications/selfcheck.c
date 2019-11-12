#include "main.h"
u32 SelfCheckErrorFlag=0x0000;
//selfcheck task is a 5ms task
const u16 SelfCheckInitValue[SELF_CHECK_ITEM_NUM]={200u,200u,200u,200u,200u,200u,200u,200u,200u,200u,100u,20u,100u};
u16 SelfCheckValue[SELF_CHECK_ITEM_NUM]={200u,200u,200u,200u,200u,200u,200u,200u,200u,200u,100u,20u,100u};
/**
  * @brief ���Ź���ʼ��
  * @param None
  * @retval None
  */
void DogInit(void)
{
	int i=0;
	for (i=0;i<SELF_CHECK_ITEM_NUM;i++)
	{
		SelfCheckValue[i]=0;
	}
	SelfCheckErrorFlag=0x0000;
} 

/**
  * @brief ι��
  * @param None
  * @retval None
  */
void FeedDog(u8 _dog_index)
{
	SelfCheckValue[_dog_index]=0;
	SelfCheckErrorFlag &= (~(u32)(1<<_dog_index));
}

/**
  * @brief �ж�ϵͳ����Щ�豸���ߣ�ʧȥ���ӣ�
  * @param None
  * @retval None
  */
void CheckDog(void)
{
	u8 i=0;
	for (i=0;i<SELF_CHECK_ITEM_NUM;i++)
	{
		SelfCheckValue[i]++;
		if (SelfCheckValue[i]>=SelfCheckInitValue[i])
		{
			SelfCheckErrorFlag|=(u32)(1<<i);
			SelfCheckValue[i]=0;
		}
	}
	if (ParamSavedFlag==1)
	{
		FeedDog(DEVICE_INDEX_NOCALI);
	}

}

/**
  * @brief ���ĳ�豸��ǰ��״̬
  * @param �豸��������ͷ�ļ��ҵ�
  * @retval ���豸���ߣ�����1�����򷵻�0
  */
u8 IsDeviceLost(int _dog_index)
{
	return (((u32)(1<<_dog_index))==(SelfCheckErrorFlag&((u32)(1<<_dog_index))));
}
