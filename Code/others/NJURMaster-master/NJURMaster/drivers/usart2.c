#include "main.h"

/*******����2����*********************/
u8 Rx_2_Buf[256];	
u8 Tx2Buffer[256];
u8 Tx2Counter=0;
u8 count2=0; 
u8 Tx2DMABuffer[256]={0};
/***********************************/

/**
  * @brief ����2��ʼ��
  * @param BaudRate
  * @retval None
  * @details	BaudRate	115200
	*						ʹ��DMA���ͣ�RXNE�����ж�
	*						TX	PD5			PA2
	*						RX	PD6			PA3
  */
void Usart2_Init(u32 br_num)
{
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);	
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 

	USART_InitStructure.USART_BaudRate = br_num;     
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; 
	USART_InitStructure.USART_StopBits = USART_StopBits_1; 
	USART_InitStructure.USART_Parity = USART_Parity_No; 
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; 
	USART_ClockInitStruct.USART_Clock = USART_Clock_Disable; 
	USART_ClockInitStruct.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStruct.USART_CPHA = USART_CPHA_2Edge; 
	USART_ClockInitStruct.USART_LastBit = USART_LastBit_Disable;
	
	USART_Init(USART2, &USART_InitStructure);
	USART_ClockInit(USART2, &USART_ClockInitStruct);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE); 
	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);

	DMA_DeInit(DMA1_Stream6);
	
	while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE){}//�ȴ�DMA������ 
	
  /* ���� DMA Stream */
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;  //ͨ��ѡ��
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;//DMA�����ַ
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)Tx2DMABuffer;//DMA �洢��0��ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//�洢��������ģʽ
  DMA_InitStructure.DMA_BufferSize = 0;//���ݴ����� 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//���������ģʽ
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�洢������ģʽ
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݳ���:8λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�洢�����ݳ���:8λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// ʹ����ͨģʽ 
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//�е����ȼ�
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//�洢��ͻ�����δ���
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//����ͻ�����δ���
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);//��ʼ��DMA Stream
	
}

/**
  * @brief ����2�жϳ���
  * @param None
  * @retval None
  * @details RXNE�жϣ�ÿ���յ�һ���ֽڼ�����һ���жϣ��������������
  */
void USART2_IRQHandler(void)
{
	u8 com_data;
	if( USART_GetITStatus(USART2,USART_IT_RXNE) )				//�����ж�
	{
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
		com_data = USART2->DR;
		Usart2_DataPrepare(com_data);											//���ݽ���
	}

}

/**
  * @brief ����2��DMA���ͺ���������һ������
  * @param DataToSend Ҫ�������ݵ������ָ��
	* @param data_num Ҫ���͵����ݵĸ���
  * @retval None
  */
void Usart2_Send(unsigned char *DataToSend ,u8 data_num)
{
	u8 i;
	static uint16_t num=0;
	static u8 len=0;
	
	DMA_Cmd(DMA1_Stream6, DISABLE);
	DMA_ClearFlag(DMA1_Stream6,DMA_FLAG_TCIF6);//���DMA2_Steam7������ɱ�־
	num = DMA_GetCurrDataCounter(DMA1_Stream6);
	for(i=0;i<data_num;i++)
	{
		Tx2Buffer[count2++] = *(DataToSend+i);
	}
	for (i=0;i<(u8)num;i++)
	{
		Tx2DMABuffer[i]=Tx2Buffer[((u8)(len-num+i))];
	}
	for (;i<(u8)(num+data_num);i++)
	{
		Tx2DMABuffer[i]=*(DataToSend+i-num);
	}
	len=count2;
	while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE){}	//ȷ��DMA���Ա�����  
	DMA1_Stream6->NDTR = (uint16_t)(num+data_num);          //���ݴ�����  
	DMA_Cmd(DMA1_Stream6, ENABLE);       
}


