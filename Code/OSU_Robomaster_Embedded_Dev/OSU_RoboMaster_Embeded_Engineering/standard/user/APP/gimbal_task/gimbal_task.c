/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       gimbal_task.c/h
  * @brief      ��ɻ�е�ۿ�������ARM1��ARM2һͬ���У�ARM3��ARM4һͬ����
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. ���
	*  V2.0.0			May-18-2019			OSU-RM					2. ���
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2016 DJI****************************
  */

#include "Gimbal_Task.h"

#include "main.h"

#include "arm_math.h"
#include "gimbal_behaviour.h"
#include "user_lib.h"
#include "INS_Task.h"
#include "remote_control.h"
#include "shoot.h"
#include "CAN_Receive.h"
#include "Detect_Task.h"
#include "pid.h"

#include "buzzer.h"//������������ڲ���
#include "filter.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

//�����λTrigger
void SoftReset(void);

//�������ֵ���� 0��8191
#define ECD_Format(ecd)         \
    {                           \
        if ((ecd) > ecd_range)  \
            (ecd) -= ecd_range; \
        else if ((ecd) < 0)     \
            (ecd) += ecd_range; \
    }

#define gimbal_total_pid_clear(gimbal_clear)                                                   \
    {                                                                                          \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_arm1_motor.gimbal_motor_absolute_angle_pid);   \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_arm1_motor.gimbal_motor_relative_angle_pid);   \
        PID_clear(&(gimbal_clear)->gimbal_arm1_motor.gimbal_motor_gyro_pid);                    \
			\
			PID_clear(&(gimbal_clear)->gimbal_arm2_motor.gimbal_motor_rpm_pid);  	\
                                                                                               \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_arm2_motor.gimbal_motor_absolute_angle_pid); \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_arm2_motor.gimbal_motor_relative_angle_pid); \
        PID_clear(&(gimbal_clear)->gimbal_arm2_motor.gimbal_motor_gyro_pid);                  \
			\
			PID_clear(&(gimbal_clear)->gimbal_arm2_motor.gimbal_motor_rpm_pid);  \
    }

#if INCLUDE_uxTaskGetStackHighWaterMark
uint32_t gimbal_high_water;
#endif

//��̨���������������
static Gimbal_Control_t gimbal_control;


//���͵�can ָ��
static int16_t Arm1_Can_Set_Current = 0, Arm2_Can_Set_Current = 0, Arm3_Can_Set_Current = 0, Arm4_Can_Set_Current = 0;

//��̨��ʼ��
static void GIMBAL_Init(Gimbal_Control_t *gimbal_init);
//��̨pid����
static void Gimbal_PID_clear(Gimbal_PID_t *gimbal_pid_clear);
//��̨״̬����
static void GIMBAL_Set_Mode(Gimbal_Control_t *gimbal_set_mode);
//��̨���ݸ���
static void GIMBAL_Feedback_Update(Gimbal_Control_t *gimbal_feedback_update);
//��̨״̬�л��������ݣ������������״̬�л���������״̬����Ŀ��ֵ
static void GIMBAL_Mode_Change_Control_Transit(Gimbal_Control_t *gimbal_mode_change);
//������̨��������ֵ����ԽǶ�
static fp32 motor_ecd_to_angle_change(uint16_t ecd, uint16_t offset_ecd);
//��̨����pid����
static void GIMBAL_Control_loop(Gimbal_Control_t *gimbal_control_loop);



static void gimbal_motor_relative_angle_control_arm1(Gimbal_Motor_t *gimbal_motor);//��е��1���
static void gimbal_motor_relative_angle_control_arm2(Gimbal_Motor_t *gimbal_motor);//��е��2���
static void gimbal_motor_relative_angle_control_arm3(Gimbal_Motor_t *gimbal_motor);//��е��3���
static void gimbal_motor_relative_angle_control_arm4(Gimbal_Motor_t *gimbal_motor);//��е��4���

static void gimbal_motor_raw_angle_control(Gimbal_Motor_t *gimbal_motor);


static void GIMBAL_PID_Init(Gimbal_PID_t *pid, fp32 maxout, fp32 intergral_limit, fp32 kp, fp32 ki, fp32 kd);
static fp32 GIMBAL_PID_Calc(Gimbal_PID_t *pid, fp32 get, fp32 set, fp32 error_delta);



static void calc_gimbal_cali(const Gimbal_Cali_t *gimbal_cali, uint16_t *yaw_offset, uint16_t *pitch_offset, fp32 *max_yaw, fp32 *min_yaw, fp32 *max_pitch, fp32 *min_pitch);
#if GIMBAL_TEST_MODE
//j-scope ����pid����
static void J_scope_gimbal_test(void);
#endif

