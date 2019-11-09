#include "MyDataProcess.h"

//Name:	EncodeFloatDataLost()
//Describe:float�������о��ȶ�ʧ�ؽس�2���ֽڣ�����buff��
//Input:float* f, unsigned char* buff
//Output:None
//Return:None
void EncodeFloatDataLost(float* f, unsigned char* buff)
{
	buff[0] = (u8)(((s16)*f)&0x00ff);
	buff[1] = (u8)(((s16)*f)&0x00ff>>8);
}



//Name:	EncodeFloatData()
//Describe:float�������޾��ȶ�ʧ�ؽس�4���ֽڣ�����buff��
//Input:float* f, unsigned char* buff
//Output:None
//Return:None
void EncodeFloatData(float* f, unsigned char* buff)
{
	u16 f_c[2];
	*(float*)f_c = *f;
	buff[0] = (f_c[0]&0x00ff);
	buff[1] = ((f_c[0]>>8)&0x00ff);
	buff[2] = (f_c[1]&0x00ff);
	buff[3] = ((f_c[1]>>8)&0x00ff);
}

//Name:	DecodeFloatDataLost()
//Describe:����buff��ʼ��2���ֽ�λƴ��ת���ɸ�����
//Input:float* f, unsigned char* buff
//Output:None
//Return:None
void DecodeFloatDataLost(float* f, unsigned char* buff)
{
	*f=(double)((s16)((u16)(buff[1]&0x00ff)<<8)|buff[0])*0.1;
}


//Name:	DecodeFloatData()
//Describe:����buff��ʼ��4���ֽ�λƴ��ת���ɸ�����
//Input:float* f, unsigned char* buff
//Output:None
//Return:None
void DecodeFloatData(float* f, unsigned char* buff)
{
	u16 f_c[2];
	f_c[0]=((buff[1]&0x00ff)<<8)|buff[0];
	f_c[1]=((buff[3]&0x00ff)<<8)|buff[2];
	*f=*(float *)f_c;
}

//Name:	EecodeS16Data()
//Describe:��16λ�з�������ת����2���ֽڷ���buff��
//Input:s16* f, unsigned char* buff
//Output:None
//Return:None
void EncodeS16Data(s16* f, unsigned char* buff)
{
	buff[0] = (uint8_t)(*f);
	buff[1] = (uint8_t)((*f)>>8);
}

//Name:	DecodeS16Data()
//Describe:��buff�е�2���ֽ�ƴ��Ϊs16
//Input:s16* f, unsigned char* buff
//Output:None
//Return:None
void DecodeS16Data(s16* f, unsigned char* buff)
{
	*f = (s16)(((u16)(buff[0])<<8)|buff[1]);
}


//Name:	EecodeU16Data()
//Describe:��16λ�޷�������ת����2���ֽڷ���buff��
//Input:u16* f, unsigned char* buff
//Output:None
//Return:None
void EncodeU16Data(u16* f, unsigned char* buff)
{
	buff[0] = (uint8_t)(*f);
	buff[1] = (uint8_t)((*f)>>8);
}

//Name:	DecodeU16Data()
//Describe:��buff�е�2���ֽ�ƴ��Ϊu16
//Input:u16* f, unsigned char* buff
//Output:None
//Return:None
void DecodeU16Data(u16* f, unsigned char* buff)
{
	*f = ((u16)(buff[1])<<8)|buff[0];
}


//Name:	EecodeS32Data()
//Describe:��32λ�з�������ת����4���ֽڷ���buff��
//Input:s16* f, unsigned char* buff
//Output:None
//Return:None
void EncodeS32Data(s32* f, unsigned char* buff)
{
	u16 f_c[2]={0,0};
	*(s32*)f_c = *f;
	buff[0] = (f_c[0]&0x00ff);
	buff[1] = ((f_c[0]>>8)&0x00ff);
	buff[2] = (f_c[1]&0x00ff);
	buff[3] = ((f_c[1]>>8)&0x00ff);
}

//Name:	DecodeS32Data()
//Describe:��buff�е�4���ֽ�ƴ��Ϊs32
//Input:s32* f, unsigned char* buff
//Output:None
//Return:None
void DecodeS32Data(s32* f, unsigned char* buff)
{
	u16 f_c[2];
	f_c[0]=((buff[1]&0x00ff)<<8)|buff[0];
	f_c[1]=((buff[3]&0x00ff)<<8)|buff[2];
	*f=*(s32 *)f_c;
}

