#ifndef __DMA_H
#define __DMA_H
#include "sys.h"
#include "Util.h"
#define N 20 //ȡ20��ƽ��ֵ
#define M 4 //5·AD

void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);
void filter(void);
void filter_correct(void);
extern u16 value[N][M];  //�洢ADCת����M*N��������������
extern u16 aftervalue[M + 1];//�洢M��ͨ����������ƽ��ֵ
extern u16 adc_offset[M + 1];
extern float all_value;
extern utilFilter_t adc_f[2];
#endif