void GIMBAL_task(void *pvParameters)
{
    //�ȴ������������������������
    vTaskDelay(GIMBAL_TASK_INIT_TIME);
    //��̨��ʼ��
    GIMBAL_Init(&gimbal_control);

//�ر��Լ�    //�жϵ���Ƿ�����
//    while (toe_is_error(YawGimbalMotorTOE) || toe_is_error(PitchGimbalMotorTOE) || toe_is_error(TriggerMotorTOE) )
//    {
        vTaskDelay(10*GIMBAL_CONTROL_TIME);
        GIMBAL_Feedback_Update(&gimbal_control);             //��̨���ݷ���
//    }

    while (1)	
    {
				
				
				
        GIMBAL_Set_Mode(&gimbal_control);                    //������̨����ģʽ

        GIMBAL_Mode_Change_Control_Transit(&gimbal_control); //����ģʽ�л� �������ݹ���
						
        GIMBAL_Feedback_Update(&gimbal_control);             //��̨���ݷ���
				
        GIMBAL_Control_loop(&gimbal_control);                //��̨����PID����


			
			
#if ARM1_TURN
        Arm1_Can_Set_Current = -gimbal_control.gimbal_arm1_motor.given_current;
#else
        Arm1_Can_Set_Current = gimbal_control.gimbal_arm1_motor.given_current;
#endif

#if ARM2_TURN
        Arm2_Can_Set_Current = -gimbal_control.gimbal_arm2_motor.given_current;
#else
        Arm2_Can_Set_Current = gimbal_control.gimbal_arm2_motor.given_current;
#endif

#if ARM3_TURN
        Arm3_Can_Set_Current = -gimbal_control.gimbal_arm3_motor.given_current;
#else
        Arm3_Can_Set_Current = gimbal_control.gimbal_arm3_motor.given_current;
#endif
				
#if ARM4_TURN
        Arm4_Can_Set_Current = -gimbal_control.gimbal_arm4_motor.given_current;
#else
        Arm4_Can_Set_Current = gimbal_control.gimbal_arm4_motor.given_current;
#endif
				
				
        //��е����ң��������״̬��relax ״̬��canָ��Ϊ0����ʹ��current����Ϊ��ķ������Ǳ�֤ң��������һ��ʹ����ֹ̨ͣ
        if (!(toe_is_error(Arm1MotorTOE) && toe_is_error(Arm2MotorTOE) && toe_is_error(Arm3MotorTOE) && toe_is_error(Arm4MotorTOE)))
        {
            if (toe_is_error(DBUSTOE))
            {
                CAN_CMD_ARM(0, 0, 0, 0);
            }
            else
            {
                CAN_CMD_ARM(Arm1_Can_Set_Current, Arm2_Can_Set_Current, Arm3_Can_Set_Current, Arm4_Can_Set_Current);
            }
        }

				
				
#if GIMBAL_TEST_MODE
        J_scope_gimbal_test();
#endif

        vTaskDelay(GIMBAL_CONTROL_TIME);

#if INCLUDE_uxTaskGetStackHighWaterMark
        gimbal_high_water = uxTaskGetStackHighWaterMark(NULL);
#endif
    }
}


/**
  * @brief          ��̨У׼���ã���У׼����̨��ֵ�Լ���С����е��ԽǶ�
  * @author         RM
  * @param[in]      yaw ��ֵ
  * @param[in]      pitch ��ֵ
  * @param[in]      yaw �����ԽǶ�
  * @param[in]      yaw ��С��ԽǶ�
  * @param[in]      pitch �����ԽǶ�
  * @param[in]      pitch ��С��ԽǶ�
  * @retval         ���ؿ�
  * @waring         �������ʹ�õ�gimbal_control ��̬�������º�������������ͨ��ָ�븴��
  */
void set_cali_gimbal_hook(const uint16_t yaw_offset, const uint16_t pitch_offset, const fp32 max_yaw, const fp32 min_yaw, const fp32 max_pitch, const fp32 min_pitch)
{
    gimbal_control.gimbal_arm1_motor.offset_ecd = yaw_offset;
    gimbal_control.gimbal_arm1_motor.max_relative_angle = max_yaw;
    gimbal_control.gimbal_arm1_motor.min_relative_angle = min_yaw;

    gimbal_control.gimbal_arm2_motor.offset_ecd = pitch_offset;
    gimbal_control.gimbal_arm2_motor.max_relative_angle = max_pitch;
    gimbal_control.gimbal_arm2_motor.min_relative_angle = min_pitch;
}


/**
  * @brief          ��̨У׼���㣬��У׼��¼����� ��Сֵ ��������̨ ��ֵ�������С��е�Ƕ�
  * @author         RM
  * @param[in]      yaw ��ֵ ָ��
  * @param[in]      pitch ��ֵ ָ��
  * @param[in]      yaw �����ԽǶ� ָ��
  * @param[in]      yaw ��С��ԽǶ� ָ��
  * @param[in]      pitch �����ԽǶ� ָ��
  * @param[in]      pitch ��С��ԽǶ� ָ��
  * @retval         ����1 ����ɹ�У׼��ϣ� ����0 ����δУ׼��
  * @waring         �������ʹ�õ�gimbal_control ��̬�������º�������������ͨ��ָ�븴��
  */
