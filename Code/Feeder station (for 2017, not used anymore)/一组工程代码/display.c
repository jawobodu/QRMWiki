#include"display.h"

//sck pd12
//si pd13
//rck  pd14
//  0    1    2    3     4    5   6    7    8    9 
u8 ma[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x27,0x7F,0x6F};//��������

void Tan(u8 dat)//���Ͷ�ѡ�ź�
{
	u8 i;
	for(i=0;i<8;i++)
	{
		GPIO_ResetBits(GPIOD,GPIO_Pin_12);
		if(dat&0x80) 
			GPIO_SetBits(GPIOD,GPIO_Pin_13);
		else 
			GPIO_ResetBits(GPIOD,GPIO_Pin_13);
			dat<<=1;
			GPIO_ResetBits(GPIOD,GPIO_Pin_12);
			GPIO_SetBits(GPIOD,GPIO_Pin_12);
			GPIO_ResetBits(GPIOD,GPIO_Pin_12);
	}	
}
void Spi_Out() //	���������
{
	GPIO_ResetBits(GPIOD,GPIO_Pin_14);
	delayMicroseconds(1);
	GPIO_SetBits(GPIOD,GPIO_Pin_14);
}
void display(u8 a,u8 b,u8 c)//
{
	Tan(ma[a]);
	Tan(ma[b]);
	Tan(ma[c]);
	Spi_Out();
}


