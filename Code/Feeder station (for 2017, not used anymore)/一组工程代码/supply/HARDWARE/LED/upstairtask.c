#include "upstairtask.h"

int up_flag = 0 , down_flag = 0;
int Photoelectric_detect1 , Photoelectric_detect2 , Photoelectric_detect3 , Photoelectric_detect4 , Photoelectric_detect5 , Photoelectric_detect6;



void upstair_task()
{
	//�˱�־�����л�ң�ػ��Զ����ϵ�ǰΪң��ģʽ
	if(remote_or_autoflag == remote_mode)
	{
		chassisMove();//���̿���
	}
	
	//�ϵ�:
	if(up_flag)
	{
		//��һ������������̧�������ִ򿪣�
		if(move_flag1 == 0)
		{
			externmotor_locationset[0] = uping_locationmax;                                                      
			externmotor_locationset[1] = uping_locationmax;
			externmotor_locationset[2] = uping_locationmax;
			externmotor_locationset[3] = uping_locationmax;
			//������ŷ���������
			move_flag1 = 1;
			movement_cnt++;  // movement_cnt == 2;
		}
		
		//ǰ��������̨�ף���⵽̨��
		if(Photoelectric_detect1 == 1 && Photoelectric_detect2 == 1 && movement_cnt == 2)
		{
			move_flag2 = 1;                                              
		}
		
		//ǰ�����ϵ�һ��̨�׺�ִ���˶�������ǰ��̧��
		if(move_flag2)
		{
			//ǰ����������,���ֲ���
			externmotor_locationset[0] = uping_locationreset;
			externmotor_locationset[1] = uping_locationreset;
			externmotor_locationset[2] = uping_locationmax;
			externmotor_locationset[3] = uping_locationmax;
			
			//��ʱ����ɲס
			remote_or_autoflag = auto_mode;  //��ʱ�л����Զ�ģʽ 
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag2 = 0;
			movement_cnt++;  // movement_cnt == 3;
		}
					
		//������������̶̹�ʱ��ǰ���ѳ���̨��
		if((externmoto_location[0] <= uping_locationreset + 2000) && (externmoto_location[1] <= uping_locationreset + 2000) && movement_cnt == 3)
		{
      move_flag3 = 1;
		}
		
		//����������
		if(move_flag3)
		{
			//1000ת���ٶ��ǹ���ֵ��ת�ٽ�С
			chassis_Motor_tar[0] = -1000;
			chassis_Motor_tar[1] =  1000; 
			chassis_Motor_tar[2] =  1000;
			chassis_Motor_tar[3] = -1000;
			move_flag3 = 0;
			movement_cnt++;  // movement_cnt == 4;
		}
		
		//�����ֿ���̨��ʱ���������������⵽̨��
		if(Photoelectric_detect3 == 1 && Photoelectric_detect4 == 1 && movement_cnt == 4)
		{  
      move_flag4 = 1;			
		}
		
		//����ɲס������̧��
		if(move_flag4)
		{
			//ǰ�ֲ���������̧��
			externmotor_locationset[0] = uping_locationreset;
			externmotor_locationset[1] = uping_locationreset;
			externmotor_locationset[2] = uping_locationreset;
			externmotor_locationset[3] = uping_locationreset;
			
			//��ʱ����ɲס
			remote_or_autoflag = auto_mode;  //��ʱ�л����Զ�ģʽ 
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag4 = 0;
			movement_cnt++; // movement_cnt == 5;
		}
		
		//������̧�������̣������ѳ���̨�ף�����ǰ��
		if((externmotor_locationset[2] <= uping_locationreset + 2000) && (externmotor_locationset[3] <= uping_locationreset + 2000) && movement_cnt == 5)
		{
      move_flag5 = 1;
		}
		
		//����ǰ��
		if(move_flag5)
		{
			chassis_Motor_tar[0] = -1000;
			chassis_Motor_tar[1] =  1000; 
			chassis_Motor_tar[2] =  1000;
			chassis_Motor_tar[3] = -1000;
			move_flag5 = 0;
			movement_cnt++; // movement_cnt == 6;
		}
		
		//�������������ⴥ����������ȫ�ϵ�һ��̨��
		if(Photoelectric_detect5 == 1 && Photoelectric_detect6 == 1 && movement_cnt == 6)
		{
			move_flag6 = 1;
		}
		
		//���ĵ�������̧����׼���ϵڶ���̨��
		if(move_flag6)
		{
			//����̧��
			externmotor_locationset[0] = uping_locationmax;                                                      
			externmotor_locationset[1] = uping_locationmax;
			externmotor_locationset[2] = uping_locationmax;
			externmotor_locationset[3] = uping_locationmax;
			
			//��ɲס
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag6 = 0;
			movement_cnt++; //movement_cnt == 7
		}
		
		//����̧�������
		if(externmotor_locationset[0] >= uping_locationmax - 2000 && externmotor_locationset[1] >= uping_locationmax - 2000 && externmotor_locationset[2] >= uping_locationmax - 2000 && externmotor_locationset[3] >= uping_locationmax - 2000 && movement_cnt == 7)           
		{
			move_flag7 = 1;
		}
		
		//����̧������ߣ����̵������������ǰ��
		if(move_flag7)
		{
			chassis_Motor_tar[0] = -1000;
			chassis_Motor_tar[1] =  1000; 
			chassis_Motor_tar[2] =  1000;
			chassis_Motor_tar[3] = -1000;
			move_flag7 = 0;
			movement_cnt++; // movement_cnt == 8;
		}
		
		//ǰ��������̨�ף���⵽̨��
		if(Photoelectric_detect1 == 1 && Photoelectric_detect2 == 1 && movement_cnt == 8)
		{
			move_flag8 = 1;                                              
		}
		
		//ǰ��̧��
		if(move_flag8)
		{
			//ǰ���������̣����ֲ���
			externmotor_locationset[0] = uping_locationreset;
			externmotor_locationset[1] = uping_locationreset;
			externmotor_locationset[2] = uping_locationmax;
			externmotor_locationset[3] = uping_locationmax;
			
			//��ɲס
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag8 = 0;
			movement_cnt++; // movement_cnt == 9;
		}
		
		//������������̶̹�ʱ��ǰ���ѳ���̨��
		if((externmoto_location[0] <= uping_locationreset + 2000) && (externmoto_location[1] <= uping_locationreset + 2000) && movement_cnt == 9)
		{
      move_flag9 = 1;
		}
		
		//������
		if(move_flag9)
		{
			chassis_Motor_tar[0] = -1000;
			chassis_Motor_tar[1] =  1000; 
			chassis_Motor_tar[2] =  1000;
			chassis_Motor_tar[3] = -1000;
			move_flag9 = 0;
			movement_cnt++; // movement_cnt == 10;
		}
			
		//�����ֿ���̨��ʱ���������������⵽̨��
		if(Photoelectric_detect3 == 1 && Photoelectric_detect4 == 1 && movement_cnt == 10)
		{  
      move_flag10 = 1;			
		}
		
		//����ɲס������̧��
		if(move_flag10)
		{
			//ǰ�ֲ���������̧��
			externmotor_locationset[0] = uping_locationreset;
			externmotor_locationset[1] = uping_locationreset;
			externmotor_locationset[2] = uping_locationreset;
			externmotor_locationset[3] = uping_locationreset;
			
			//��ʱ����ɲס
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag10 = 0;
			movement_cnt++; // movement_cnt == 11;
		}
 
		//������̧�������̣������ѳ���̨�ף�����ǰ��
		if((externmotor_locationset[2] <= uping_locationreset + 2000) && (externmotor_locationset[3] <= uping_locationreset + 2000) && movement_cnt == 11)
		{
      move_flag11 = 1;
		}
		
		//����ǰ��
		if(move_flag11)
		{
			chassis_Motor_tar[0] = -1000;
			chassis_Motor_tar[1] =  1000; 
			chassis_Motor_tar[2] =  1000;
			chassis_Motor_tar[3] = -1000;
			move_flag11 = 0;
			movement_cnt++; // movement_cnt == 12;
		}
		
		//�������������ⴥ����������ȫ�ϵڶ���̨��
		if(Photoelectric_detect5 == 1 && Photoelectric_detect6 == 1 && movement_cnt == 12)
		{
			move_flag12 = 1;
		}
		
		//�ϵ�֮�󣬳�ɲס
		if(move_flag12)
		{
			//��ʱ����ɲס
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag12 = 0;
			movement_cnt++; // movement_cnt == 13;
		}
		
		//�ϵ��������л���ң��ģʽ
		if(movement_cnt == 13)
		{
			up_flag = 0;
			move_flag1 = 0;
			movement_cnt = 1;
			remote_or_autoflag = remote_mode;
		}
	}
	
	//�µ���
	if(down_flag)
	{
		
		//��һ�������������½�����ͣ�׼���µ�
		if(move_flag1 == 0)
		{
			externmotor_locationset[0] = uping_locationreset;                                                      
			externmotor_locationset[1] = uping_locationreset;
			externmotor_locationset[2] = uping_locationreset;
			externmotor_locationset[3] = uping_locationreset;
			//������ŷ���������
			move_flag1 = 1;
			movement_cnt++;  // movement_cnt == 2;
		}
		
		//���ּ�⵽��һ��̨�ף���ʼ����
		if(Photoelectric_detect3 == 1 && Photoelectric_detect4 == 1 && movement_cnt == 2)
		{
			move_flag2 = 1;                                              
		}
		
		//��������
		if(move_flag2)
		{
			//ǰ����������,���ֲ���
			externmotor_locationset[0] = uping_locationreset;
			externmotor_locationset[1] = uping_locationreset;
			externmotor_locationset[2] = uping_locationmax;
			externmotor_locationset[3] = uping_locationmax;
			
			//��ʱ����ɲס
			remote_or_autoflag = auto_mode;  //��ʱ�л����Զ�ģʽ 
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag2 = 0;
			movement_cnt++;  // movement_cnt == 3;
		}
					
		//�������½�����һ��̨��
		if((externmoto_location[2] >= uping_locationmax - 2000) && (externmoto_location[3] >= uping_locationmax - 2000) && movement_cnt == 3)
		{
      move_flag3 = 1;
		}
		
		//����������
		if(move_flag3)
		{
			//1000ת���ٶ��ǹ���ֵ��ת�ٽ�С
			chassis_Motor_tar[0] =  1000;
			chassis_Motor_tar[1] = -1000; 
			chassis_Motor_tar[2] = -1000;
			chassis_Motor_tar[3] =  1000;
			move_flag3 = 0;
			movement_cnt++;  // movement_cnt == 4;
		}
		
		//�����ֿ���̨��ʱ���������������⵽̨��
		if(Photoelectric_detect1 == 1 && Photoelectric_detect2 == 1 && movement_cnt == 4)
		{  
      move_flag4 = 1;			
		}
		
		//����ɲס��ǰ������
		if(move_flag4)
		{
			//���ֲ�����ǰ������
			externmotor_locationset[0] = uping_locationmax;
			externmotor_locationset[1] = uping_locationmax;
			externmotor_locationset[2] = uping_locationmax;
			externmotor_locationset[3] = uping_locationmax;
			
			//��ʱ����ɲס
			remote_or_autoflag = auto_mode;  //��ʱ�л����Զ�ģʽ 
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag4 = 0;
			movement_cnt++; // movement_cnt == 5;
		}
		
		//���½�����һ��̨��ʱ
		if((externmotor_locationset[0] >= uping_locationmax - 2000) && (externmotor_locationset[1] >= uping_locationmax - 2000) && movement_cnt == 5)
		{
      move_flag5 = 1;
		}
		
		//������
		if(move_flag5)
		{
			chassis_Motor_tar[0] =  1000;
			chassis_Motor_tar[1] = -1000; 
			chassis_Motor_tar[2] = -1000;
			chassis_Motor_tar[3] =  1000;
			move_flag5 = 0;
			movement_cnt++; // movement_cnt == 6;
		}
		
		//�������������ⴥ���������˶�����һ��̨�ױ�Ե
		if(Photoelectric_detect5 == 1 && Photoelectric_detect6 == 1 && movement_cnt == 6)
		{
			move_flag6 = 1;
		}
		
		//���ĵ��������½���׼���µ�
		if(move_flag6)
		{
			//����̧��
			externmotor_locationset[0] = uping_locationreset;                                                      
			externmotor_locationset[1] = uping_locationreset;
			externmotor_locationset[2] = uping_locationreset;
			externmotor_locationset[3] = uping_locationreset;
			
			//��ɲס
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag6 = 0;
			movement_cnt++; //movement_cnt == 7
		}
		
		//����̧�������
		if(externmotor_locationset[0] <= uping_locationreset + 2000 && externmotor_locationset[1] <= uping_locationreset + 2000 && externmotor_locationset[2] <= uping_locationreset + 2000 && externmotor_locationset[3] <= uping_locationreset + 2000 && movement_cnt == 7)           
		{
			move_flag7 = 1;
		}
		
		//���̽�����ͣ�������
		if(move_flag7)
		{
			chassis_Motor_tar[0] =  1000;
			chassis_Motor_tar[1] = -1000; 
			chassis_Motor_tar[2] = -1000;
			chassis_Motor_tar[3] =  1000;
			move_flag7 = 0;
			movement_cnt++; // movement_cnt == 8;
		}
		
		//ǰ��������̨�ף���⵽̨��
		if(Photoelectric_detect3 == 1 && Photoelectric_detect4 == 1 && movement_cnt == 8)
		{
			move_flag8 = 1;                                              
		}
		
		//�����½�
		if(move_flag8)
		{
			//ǰ���������̣����ֲ���
			externmotor_locationset[0] = uping_locationreset;
			externmotor_locationset[1] = uping_locationreset;
			externmotor_locationset[2] = uping_locationmax;
			externmotor_locationset[3] = uping_locationmax;
			
			//��ɲס
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag8 = 0;
			movement_cnt++; // movement_cnt == 9;
		}
		
		//�������µ�����ʱ
		if((externmoto_location[2] >= uping_locationmax + 2000) && (externmoto_location[3] >= uping_locationmax + 2000) && movement_cnt == 9)
		{
      move_flag9 = 1;
		}
		
		//������
		if(move_flag9)
		{
			chassis_Motor_tar[0] =  1000;
			chassis_Motor_tar[1] = -1000; 
			chassis_Motor_tar[2] = -1000;
			chassis_Motor_tar[3] =  1000;
			move_flag9 = 0;
			movement_cnt++; // movement_cnt == 10;
		}
			
		//��ǰ���ּ�⵽����ʱ
		if(Photoelectric_detect1 == 1 && Photoelectric_detect2 == 1 && movement_cnt == 10)
		{  
      move_flag10 = 1;			
		}
		
		//����ɲס��ǰ���½�
		if(move_flag10)
		{
			//ǰ�ֲ���������̧��
			externmotor_locationset[0] = uping_locationmax;
			externmotor_locationset[1] = uping_locationmax;
			externmotor_locationset[2] = uping_locationmax;
			externmotor_locationset[3] = uping_locationmax;
			
			//��ʱ����ɲס
			chassis_Motor_tar[0] = 0;
			chassis_Motor_tar[1] = 0; 
			chassis_Motor_tar[2] = 0;
			chassis_Motor_tar[3] = 0;
			move_flag10 = 0;
			movement_cnt++; // movement_cnt == 11;
		}
 
		//��ǰ���µ�����ʱ�����Ѿ���ȫ�µ�
		if((externmotor_locationset[0] >= uping_locationmax - 2000) && (externmotor_locationset[2] >= uping_locationmax - 2000) && movement_cnt == 11)
		{
      move_flag11 = 1;
		}
		
		//�µ�֮���л�Ϊң��ģʽ
		if(move_flag11)
		{
			down_flag = 0;
			move_flag11 = 0;
			movement_cnt = 1;
			remote_or_autoflag = remote_mode;
		}

	}
	
}


