bool_t cmd_cali_gimbal_hook(uint16_t *yaw_offset, uint16_t *pitch_offset, fp32 *max_yaw, fp32 *min_yaw, fp32 *max_pitch, fp32 *min_pitch)
{
    if (gimbal_control.gimbal_cali.step == 0)
    {
        gimbal_control.gimbal_cali.step = GIMBAL_CALI_START_STEP;
        //�������ʱ������ݣ���Ϊ��ʼ���ݣ����ж������Сֵ
        gimbal_control.gimbal_cali.max_pitch = gimbal_control.gimbal_arm2_motor.absolute_angle;
        gimbal_control.gimbal_cali.max_pitch_ecd = gimbal_control.gimbal_arm2_motor.gimbal_motor_measure->ecd;
        gimbal_control.gimbal_cali.max_yaw = gimbal_control.gimbal_arm1_motor.absolute_angle;
        gimbal_control.gimbal_cali.max_yaw_ecd = gimbal_control.gimbal_arm1_motor.gimbal_motor_measure->ecd;
        gimbal_control.gimbal_cali.min_pitch = gimbal_control.gimbal_arm2_motor.absolute_angle;
        gimbal_control.gimbal_cali.min_pitch_ecd = gimbal_control.gimbal_arm2_motor.gimbal_motor_measure->ecd;
        gimbal_control.gimbal_cali.min_yaw = gimbal_control.gimbal_arm1_motor.absolute_angle;
        gimbal_control.gimbal_cali.min_yaw_ecd = gimbal_control.gimbal_arm1_motor.gimbal_motor_measure->ecd;
        return 0;
    }
    else if (gimbal_control.gimbal_cali.step == GIMBAL_CALI_END_STEP)
    {
        calc_gimbal_cali(&gimbal_control.gimbal_cali, yaw_offset, pitch_offset, max_yaw, min_yaw, max_pitch, min_pitch);
        gimbal_control.gimbal_arm1_motor.offset_ecd = *yaw_offset;
        gimbal_control.gimbal_arm1_motor.max_relative_angle = *max_yaw;
        gimbal_control.gimbal_arm1_motor.min_relative_angle = *min_yaw;
        gimbal_control.gimbal_arm2_motor.offset_ecd = *pitch_offset;
        gimbal_control.gimbal_arm2_motor.max_relative_angle = *max_pitch;
        gimbal_control.gimbal_arm2_motor.min_relative_angle = *min_pitch;

        return 1;
    }
    else
    {
        return 0;
    }
}

//У׼���㣬������Ƕȣ���̨��ֵ
static void calc_gimbal_cali(const Gimbal_Cali_t *gimbal_cali, uint16_t *yaw_offset, uint16_t *pitch_offset, fp32 *max_yaw, fp32 *min_yaw, fp32 *max_pitch, fp32 *min_pitch)
{
    if (gimbal_cali == NULL || yaw_offset == NULL || pitch_offset == NULL || max_yaw == NULL || min_yaw == NULL || max_pitch == NULL || min_pitch == NULL)
    {
        return;
    }

    int16_t temp_max_ecd = 0, temp_min_ecd = 0, temp_ecd = 0;

#if YAW_TURN
    temp_ecd = gimbal_cali->min_yaw_ecd - gimbal_cali->max_yaw_ecd;

    if (temp_ecd < 0)
    {
        temp_ecd += ecd_range;
    }
    temp_ecd = gimbal_cali->max_yaw_ecd + (temp_ecd / 2);

    ECD_Format(temp_ecd);
    *yaw_offset = temp_ecd;
    *max_yaw = -motor_ecd_to_angle_change(gimbal_cali->max_yaw_ecd, *yaw_offset);
    *min_yaw = -motor_ecd_to_angle_change(gimbal_cali->min_yaw_ecd, *yaw_offset);

#else

    temp_ecd = gimbal_cali->max_yaw_ecd - gimbal_cali->min_yaw_ecd;

    if (temp_ecd < 0)
    {
        temp_ecd += ecd_range;
    }
    temp_ecd = gimbal_cali->max_yaw_ecd - (temp_ecd / 2);

    ECD_Format(temp_ecd);
    *yaw_offset = temp_ecd;
    *max_yaw = motor_ecd_to_angle_change(gimbal_cali->max_yaw_ecd, *yaw_offset);
    *min_yaw = motor_ecd_to_angle_change(gimbal_cali->min_yaw_ecd, *yaw_offset);

#endif

#if PITCH_TURN

    temp_ecd = (int16_t)(gimbal_cali->max_pitch / Motor_Ecd_to_Rad);
    temp_max_ecd = gimbal_cali->max_pitch_ecd + temp_ecd;
    temp_ecd = (int16_t)(gimbal_cali->min_pitch / Motor_Ecd_to_Rad);
    temp_min_ecd = gimbal_cali->min_pitch_ecd + temp_ecd;

    ECD_Format(temp_max_ecd);
    ECD_Format(temp_min_ecd);

    temp_ecd = temp_max_ecd - temp_min_ecd;

    if (temp_ecd > Half_ecd_range)
    {
        temp_ecd -= ecd_range;
    }
    else if (temp_ecd < -Half_ecd_range)
    {
        temp_ecd += ecd_range;
    }

    if (temp_max_ecd > temp_min_ecd)
    {
        temp_min_ecd += ecd_range;
    }

    temp_ecd = temp_max_ecd - temp_ecd / 2;

    ECD_Format(temp_ecd);

    *pitch_offset = temp_ecd;

    *max_pitch = -motor_ecd_to_angle_change(gimbal_cali->max_pitch_ecd, *pitch_offset);
    *min_pitch = -motor_ecd_to_angle_change(gimbal_cali->min_pitch_ecd, *pitch_offset);

#else
    temp_ecd = (int16_t)(gimbal_cali->max_pitch / Motor_Ecd_to_Rad);
    temp_max_ecd = gimbal_cali->max_pitch_ecd - temp_ecd;
    temp_ecd = (int16_t)(gimbal_cali->min_pitch / Motor_Ecd_to_Rad);
    temp_min_ecd = gimbal_cali->min_pitch_ecd - temp_ecd;

    ECD_Format(temp_max_ecd);
    ECD_Format(temp_min_ecd);

    temp_ecd = temp_max_ecd - temp_min_ecd;

    if (temp_ecd > Half_ecd_range)
    {
        temp_ecd -= ecd_range;
    }
    else if (temp_ecd < -Half_ecd_range)
    {
        temp_ecd += ecd_range;
    }

    temp_ecd = temp_max_ecd - temp_ecd / 2;

    ECD_Format(temp_ecd);

    *pitch_offset = temp_ecd;

    *max_pitch = motor_ecd_to_angle_change(gimbal_cali->max_pitch_ecd, *pitch_offset);
    *min_pitch = motor_ecd_to_angle_change(gimbal_cali->min_pitch_ecd, *pitch_offset);
#endif
}

