#ifndef _PID_H_
#define _PID_H_

#include "stm32f4xx.h"
#define LIMIT( x,min,max ) ( (x) < (min)  ? (min) : ( (x) > (max) ? (max) : (x) ) )

//typedef struct
//{
//	//PID �������� 
//	double Kp;
//	double Ki;
//	double Kd;
//	//���ַ���
//	int beta;//0���ǲ������ַ��룬 �����Ϊ����ô���ǻ��ַ�Χ 
//	int outbeta;//0���ǲ�������޷��������Ϊ����ô�����޷���Χ
//	//΢������
//	int dif_prior;//0�ǲ���΢������ 
//	//���ٻ��� 
//	//PID���ֵ
//	double last_U; //��һ�ε����ֵ 
//	double delta_U;//����ʽPID
//	double U;//λ��ʽPID
//	//�趨ֵ
//	double s[3]; 
//	//���
//	double e[3];//ÿ�ε����2�����µ�1����һ�ε�0�Ǵ��ϴ�
//	//ʵ��ֵ
//	double r[3];
//	//ǰ������
//	double Kvff; 
//	double Kaff;
//	int full_beta;
//	 
//}PidTypeDef;

typedef struct
{
	//PID �������� 
	float Kp;
	float Ki;
	float Kd;
	float out_kp;
	float out_ki;
	float out_kd;
	float err;
	float last_err;
	float U;

	//
	float   limit_Ki;
	float   limit_U;
	 
}PidTypeDef;


extern PidTypeDef PidDataLocation_rm2006_1;
extern PidTypeDef PidDataLocation_rm2006_2;
extern PidTypeDef PidDataLocation_rm2006_3;
extern PidTypeDef PidDataLocation_rm3508; 
extern PidTypeDef PidDataSpeed_rm2006_1;
extern PidTypeDef PidDataSpeed_rm2006_2;
extern PidTypeDef PidDataSpeed_rm2006_3;
extern PidTypeDef PidDataSpeed_rm3508; 




void PID_InitALL(void);
void PID_Init(PidTypeDef * pid);

void PID_SetParam(PidTypeDef * pid,float p, float i, float d, float limit_Ki , float limit_U)	;
void datalimit(double data , double limit);
float PID_Calc(PidTypeDef * pid, double rel_val, double set_val);


#endif
