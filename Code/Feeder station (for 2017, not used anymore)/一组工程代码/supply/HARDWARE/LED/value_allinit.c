#include "value_allinit.h"


//��ʱ���ڱ�־��
int Tim_circle_flag_1ms = 1;
int Tim_circle_flag_2ms = 1;
int Tim_circle_flag_3ms = 1;

//�����������
s16 rm2006_1[4] , rm2006_1_offset , rm2006_1_cnt , rm2006_1_last;
s16 rm2006_2[4] , rm2006_2_offset , rm2006_2_cnt , rm2006_2_last;
s16 rm2006_3[4] , rm2006_3_offset , rm2006_3_cnt , rm2006_3_last;
s16 rm3508[4]   , rm3508_offset , rm3508_cnt , rm3508_last;

//�������Ԥ��ת�٣�
int rm2006_1_tar = 6000 , rm2006_3_tar = 6000;//6000����

//3508���ת��λ�ã�
float box_motor_location;
float box_motor_locationset;
float box_motor_locationmax = 1110000;//����г�

//���ӿ���2006���
float open_or_close_motorlocation;
float open_or_close_motorlocationset;
float open_or_close_motorlocationmax = 1000000;

//©����1 ��ʮ�����ޱ�־
int bullet_outer1_limit50_flag = 0;
//©����2 ��ʮ�����ޱ�־
int bullet_outer2_limit50_flag = 0;

//������©����1���굯��־��
int infantry_supply1_confirmflag = 0;
//������©����2���굯��־��
int infantry_supply2_confirmflag = 0;


//λ��pid�����־��
s16 location_calc_flag = 0;

//�ٶ�pid�����־��
s16 speed_calc_flag = 1;

//pid�����־��
s16 pidcalc_startflag_can1 = 0 , pidcalc_startflag_can2 = 0;
 
//��λ���ر�־��
u8 key[6];

//��翪�ر�־��
u8 light[4] ;

//����PID�м䴫�ݲ���
int speed_set_1 = 0 , speed_set_2 = 0 , speed_set_3 = 0 , speed_set_4 = 0;
float set_1 = 0 , set_2 = 0 ,set_3 = 0;

//���̸�λ��־��
u8 chassis_reset_flag = 1;

//����ģʽ��
u8 Remote_Mode = 0;

//��ֹң����ʧ��:
int usart_cnt = 1;
int last_usart_cnt = 1;
int remote_detect_flag = 1;
int cnt_bais = 0;
int danger_cnt = 0;

   