const Gimbal_Motor_t *get_arm1_motor_point(void)
{
    return &gimbal_control.gimbal_arm1_motor;
}

const Gimbal_Motor_t *get_arm2_motor_point(void)
{
    return &gimbal_control.gimbal_arm2_motor;
}

const Gimbal_Motor_t *get_arm3_motor_point(void)
{
    return &gimbal_control.gimbal_arm3_motor;
}

const Gimbal_Motor_t *get_arm4_motor_point(void)
{
    return &gimbal_control.gimbal_arm4_motor;
}

//��ʼ��pid ����ָ��
static void GIMBAL_Init(Gimbal_Control_t *gimbal_init)
{

		static const fp32 Arm_speed_pid[3] = {ARM_SPEED_PID_KP, ARM_SPEED_PID_KI, ARM_SPEED_PID_KD};
	
	
    //�������ָ���ȡ
    gimbal_init->gimbal_arm1_motor.gimbal_motor_measure = get_Arm1_Motor_Measure_Point();
    gimbal_init->gimbal_arm2_motor.gimbal_motor_measure = get_Arm2_Motor_Measure_Point();
    gimbal_init->gimbal_arm3_motor.gimbal_motor_measure = get_Arm3_Motor_Measure_Point();
    gimbal_init->gimbal_arm4_motor.gimbal_motor_measure = get_Arm4_Motor_Measure_Point();
    //����������ָ���ȡ
    gimbal_init->gimbal_INT_angle_point = get_INS_angle_point();
    gimbal_init->gimbal_INT_gyro_point = get_MPU6500_Gyro_Data_Point();
    //ң��������ָ���ȡ
    gimbal_init->gimbal_rc_ctrl = get_remote_control_point();
    //��ʼ�����ģʽ
    gimbal_init->gimbal_arm1_motor.gimbal_motor_mode = gimbal_init->gimbal_arm1_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    gimbal_init->gimbal_arm2_motor.gimbal_motor_mode = gimbal_init->gimbal_arm2_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;
		gimbal_init->gimbal_arm3_motor.gimbal_motor_mode = gimbal_init->gimbal_arm3_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    gimbal_init->gimbal_arm4_motor.gimbal_motor_mode = gimbal_init->gimbal_arm4_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    
		//��ʼ��ARM1���pid
//    GIMBAL_PID_Init(&gimbal_init->gimbal_arm1_motor.gimbal_motor_relative_angle_pid, ARM_ENCODE_RELATIVE_PID_MAX_OUT, ARM_ENCODE_RELATIVE_PID_MAX_IOUT, ARM_ENCODE_RELATIVE_PID_KP, ARM_ENCODE_RELATIVE_PID_KI, ARM_ENCODE_RELATIVE_PID_KD);
    PID_Init(&gimbal_init->gimbal_arm1_motor.gimbal_motor_rpm_pid, PID_POSITION, Arm_speed_pid, ARM_SPEED_PID_MAX_OUT, ARM_SPEED_PID_MAX_IOUT);
    //��ʼ��ARM2���pid
//    GIMBAL_PID_Init(&gimbal_init->gimbal_arm2_motor.gimbal_motor_relative_angle_pid, ARM_ENCODE_RELATIVE_PID_MAX_OUT, ARM_ENCODE_RELATIVE_PID_MAX_IOUT, ARM_ENCODE_RELATIVE_PID_KP, ARM_ENCODE_RELATIVE_PID_KI, ARM_ENCODE_RELATIVE_PID_KD);
		PID_Init(&gimbal_init->gimbal_arm2_motor.gimbal_motor_rpm_pid, PID_POSITION, Arm_speed_pid, ARM_SPEED_PID_MAX_OUT, ARM_SPEED_PID_MAX_IOUT);
    //��ʼ��ARM3���pid
//    GIMBAL_PID_Init(&gimbal_init->gimbal_arm3_motor.gimbal_motor_relative_angle_pid, ARM_ENCODE_RELATIVE_PID_MAX_OUT, ARM_ENCODE_RELATIVE_PID_MAX_IOUT, ARM_ENCODE_RELATIVE_PID_KP, ARM_ENCODE_RELATIVE_PID_KI, ARM_ENCODE_RELATIVE_PID_KD);
		PID_Init(&gimbal_init->gimbal_arm3_motor.gimbal_motor_rpm_pid, PID_POSITION, Arm_speed_pid, ARM_SPEED_PID_MAX_OUT, ARM_SPEED_PID_MAX_IOUT);
    //��ʼ��ARM4���pid
//    GIMBAL_PID_Init(&gimbal_init->gimbal_arm2_motor.gimbal_motor_relative_angle_pid, ARM_ENCODE_RELATIVE_PID_MAX_OUT, ARM_ENCODE_RELATIVE_PID_MAX_IOUT, ARM_ENCODE_RELATIVE_PID_KP, ARM_ENCODE_RELATIVE_PID_KI, ARM_ENCODE_RELATIVE_PID_KD);
		PID_Init(&gimbal_init->gimbal_arm4_motor.gimbal_motor_rpm_pid, PID_POSITION, Arm_speed_pid, ARM_SPEED_PID_MAX_OUT, ARM_SPEED_PID_MAX_IOUT);

    //�������PID
    gimbal_total_pid_clear(gimbal_init);

    gimbal_init->gimbal_arm1_motor.relative_angle_set = gimbal_init->gimbal_arm1_motor.relative_angle;
    gimbal_init->gimbal_arm2_motor.relative_angle_set = gimbal_init->gimbal_arm2_motor.relative_angle;
    gimbal_init->gimbal_arm3_motor.relative_angle_set = gimbal_init->gimbal_arm3_motor.relative_angle;
    gimbal_init->gimbal_arm4_motor.relative_angle_set = gimbal_init->gimbal_arm4_motor.relative_angle;


}



