#include "spi.h"

void SPI5_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//ʹ��GPIOFʱ��
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI5, ENABLE);//ʹ��SPI5ʱ��
 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;//PF6
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOF, &GPIO_InitStructure);//��ʼ��
	
  //GPIOFB3,4,5��ʼ������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_9|GPIO_Pin_8;//PB7-9���ù������	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOF, &GPIO_InitStructure);//��ʼ��
	
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource7,GPIO_AF_SPI5); //PF7����Ϊ SPI5
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource9,GPIO_AF_SPI5); //PF8����Ϊ SPI5
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource8,GPIO_AF_SPI5); //PF9����Ϊ SPI5
 
	//����ֻ���SPI�ڳ�ʼ��
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI5,ENABLE);//��λSPI5
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI5,DISABLE);//ֹͣ��λSPI5

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 10;	//CRCֵ����Ķ���ʽ
	SPI_Init(SPI5, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
 
	SPI_Cmd(SPI5, ENABLE); //ʹ��SPI����

//	SPI5_ReadWriteByte(0xff);//��������		 
}   
//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   
//SPI_BaudRatePrescaler_8   8��Ƶ   
//SPI_BaudRatePrescaler_16  16��Ƶ  
//SPI_BaudRatePrescaler_256 256��Ƶ 
  
void SPI5_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//�ж���Ч��
	SPI5->CR1&=0XFFC7;//λ3-5���㣬�������ò�����
	SPI5->CR1|=SPI_BaudRatePrescaler;	//����SPI5�ٶ� 
	SPI_Cmd(SPI5,ENABLE); //ʹ��SPI5
} 

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI5_ReadWriteByte(u8 TxData)
{		
  while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_TXE) == RESET){}//�ȴ���������  
	
	SPI_I2S_SendData(SPI5, TxData); //ͨ������SPIx����һ��byte  ����
		
  while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_RXNE) == RESET){} //�ȴ�������һ��byte  
 
	return SPI_I2S_ReceiveData(SPI5); //����ͨ��SPIx������յ�����						    
}


void ImuSPI5_ReadData(uint8_t address, uint8_t* pdat, uint16_t dataLength)
{
   GPIO_ResetBits(GPIOF,GPIO_Pin_6);
	//while(SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_BSY)!=RESET){;}
	address = address| 0x80;
	
	SPI5_ReadWriteByte(address);
	while(dataLength--)
	{
		*pdat = SPI5_ReadWriteByte(0xff);
		pdat++;
	}
   GPIO_SetBits(GPIOF,GPIO_Pin_6);
}	




























