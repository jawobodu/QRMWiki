#include "main.h"

/**
  * @brief main���������ڳ�ʼ��ִ����ѭ����
  * @param None
  * @retval �ɹ�����0��ʧ�ܷ��ش�����
  */
int main()
{

	AppInit();
	All_Init();

	while (1)
	{
		if (ParaSavingFlag)
		{
			ParametersSave();
			ParaSavingFlag=0;
		}
		delay_ms(1000);
	}
}