#if GIMBAL_TEST_MODE

//jscope�۲�����
int32_t arm1_ins_int_1000, arm2_ins_int_1000, arm1_ins_raw_int_1000;
int32_t arm1_ins_set_1000, arm2_ins_set_1000;
int32_t arm2_relative_set_1000, arm2_relative_angle_1000;
int32_t arm1_relative_set_1000, arm1_relative_angle_1000;
int32_t arm1_speed_int_1000, arm2_speed_int_1000;
int32_t arm1_speed_set_int_1000, arm2_speed_set_int_1000;


static void J_scope_gimbal_test(void)
{
    arm1_ins_int_1000 = (int32_t)(gimbal_control.gimbal_arm1_motor.absolute_angle * 1000);
    arm1_ins_set_1000 = (int32_t)(gimbal_control.gimbal_arm1_motor.absolute_angle_set * 1000);
    arm1_speed_int_1000 = (int32_t)(gimbal_control.gimbal_arm1_motor.motor_gyro * 1000);
    arm1_speed_set_int_1000 = (int32_t)(gimbal_control.gimbal_arm1_motor.motor_gyro_set * 1000);
		arm1_relative_angle_1000 = (int32_t)(gimbal_control.gimbal_arm1_motor.relative_angle * 1000);
    arm1_relative_set_1000 = (int32_t)(gimbal_control.gimbal_arm1_motor.relative_angle_set * 1000);

    arm2_ins_int_1000 = (int32_t)(gimbal_control.gimbal_arm2_motor.absolute_angle * 1000);
    arm2_ins_set_1000 = (int32_t)(gimbal_control.gimbal_arm2_motor.absolute_angle_set * 1000);
    arm2_speed_int_1000 = (int32_t)(gimbal_control.gimbal_arm2_motor.motor_rpm * 1000);
    arm2_speed_set_int_1000 = (int32_t)(gimbal_control.gimbal_arm2_motor.motor_rpm_set * 1000);
    arm2_relative_angle_1000 = (int32_t)(gimbal_control.gimbal_arm2_motor.relative_angle * -1000);
    arm2_relative_set_1000 = (int32_t)(gimbal_control.gimbal_arm2_motor.relative_angle_set * 1000);
	
}

