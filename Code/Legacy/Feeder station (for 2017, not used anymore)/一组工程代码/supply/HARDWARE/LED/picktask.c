#include "picktask.h"

int pick_flag = 0 , receive_flag = 0 , push_confirm_flag = 0;
 

//void picktask()
//{
//	//�˱�־�����л�ң�ػ��Զ����ϵ�ǰΪң��ģʽ
//	if(remote_or_autoflag == remote_mode)
//	{
//		chassisMove();//���̿���
//	}
//	
//	if(pick_flag)  //ȡ����־
//	{
//		if(move_flag1 == 0)
//		{
//			//���ȡ��������
//			pickmotor_locationset_extern  = extern_locationmax;
//			
//			//����̧����
//			externmotor_locationset[0] = uping_locationmax;                                                      
//			externmotor_locationset[1] = uping_locationmax;
//			externmotor_locationset[2] = uping_locationmax;
//			externmotor_locationset[3] = uping_locationmax;
//			move_flag1 = 1;
//			movement_cnt++;
//		}
//		
//		//��ʱ���ֶ���λ
//		if(move_flag2 == 0 && movement_cnt == 2)
//		{
//			if(push_confirm_flag) //���¿�ʼץȡ
//			{
//				//�����½�:
//				externmotor_locationset[0] = uping_locationreset;                                                      
//				externmotor_locationset[1] = uping_locationreset;
//				externmotor_locationset[2] = uping_locationreset;
//				externmotor_locationset[3] = uping_locationreset;
//				
//				//�����½������
//				if((externmoto_location[0] <= uping_locationreset + 2000) && (externmoto_location[1] <= uping_locationreset + 2000) && (externmoto_location[2] <= uping_locationreset + 2000) && (externmoto_location[3] <= uping_locationreset + 2000))
//				{
//					//����һ��ֹͣ��������ȡ��ҩ�䲢����
//					//������ŷ���������ȡ
//					
//					//���ڼ���Ҫ��ʱ����1s����ȡ��Ҫʱ��
//					
//					//Ȼ�����������γ���ҩ��
//					externmotor_locationset[0] = uping_locationmax;                                                      
//					externmotor_locationset[1] = uping_locationmax;
//					externmotor_locationset[2] = uping_locationmax;
//					externmotor_locationset[3] = uping_locationmax;
//					move_flag2 = 1;
//					movement_cnt++;//movement_cnt == 3;
//				}		
//			}
//		}
//		
//		//�����������
//		if((externmoto_location[0] >= uping_locationmax - 2000) && (externmoto_location[1] >= uping_locationmax - 2000) && (externmoto_location[2] >= uping_locationmax - 2000) && (externmoto_location[3] >= uping_locationmax - 2000))
//		{
//			move_flag3 = 1;
//		}
//		
//		//�ջص�ҩ�䲢���ӵ�
//		if(move_flag3 == 1 && movement_cnt == 3)
//		{
//			pickmotor_locationset_extern = extern_locationreset;
//			
//			//������λ����
//			
//			if(1)//�ڿ����ת�������ӵ�
//			{
//				pickmotor_locationset_rotate = rotate_locationmax;
//				
//				move_flag1 = 0;
//				move_flag2 = 0;
//				move_flag3 = 0;
//				pick_flag  = 0;
//				movement_cnt = 1;
//			}
//		}
//		
//	}
//	
//	
//	
//}





