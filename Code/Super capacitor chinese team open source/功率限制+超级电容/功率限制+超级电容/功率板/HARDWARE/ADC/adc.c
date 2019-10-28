#include "adc.h"
#include "dma.h"
//��ʼ��ADC
//����ADC1��ͨ��0~15,��Ӧ��ϵ����
//ADC1_CH0-->PA0	
//ADC1_CH1-->PA1	
//ADC1_CH2-->PA2
//ADC1_CH3-->PA3	
//ADC1_CH4-->PA4	
//ADC1_CH5-->PA5	
//ADC1_CH6-->PA6 
//ADC1_CH7-->PA7 
//ADC1_CH8-->PB0 
//ADC1_CH9-->PB1 
//ADC1_CH10-->PC0
//ADC1_CH11-->PC1
//ADC1_CH12-->PC2
//ADC1_CH13-->PC3
//ADC1_CH14-->PC4 
//ADC1_CH15-->PC5 
void Adc_Int(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1,ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);  //����ADC��Ƶ����6  72/6=12  ADC���ʱ�䲻����14M
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;  //ģ����������
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	ADC_DeInit(ADC1);//��λADC1
	
	ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;  //ADC����ģʽ��ADC1 ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode=ENABLE;        //ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode=ENABLE;  //ѭ��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;//ת��������Ȳ����ⲿ����
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel=M;     //˳�����ת��ͨ����
	ADC_Init(ADC1,&ADC_InitStructure);    //����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, ADC_SampleTime_239Cycles5 );	//���ò���ʱ��Ϊ239.5����	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  2, ADC_SampleTime_239Cycles5 );	//���ò���ʱ��Ϊ239.5����
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2,  3, ADC_SampleTime_239Cycles5 );	//���ò���ʱ��Ϊ239.5����
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3,  4, ADC_SampleTime_239Cycles5 );	//���ò���ʱ��Ϊ239.5����	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4,  5, ADC_SampleTime_239Cycles5 );	//���ò���ʱ��Ϊ239.5����	

	ADC_Cmd(ADC1,ENABLE);
	ADC_DMACmd(ADC1,ENABLE);
	
	ADC_ResetCalibration(ADC1);//ʹ�ܸ�λУ׼
	while(ADC_GetCalibrationStatus(ADC1));//�ȴ���λУ׼����
	ADC_StartCalibration(ADC1);//����ADУ׼
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);//�����ADCת��

}