#endif



static void GIMBAL_Set_Mode(Gimbal_Control_t *gimbal_set_mode)
{
    if (gimbal_set_mode == NULL)
    {
        return;
    }
    gimbal_behaviour_mode_set(gimbal_set_mode);
}

static void GIMBAL_Feedback_Update(Gimbal_Control_t *gimbal_feedback_update)
{
    if (gimbal_feedback_update == NULL)
    {
        return;
    }
    //��̨���ݸ���
		gimbal_feedback_update->gimbal_arm1_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_arm1_motor.gimbal_motor_measure->ecd,
                                                                                        gimbal_feedback_update->gimbal_arm1_motor.offset_ecd);
		gimbal_feedback_update->gimbal_arm1_motor.motor_rpm=0.25f*0.000415809748903494517209f*gimbal_feedback_update->gimbal_arm1_motor.gimbal_motor_measure->speed_rpm;//ARM1���ת�٣�3508ȫ����ͬ
		
		gimbal_feedback_update->gimbal_arm2_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_arm2_motor.gimbal_motor_measure->ecd,
                                                                                          gimbal_feedback_update->gimbal_arm2_motor.offset_ecd);
		gimbal_feedback_update->gimbal_arm2_motor.motor_rpm=0.25f*0.000415809748903494517209f*gimbal_feedback_update->gimbal_arm2_motor.gimbal_motor_measure->speed_rpm;//ARM2���ת��

		gimbal_feedback_update->gimbal_arm3_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_arm3_motor.gimbal_motor_measure->ecd,
                                                                                        gimbal_feedback_update->gimbal_arm3_motor.offset_ecd);
		gimbal_feedback_update->gimbal_arm3_motor.motor_rpm=0.25f*0.000415809748903494517209f*gimbal_feedback_update->gimbal_arm3_motor.gimbal_motor_measure->speed_rpm;//ARM1���ת�٣�3508ȫ����ͬ

		gimbal_feedback_update->gimbal_arm4_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_arm4_motor.gimbal_motor_measure->ecd,
                                                                                        gimbal_feedback_update->gimbal_arm4_motor.offset_ecd);
		gimbal_feedback_update->gimbal_arm4_motor.motor_rpm=0.25f*0.000415809748903494517209f*gimbal_feedback_update->gimbal_arm4_motor.gimbal_motor_measure->speed_rpm;//ARM1���ת�٣�3508ȫ����ͬ
}
//������ԽǶ�
static fp32 motor_ecd_to_angle_change(uint16_t ecd, uint16_t offset_ecd)
{
    int32_t relative_ecd = ecd - offset_ecd;
    if (relative_ecd > Half_ecd_range)
    {
        relative_ecd -= ecd_range;
    }
    else if (relative_ecd < -Half_ecd_range)
    {
        relative_ecd += ecd_range;
    }

    return relative_ecd * Motor_Ecd_to_Rad;
}

//��̨״̬�л����棬����״̬�л�����

static void GIMBAL_Mode_Change_Control_Transit(Gimbal_Control_t *gimbal_mode_change)
{
    if (gimbal_mode_change == NULL)
    {
        return;
    }
    //��е��1���״̬���л���������
    if (gimbal_mode_change->gimbal_arm1_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && gimbal_mode_change->gimbal_arm1_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_arm1_motor.raw_cmd_current = gimbal_mode_change->gimbal_arm1_motor.current_set = gimbal_mode_change->gimbal_arm1_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_arm1_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCODE && gimbal_mode_change->gimbal_arm1_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCODE)
    {
        gimbal_mode_change->gimbal_arm1_motor.relative_angle_set = gimbal_mode_change->gimbal_arm1_motor.relative_angle;
		}
    gimbal_mode_change->gimbal_arm1_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_arm1_motor.gimbal_motor_mode;

		
    //��е��2���״̬���л���������
    if (gimbal_mode_change->gimbal_arm2_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && gimbal_mode_change->gimbal_arm2_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_arm2_motor.raw_cmd_current = gimbal_mode_change->gimbal_arm2_motor.current_set = gimbal_mode_change->gimbal_arm2_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_arm2_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCODE && gimbal_mode_change->gimbal_arm2_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCODE)
    {
        gimbal_mode_change->gimbal_arm2_motor.relative_angle_set = gimbal_mode_change->gimbal_arm2_motor.relative_angle;
    }
    gimbal_mode_change->gimbal_arm2_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_arm2_motor.gimbal_motor_mode;
		
		 //��е��3���״̬���л���������
    if (gimbal_mode_change->gimbal_arm3_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && gimbal_mode_change->gimbal_arm3_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_arm3_motor.raw_cmd_current = gimbal_mode_change->gimbal_arm3_motor.current_set = gimbal_mode_change->gimbal_arm3_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_arm3_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCODE && gimbal_mode_change->gimbal_arm3_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCODE)
    {
        gimbal_mode_change->gimbal_arm3_motor.relative_angle_set = gimbal_mode_change->gimbal_arm3_motor.relative_angle;
    }
    gimbal_mode_change->gimbal_arm3_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_arm3_motor.gimbal_motor_mode;
		
		//��е��44���״̬���л���������
    if (gimbal_mode_change->gimbal_arm4_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && gimbal_mode_change->gimbal_arm4_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_arm4_motor.raw_cmd_current = gimbal_mode_change->gimbal_arm4_motor.current_set = gimbal_mode_change->gimbal_arm4_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_arm4_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCODE && gimbal_mode_change->gimbal_arm4_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCODE)
    {
        gimbal_mode_change->gimbal_arm4_motor.relative_angle_set = gimbal_mode_change->gimbal_arm4_motor.relative_angle;
    }
    gimbal_mode_change->gimbal_arm4_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_arm4_motor.gimbal_motor_mode;
}


