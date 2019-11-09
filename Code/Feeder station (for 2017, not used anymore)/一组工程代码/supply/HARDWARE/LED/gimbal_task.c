#include "gimbal_task.h"


void gimbal_task(void)
{
  gimbal_yawmotor_locationset = GimbalRef.yaw_speed_ref * gimbal_movepara * 4 + gimbal_yawmotor_locationinit;
	gimbal_pitchmotor_locationset = GimbalRef.pitch_speed_ref * gimbal_movepara + gimbal_pitchmotor_locationinit;
	
		//��̨���λ�û���
	set_1 =               -PID_Calc(&PidDataLocation_gimbal_yaw , gimbal_yaw_Motor[0] , gimbal_yawmotor_locationset);
	set_2 =               -PID_Calc(&PidDataLocation_gimbal_pitch , gimbal_pitch_Motor[0] , gimbal_pitchmotor_locationset);
	//��̨����ٶȻ���  yaw������ֵ���Ϊ��������ʱ��ת   pitch������ֵ���Ϊ������˳ʱ��ת
	//��������ٶȻ���
	if(RC_CtrlData.rc.s2 != remote_down)
	{
		Send_motorvalue_CAN1_2(PID_Calc(&PidDataSpeed_gimbal_yaw    , gimbal_speed_yaw   , set_1) ,//PID_Calc(&PidDataSpeed_gimbal_yaw    , gimbal_speed_yaw   , set_1)
													 PID_Calc(&PidDataSpeed_gimbal_pitch  , gimbal_speed_pitch , set_2) ,//PID_Calc(&PidDataSpeed_gimbal_pitch  , gimbal_speed_pitch , set_2)
													 PID_Calc(&PidDataSpeed_trigger_motor , trigger_Motor[1]   , trigger_motor_speedtar) ,   //set_3ҪΪ��������
													 0);  
	}
}