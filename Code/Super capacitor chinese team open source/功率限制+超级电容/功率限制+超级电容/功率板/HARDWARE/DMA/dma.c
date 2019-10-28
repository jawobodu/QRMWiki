#include "dma.h"
#include "usart.h"	
#include "Util.h"

u8 DMA1_MEM_LEN;
u16 value[N][M]={0}; //�洢ADCת����N*M��������������
u16 aftervalue[M + 1]={0};//�洢M��ͨ����������ƽ��ֵ
u16 adc_offset[M + 1]={0};
utilFilter_t adc_f[2];

//DMA1�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//������ģʽ->�洢��/16λ���ݿ��/�洢������ģʽ
//DMA_CHx:DMAͨ��CHx		   //cpar:�����ַ //cmar:�洢����ַ//cndtr:���ݴ�����
void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{	  
  DMA_InitTypeDef DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMAʱ��
	
    DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

	DMA1_MEM_LEN=cndtr;//ͨ�����ݳ���
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴����跢�͵��ڴ�  DMA_CCRXλ4
	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //�������ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //�ڴ����ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�и����ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA_CHx, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��ADC_DMA_Channel����ʶ�ļĴ���	

	
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//	
//	DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, ENABLE);
	DMA_Cmd(DMA1_Channel1, ENABLE);	//��DMA����
	
}
//#define adc_offset  						3090													//adc�����м�ֵ
#define adc_max									4096													//adc�������ֵ
#define adc_range								8000													//ADC����0-8000mA	
//#define adc_Resolving_power			8.03     //��λadc����ֵ ���ֱ���  3.90  8000/(adc_max-adc_offset)
float adc_Resolving_power[5]={0};
int temp[5]= {0};
void filter(void)
{	
	int sum = 0;//sum ͨ��������ֵ���ܺ�
	u8 count;//
	u8 i=0;
	
	for(i=0;i<M;i++)//ÿ��ͨ������ѭ���˲�
	{
		for(count=0;count<N;count++)//����N��ѭ��
		{
			 sum+=value[count][i];//�����N�β���ֵ���ܺ�
    }
    aftervalue[i] = sum/N;//���ͨ����������ƽ��ֵ*3300/4096*4.545
//		aftervalue[i] = utilFilter(adc_f,aftervalue[i]);
		temp[i]	=	aftervalue[i]-adc_offset[i];
		if(temp[i]<=0)	{temp[i] = 0 ;}
		aftervalue[i]	=	temp[i]*adc_Resolving_power[i];
//		aftervalue[i]	=	(aftervalue[i]-adc_offset)*adc_Resolving_power;
    sum=0;
  }
//	for(i=0;i<M;i++){
//	//		temp	=	aftervalue[i]-adc_offset[i];
////		if(temp<=0)	{temp = 0 ;}
////		aftervalue[i]	=	temp*8;
//	}
}

void filter_correct(void)
{	
	int sum = 0;//sum ͨ��������ֵ���ܺ�
	u8 count;//
	u8 i=0;
	float temp;
	for(i=0;i<M;i++)//ÿ��ͨ������ѭ���˲�
	{
		for(count=0;count<N;count++)//����N��ѭ��
		{
			 sum+=value[count][i];//�����N�β���ֵ���ܺ�
    }
    aftervalue[i] = sum/N;//���ͨ����������ƽ��ֵ*3300/4096*4.545
		adc_offset[i]	=	aftervalue[i];
//		adc_Resolving_power[i]=(4096-adc_offset[i])
//		if(temp<0)	temp=0;
//		aftervalue[i]	=	temp*adc_Resolving_power;
//		aftervalue[i]	=	(aftervalue[i]-adc_offset)*adc_Resolving_power;
    sum=0;
  }
	adc_Resolving_power[0]=8791.0/(4096.0-adc_offset[0]);
	adc_Resolving_power[1]=8081.0/(4096.0-adc_offset[1]);
	adc_Resolving_power[2]=8696.0/(4096.0-adc_offset[2]);
	adc_Resolving_power[3]=8333.0/(4096.0-adc_offset[3]);
}