//��̨����״̬ʹ�ò�ͬ����pid
static void GIMBAL_Control_loop(Gimbal_Control_t *gimbal_control_loop)
{
    if (gimbal_control_loop == NULL)
    {
        return;
    }
    //ARM1��ͬģʽ���ڲ�ͬ�Ŀ��ƺ���
    if (gimbal_control_loop->gimbal_arm1_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw����
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_arm1_motor);
    }
    else if (gimbal_control_loop->gimbal_arm1_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCODE)
    {
        //enconde�Ƕȿ���
        gimbal_motor_relative_angle_control_arm1(&gimbal_control_loop->gimbal_arm1_motor);//��е��1
			
			//�����λ����
			if (switch_is_down(gimbal_control_loop->gimbal_rc_ctrl->rc.s[1]))
			{
        SoftReset();
			}
			//�����λ����
		
    }
		
		
    //ARM2��ͬģʽ���ڲ�ͬ�Ŀ��ƺ���
    if (gimbal_control_loop->gimbal_arm2_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw����
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_arm2_motor);
    }
    else if (gimbal_control_loop->gimbal_arm2_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCODE)
    {
        //enconde�Ƕȿ���
        gimbal_motor_relative_angle_control_arm2(&gimbal_control_loop->gimbal_arm2_motor);
    }
		
		//ARM3��ͬģʽ���ڲ�ͬ�Ŀ��ƺ���
    if (gimbal_control_loop->gimbal_arm3_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw����
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_arm3_motor);
    }
    else if (gimbal_control_loop->gimbal_arm3_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCODE)
    {
        //enconde�Ƕȿ���
        gimbal_motor_relative_angle_control_arm3(&gimbal_control_loop->gimbal_arm3_motor);
    }
		
		//ARM4��ͬģʽ���ڲ�ͬ�Ŀ��ƺ���
    if (gimbal_control_loop->gimbal_arm4_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw����
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_arm4_motor);
    }
    else if (gimbal_control_loop->gimbal_arm4_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCODE)
    {
        //enconde�Ƕȿ���
        gimbal_motor_relative_angle_control_arm4(&gimbal_control_loop->gimbal_arm4_motor);
    }
}



static void gimbal_motor_relative_angle_control_arm1(Gimbal_Motor_t *gimbal_motor)//��е��1���
{
    if (gimbal_motor == NULL)
    {
        return;
    }
				
		static fp32 delta_arm1;//ARM1����仯��
//		static fp32 const data_to_deg_ratio=0.001745329252f;//2*pi����/360�Ƕ�/10����

		//����
		gimbal_control.gimbal_arm1_motor.motor_rpm=fp32_constrain(gimbal_control.gimbal_arm1_motor.motor_rpm, -ARM_MAX_SPEED, ARM_MAX_SPEED);
		
		delta_arm1=(fp32)(gimbal_control.gimbal_rc_ctrl->rc.ch[Arm12Channel])*ARM_RC_SEN
						 +(fp32)(gimbal_control.gimbal_rc_ctrl->mouse.y)*ARM_MOUSE_SEN;
		

		//����motor_rpm_set��ֵ���ﵽ�����ٶȻ�
		gimbal_motor->motor_rpm_set=delta_arm1;
		//�ٶȻ�PID
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_rpm_pid, gimbal_motor->motor_rpm, gimbal_motor->motor_rpm_set);
		//����ֵ��ֵ
		gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
		
}

