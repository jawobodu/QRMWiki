#include "supply_task.h"

//©����1����С�ӵ �    �����Ӵ�����������ʱ���ʱ 
int smallbullet_out1_timecnt = 1 , smallbullet_out2_timecnt = 1 ;
extern int bullet_cnt_1 , bullet_cnt_2;

//����������
void infantry_supply()
{
	//��߲���
	bullet_outer_1();
	if(key[2] == 0 || key[3] == 0) 
	{
		smallbullet_out1_timecnt++;
		if(smallbullet_out1_timecnt >= 100)
		{
			//ֹͣ����
			speed_set_1 = 0;
			//��������
			bullet_cnt_1 = 0;
			//ȷ���Ѿ�����
			infantry_supply1_confirmflag = 1;
			smallbullet_out1_timecnt = 100;
			//�򿪵���
			TIM2->CCR2 = 2400; //��
		}
	}
	else  
	{
		if(infantry_supply1_confirmflag == 1)
		{
			infantry_supply1_confirmflag = 0;
			bullet_outer1_limit50_flag = 0;
		}
		TIM2->CCR2 = 2000; //��
		smallbullet_out1_timecnt = 0;
	}
	
	////�ұ߲���:
	bullet_outer_2();
	if(key[4] == 0 || key[5] == 0) 
	{
		smallbullet_out2_timecnt++;
		if(smallbullet_out2_timecnt >= 100)
		{
			//ֹͣ����
			speed_set_3 = 0;
			//��������
			bullet_cnt_2 = 0;
			//ȷ���Ѿ�����
			infantry_supply2_confirmflag = 1;
			smallbullet_out2_timecnt = 100;
			TIM2->CCR1 = 2500; //��
		}
	}
	else  
	{
		if(infantry_supply2_confirmflag == 1)
		{
			infantry_supply2_confirmflag = 0;
			bullet_outer2_limit50_flag = 0;
			bullet_cnt_2 = 0;
		}
		TIM2->CCR1 = 2150; //��
		smallbullet_out2_timecnt = 0;
	}
}


//���̷ŵ�
int engineer_detect_flag = 0 , engineer_detect_cnt = 0;
int supply_box_uping_flag = 0;
int hero_supply_flag = 0;
int box_down_and_close_flag = 0;
void engineer_to_hero_supply()
{
	if(light[0] == 0 && engineer_detect_flag == 0)
	{
		engineer_detect_cnt++;
		if(engineer_detect_cnt >= 200)
		{
			engineer_detect_flag = 1;//��⵽���̳�
		}
	}
	///////////
	if(engineer_detect_flag == 1 && supply_box_uping_flag == 0)
	{
		supply_box_uping_flag = 1;//��ʼ��ʱ  -> TIM3
	}
	///////
	if(supply_box_uping_flag == 2)
	{
		//��������:
		box_motor_locationset = box_motor_locationmax;
		
		if(box_motor_location >= box_motor_locationmax - 30000 && hero_supply_flag == 0)
		{
			hero_supply_flag = 1;
		}
	}
	/////////
	//׼����Ӣ�۲�����
	if(hero_supply_flag == 1)
	{
		if(light[1] == 0)//��⵽Ӣ��
		{
			TIM4->CCR3 = 2200;
			open_or_close_motorlocationset = open_or_close_motorlocationmax;
			/////////////////////////
			engineer_detect_flag = 0;
			supply_box_uping_flag = 0;
			hero_supply_flag = 0;
			/////
			box_down_and_close_flag = 1;//����׼���½�
		}
	}
}