static void gimbal_motor_relative_angle_control_arm2(Gimbal_Motor_t *gimbal_motor)//��е��2���
{
    if (gimbal_motor == NULL)
    {
        return;
    }

		static fp32 delta_arm2;//ARM2����仯��
//		static fp32 const data_to_deg_ratio=0.001745329252f;//2*pi����/360�Ƕ�/10����
		
		
		//����
		gimbal_control.gimbal_arm2_motor.motor_rpm=fp32_constrain(gimbal_control.gimbal_arm2_motor.motor_rpm, -ARM_MAX_SPEED, ARM_MAX_SPEED);

		delta_arm2=(fp32)(gimbal_control.gimbal_rc_ctrl->rc.ch[Arm12Channel])*ARM_RC_SEN
							 +(fp32)(gimbal_control.gimbal_rc_ctrl->mouse.y)*ARM_MOUSE_SEN;
		
		//����motor_rpm_set��ֵ���ﵽ�����ٶȻ�
		gimbal_motor->motor_rpm_set=delta_arm2;
		//�ٶȻ�PID
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_rpm_pid, gimbal_motor->motor_rpm, gimbal_motor->motor_rpm_set);
    //����ֵ��ֵ
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}

static void gimbal_motor_relative_angle_control_arm3(Gimbal_Motor_t *gimbal_motor)//��е��3���
{
    if (gimbal_motor == NULL)
    {
        return;
    }

		static fp32 delta_arm3;//ARM2����仯��
//		static fp32 const data_to_deg_ratio=0.001745329252f;//2*pi����/360�Ƕ�/10����
		
		
		//����
		gimbal_control.gimbal_arm3_motor.motor_rpm=fp32_constrain(gimbal_control.gimbal_arm3_motor.motor_rpm, -ARM_MAX_SPEED, ARM_MAX_SPEED);

		delta_arm3=(fp32)(gimbal_control.gimbal_rc_ctrl->rc.ch[Arm34Channel])*ARM_RC_SEN
							 +(fp32)(gimbal_control.gimbal_rc_ctrl->mouse.x)*ARM_MOUSE_SEN;
		
		//����motor_rpm_set��ֵ���ﵽ�����ٶȻ�
		gimbal_motor->motor_rpm_set=delta_arm3;
		//�ٶȻ�PID
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_rpm_pid, gimbal_motor->motor_rpm, gimbal_motor->motor_rpm_set);
    //����ֵ��ֵ
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}

static void gimbal_motor_relative_angle_control_arm4(Gimbal_Motor_t *gimbal_motor)//��е��4���
{
    if (gimbal_motor == NULL)
    {
        return;
    }

		static fp32 delta_arm4;//ARM2����仯��
//		static fp32 const data_to_deg_ratio=0.001745329252f;//2*pi����/360�Ƕ�/10����
		
		
		//����
		gimbal_control.gimbal_arm4_motor.motor_rpm=fp32_constrain(gimbal_control.gimbal_arm4_motor.motor_rpm, -ARM_MAX_SPEED, ARM_MAX_SPEED);

		delta_arm4=(fp32)(gimbal_control.gimbal_rc_ctrl->rc.ch[Arm34Channel])*ARM_RC_SEN
							 +(fp32)(gimbal_control.gimbal_rc_ctrl->mouse.x)*ARM_MOUSE_SEN;
		
		//����motor_rpm_set��ֵ���ﵽ�����ٶȻ�
		gimbal_motor->motor_rpm_set=delta_arm4;
		//�ٶȻ�PID
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_rpm_pid, gimbal_motor->motor_rpm, gimbal_motor->motor_rpm_set);
    //����ֵ��ֵ
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}
static void gimbal_motor_raw_angle_control(Gimbal_Motor_t *gimbal_motor)
{
    if (gimbal_motor == NULL)
    {
        return;
    }
    gimbal_motor->current_set = gimbal_motor->raw_cmd_current;
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}

static void GIMBAL_PID_Init(Gimbal_PID_t *pid, fp32 maxout, fp32 max_iout, fp32 kp, fp32 ki, fp32 kd)
{
    if (pid == NULL)
    {
        return;
    }
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->err = 0.0f;
    pid->get = 0.0f;

    pid->max_iout = max_iout;
    pid->max_out = maxout;
}
static fp32 GIMBAL_PID_Calc(Gimbal_PID_t *pid, fp32 get, fp32 set, fp32 error_delta)
{
    fp32 err;
    if (pid == NULL)
    {
        return 0.0f;
    }
    pid->get = get;
    pid->set = set;

    err = set - get;
    pid->err = rad_format(err);
    pid->Pout = pid->kp * pid->err;
    pid->Iout += pid->ki * pid->err;
    pid->Dout = pid->kd * error_delta;
    abs_limit(&pid->Iout, pid->max_iout);
    pid->out = pid->Pout + pid->Iout + pid->Dout;
    abs_limit(&pid->out, pid->max_out);
    return pid->out;
}

//pid��������
static void Gimbal_PID_clear(Gimbal_PID_t *gimbal_pid_clear)
{
    if (gimbal_pid_clear == NULL)
    {
        return;
    }
    gimbal_pid_clear->err = gimbal_pid_clear->set = gimbal_pid_clear->get = 0.0f;
    gimbal_pid_clear->out = gimbal_pid_clear->Pout = gimbal_pid_clear->Iout = gimbal_pid_clear->Dout = 0.0f;
}
