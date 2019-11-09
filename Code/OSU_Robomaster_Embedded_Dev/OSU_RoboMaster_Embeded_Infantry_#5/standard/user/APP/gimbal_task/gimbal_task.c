/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       gimbal_task.c/h
  * @brief      �����̨��������������̨ʹ�������ǽ�����ĽǶȣ��䷶Χ�ڣ�-pi,pi��
  *             �ʶ�����Ŀ��ǶȾ�Ϊ��Χ���������ԽǶȼ���ĺ�������̨��Ҫ��Ϊ2��
  *             ״̬�������ǿ���״̬�����ð��������ǽ������̬�ǽ��п��ƣ�����������
  *             ״̬��ͨ����������ı���ֵ���Ƶ�У׼�����⻹��У׼״̬��ֹͣ״̬�ȡ�
  * @note
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. ���
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2016 DJI****************************
  */

#include "Gimbal_Task.h"
#include "stdlib.h"
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
#include "laser.h"
#include "filter.h"
#include "User_Task.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"


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
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_yaw_motor.gimbal_motor_absolute_angle_pid);   \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_yaw_motor.gimbal_motor_relative_angle_pid);   \
        PID_clear(&(gimbal_clear)->gimbal_yaw_motor.gimbal_motor_gyro_pid);                    \
                                                                                               \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_pitch_motor.gimbal_motor_absolute_angle_pid); \
        Gimbal_PID_clear(&(gimbal_clear)->gimbal_pitch_motor.gimbal_motor_relative_angle_pid); \
        PID_clear(&(gimbal_clear)->gimbal_pitch_motor.gimbal_motor_gyro_pid);                  \
    }

#if INCLUDE_uxTaskGetStackHighWaterMark
uint32_t gimbal_high_water;
#endif

//��̨���������������
extern Gimbal_Control_t gimbal_control;


//���͵�can ָ��
static int16_t Yaw_Can_Set_Current = 0, Pitch_Can_Set_Current = 0, Shoot_Can_Set_Current = 0;

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
//������̨������
static void GIMBAL_Set_Contorl(Gimbal_Control_t *gimbal_set_control);
//��̨����pid����
static void GIMBAL_Control_loop(Gimbal_Control_t *gimbal_control_loop);


static void gimbal_motor_absolute_angle_control(Gimbal_Motor_t *gimbal_motor);
//static void gimbal_motor_absolute_angle_control_yaw(Gimbal_Motor_t *gimbal_motor);
static void gimbal_motor_drive_mode_control_yaw(Gimbal_Motor_t *gimbal_motor);//��ʻģʽ��YAW���
static void gimbal_motor_drive_mode_control_pitch(Gimbal_Motor_t *gimbal_motor);//��ʻģʽ��PITCH���
static void gimbal_motor_relative_angle_control_yaw(Gimbal_Motor_t *gimbal_motor);//��ͨģʽ��YAW���
static void gimbal_motor_relative_angle_control_pitch(Gimbal_Motor_t *gimbal_motor);//��ͨģʽ��PITCH���
static void gimbal_motor_aim_control_gyro_yaw(Gimbal_Motor_t *gimbal_motor);//����ģʽ-�����Ǿ���λ��-YAW���
static void gimbal_motor_aim_control_pitch(Gimbal_Motor_t *gimbal_motor);//����ģʽ��PITCH���

static void gimbal_motor_raw_angle_control(Gimbal_Motor_t *gimbal_motor);

//�������ǽǶȿ����£��Կ��Ƶ�Ŀ��ֵ�������Է��������ԽǶ�
static void GIMBAL_relative_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add);//��У׼��
static void GIMBAL_absolute_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add);//��У׼��
//static void GIMBAL_relative_angle_limit_yaw(Gimbal_Motor_t *gimbal_motor, fp32 add);
//static void GIMBAL_relative_angle_limit_pitch(Gimbal_Motor_t *gimbal_motor, fp32 add);
static void GIMBAL_PID_Init(Gimbal_PID_t *pid, fp32 maxout, fp32 intergral_limit, fp32 kp, fp32 ki, fp32 kd);
static fp32 GIMBAL_PID_Calc(Gimbal_PID_t *pid, fp32 get, fp32 set, fp32 error_delta);

//�����Ǿ��ԽǶ���ʱ����ֵ
fp32 temp_absolute_angle_reference;



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
    //�����ʼ��
    shoot_init();
    //�жϵ���Ƿ�����
    while (toe_is_error(YawGimbalMotorTOE) || toe_is_error(PitchGimbalMotorTOE) || toe_is_error(TriggerMotorTOE))
    {
        vTaskDelay(10*GIMBAL_CONTROL_TIME);
        GIMBAL_Feedback_Update(&gimbal_control);             //��̨���ݷ���
    }


    while (1)
    {

        GIMBAL_Set_Mode(&gimbal_control);                    //������̨����ģʽ

        GIMBAL_Mode_Change_Control_Transit(&gimbal_control); //����ģʽ�л� �������ݹ���

        GIMBAL_Feedback_Update(&gimbal_control);             //��̨���ݷ���

        GIMBAL_Set_Contorl(&gimbal_control);                 //������̨������

        GIMBAL_Control_loop(&gimbal_control);                //��̨����PID����

        Shoot_Can_Set_Current = shoot_control_loop();        //����������ѭ��
				laser_on();



#if YAW_TURN
        Yaw_Can_Set_Current = -gimbal_control.gimbal_yaw_motor.given_current;
#else
        Yaw_Can_Set_Current = gimbal_control.gimbal_yaw_motor.given_current;
#endif

#if PITCH_TURN
        Pitch_Can_Set_Current = -gimbal_control.gimbal_pitch_motor.given_current;
#else
        Pitch_Can_Set_Current = gimbal_control.gimbal_pitch_motor.given_current;
#endif





        //��̨��ң��������״̬��relax ״̬��canָ��Ϊ0����ʹ��current����Ϊ��ķ������Ǳ�֤ң��������һ��ʹ����ֹ̨ͣ
        if (!(toe_is_error(YawGimbalMotorTOE) && toe_is_error(PitchGimbalMotorTOE)&& toe_is_error(TriggerMotorTOE)))//
        {
            if (toe_is_error(DBUSTOE))
            {
                CAN_CMD_GIMBAL(0, 0, 0, 0);
            }
            else
            {
                CAN_CMD_GIMBAL(Yaw_Can_Set_Current, Pitch_Can_Set_Current, Shoot_Can_Set_Current, 0);
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
    gimbal_control.gimbal_yaw_motor.offset_ecd = yaw_offset;
    gimbal_control.gimbal_yaw_motor.max_relative_angle = max_yaw;
    gimbal_control.gimbal_yaw_motor.min_relative_angle = min_yaw;

    gimbal_control.gimbal_pitch_motor.offset_ecd = pitch_offset;
    gimbal_control.gimbal_pitch_motor.max_relative_angle = max_pitch;
    gimbal_control.gimbal_pitch_motor.min_relative_angle = min_pitch;
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
        gimbal_control.gimbal_cali.max_pitch = gimbal_control.gimbal_pitch_motor.absolute_angle;
        gimbal_control.gimbal_cali.max_pitch_ecd = gimbal_control.gimbal_pitch_motor.gimbal_motor_measure->ecd;
        gimbal_control.gimbal_cali.max_yaw = gimbal_control.gimbal_yaw_motor.absolute_angle;
        gimbal_control.gimbal_cali.max_yaw_ecd = gimbal_control.gimbal_yaw_motor.gimbal_motor_measure->ecd;
        gimbal_control.gimbal_cali.min_pitch = gimbal_control.gimbal_pitch_motor.absolute_angle;
        gimbal_control.gimbal_cali.min_pitch_ecd = gimbal_control.gimbal_pitch_motor.gimbal_motor_measure->ecd;
        gimbal_control.gimbal_cali.min_yaw = gimbal_control.gimbal_yaw_motor.absolute_angle;
        gimbal_control.gimbal_cali.min_yaw_ecd = gimbal_control.gimbal_yaw_motor.gimbal_motor_measure->ecd;
        return 0;
    }
    else if (gimbal_control.gimbal_cali.step == GIMBAL_CALI_END_STEP)
    {
        calc_gimbal_cali(&gimbal_control.gimbal_cali, yaw_offset, pitch_offset, max_yaw, min_yaw, max_pitch, min_pitch);
        gimbal_control.gimbal_yaw_motor.offset_ecd = *yaw_offset;
        gimbal_control.gimbal_yaw_motor.max_relative_angle = *max_yaw;
        gimbal_control.gimbal_yaw_motor.min_relative_angle = *min_yaw;
        gimbal_control.gimbal_pitch_motor.offset_ecd = *pitch_offset;
        gimbal_control.gimbal_pitch_motor.max_relative_angle = *max_pitch;
        gimbal_control.gimbal_pitch_motor.min_relative_angle = *min_pitch;

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

const Gimbal_Motor_t *get_yaw_motor_point(void)
{
    return &gimbal_control.gimbal_yaw_motor;
}

const Gimbal_Motor_t *get_pitch_motor_point(void)
{
    return &gimbal_control.gimbal_pitch_motor;
}

//��ʼ��pid ����ָ��
static void GIMBAL_Init(Gimbal_Control_t *gimbal_init)
{

    static const fp32 Pitch_speed_pid[3] = {PITCH_SPEED_PID_KP, PITCH_SPEED_PID_KI, PITCH_SPEED_PID_KD};
    static const fp32 Yaw_speed_pid[3] = {YAW_SPEED_PID_KP, YAW_SPEED_PID_KI, YAW_SPEED_PID_KD};
    //�������ָ���ȡ
    gimbal_init->gimbal_yaw_motor.gimbal_motor_measure = get_Yaw_Gimbal_Motor_Measure_Point();
    gimbal_init->gimbal_pitch_motor.gimbal_motor_measure = get_Pitch_Gimbal_Motor_Measure_Point();
    //����������ָ���ȡ
    gimbal_init->gimbal_INT_angle_point = get_INS_angle_point();
    gimbal_init->gimbal_INT_gyro_point = get_MPU6500_Gyro_Data_Point();
    //ң��������ָ���ȡ
    gimbal_init->gimbal_rc_ctrl = get_remote_control_point();
    //��ʼ�����ģʽ
    gimbal_init->gimbal_yaw_motor.gimbal_motor_mode = gimbal_init->gimbal_yaw_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    gimbal_init->gimbal_pitch_motor.gimbal_motor_mode = gimbal_init->gimbal_pitch_motor.last_gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    //��ʼ��yaw���pid
    GIMBAL_PID_Init(&gimbal_init->gimbal_yaw_motor.gimbal_motor_absolute_angle_pid, YAW_GYRO_ABSOLUTE_PID_MAX_OUT, YAW_GYRO_ABSOLUTE_PID_MAX_IOUT, YAW_GYRO_ABSOLUTE_PID_KP, YAW_GYRO_ABSOLUTE_PID_KI, YAW_GYRO_ABSOLUTE_PID_KD);
    GIMBAL_PID_Init(&gimbal_init->gimbal_yaw_motor.gimbal_motor_relative_angle_pid, YAW_ENCODE_RELATIVE_PID_MAX_OUT, YAW_ENCODE_RELATIVE_PID_MAX_IOUT, YAW_ENCODE_RELATIVE_PID_KP, YAW_ENCODE_RELATIVE_PID_KI, YAW_ENCODE_RELATIVE_PID_KD);
    PID_Init(&gimbal_init->gimbal_yaw_motor.gimbal_motor_gyro_pid, PID_POSITION, Yaw_speed_pid, YAW_SPEED_PID_MAX_OUT, YAW_SPEED_PID_MAX_IOUT);
    //��ʼ��pitch���pid
    GIMBAL_PID_Init(&gimbal_init->gimbal_pitch_motor.gimbal_motor_absolute_angle_pid, PITCH_GYRO_ABSOLUTE_PID_MAX_OUT, PITCH_GYRO_ABSOLUTE_PID_MAX_IOUT, PITCH_GYRO_ABSOLUTE_PID_KP, PITCH_GYRO_ABSOLUTE_PID_KI, PITCH_GYRO_ABSOLUTE_PID_KD);
    GIMBAL_PID_Init(&gimbal_init->gimbal_pitch_motor.gimbal_motor_relative_angle_pid, PITCH_ENCODE_RELATIVE_PID_MAX_OUT, PITCH_ENCODE_RELATIVE_PID_MAX_IOUT, PITCH_ENCODE_RELATIVE_PID_KP, PITCH_ENCODE_RELATIVE_PID_KI, PITCH_ENCODE_RELATIVE_PID_KD);
    PID_Init(&gimbal_init->gimbal_pitch_motor.gimbal_motor_gyro_pid, PID_POSITION, Pitch_speed_pid, PITCH_SPEED_PID_MAX_OUT, PITCH_SPEED_PID_MAX_IOUT);

    //�������PID
    gimbal_total_pid_clear(gimbal_init);

    GIMBAL_Feedback_Update(gimbal_init);

    gimbal_init->gimbal_yaw_motor.absolute_angle_set = gimbal_init->gimbal_yaw_motor.absolute_angle;
    gimbal_init->gimbal_yaw_motor.relative_angle_set = gimbal_init->gimbal_yaw_motor.relative_angle;
    gimbal_init->gimbal_yaw_motor.motor_gyro_set = gimbal_init->gimbal_yaw_motor.motor_gyro;


    gimbal_init->gimbal_pitch_motor.absolute_angle_set = gimbal_init->gimbal_pitch_motor.absolute_angle;
    gimbal_init->gimbal_pitch_motor.relative_angle_set = gimbal_init->gimbal_pitch_motor.relative_angle;
    gimbal_init->gimbal_pitch_motor.motor_gyro_set = gimbal_init->gimbal_pitch_motor.motor_gyro;


}



#if GIMBAL_TEST_MODE

//jscope�۲�����
int32_t yaw_ins_int_1000, pitch_ins_int_1000, yaw_ins_raw_int_1000;
int32_t yaw_ins_set_1000, pitch_ins_set_1000;
int32_t pitch_relative_set_1000, pitch_relative_angle_1000;
int32_t yaw_relative_set_1000, yaw_relative_angle_1000;
int32_t yaw_speed_int_1000, pitch_speed_int_1000;
int32_t yaw_speed_set_int_1000, pitch_speed_set_int_1000;


//jscope�Զ���۲�����
int32_t filtered_final_yaw_angle_set_jscope;
int32_t filtered_final_pitch_angle_set_jscope;
int32_t delayed_yaw_absolute_angle_jscope;
int32_t delayed_pitch_relative_angle_jscope;
int32_t filtered_horizontal_pixel_jscope;
int32_t filtered_yaw_motor_speed_jscope;
int32_t filtered_vertical_pixel_jscope;
int32_t filtered_pitch_motor_speed_jscope;

int32_t temp_absolute_angle_reference_jscope;

int32_t prediciton_filtered_final_yaw_angle_set_jscope;

//��������
extern int32_t final_yaw_angle_set;//���ոı��YAW��Ƕ�ֵ
extern int32_t final_pitch_angle_set;//���ոı��PITCH��Ƕ�ֵ

/////////////////////////////////////////////////
//�ⲿ�ļ���������
fp32 *filtered_final_angle_set;
fp32 *filtered_aim_data;
fp32 delayed_yaw_absolute_angle;
fp32 delayed_pitch_relative_angle;//�����user_task.c extern��group delay���relative angle��ֵ
int32_t yaw_mid_offset;
int32_t pitch_mid_offset;
fp32 prediction_time_offset;
/////////////////////////////////////////////////

static void J_scope_gimbal_test(void)
{
    yaw_ins_int_1000 = (int32_t)(gimbal_control.gimbal_yaw_motor.absolute_angle * 1000);
    yaw_ins_set_1000 = (int32_t)(gimbal_control.gimbal_yaw_motor.absolute_angle_set * 1000);
    yaw_speed_int_1000 = (int32_t)(gimbal_control.gimbal_yaw_motor.motor_gyro * 1000);
    yaw_speed_set_int_1000 = (int32_t)(gimbal_control.gimbal_yaw_motor.motor_gyro_set * 1000);
    yaw_relative_angle_1000 = (int32_t)(gimbal_control.gimbal_yaw_motor.relative_angle * 1000);
    yaw_relative_set_1000 = (int32_t)(gimbal_control.gimbal_yaw_motor.relative_angle_set * 1000);

    pitch_ins_int_1000 = (int32_t)(gimbal_control.gimbal_pitch_motor.absolute_angle * 1000);
    pitch_ins_set_1000 = (int32_t)(gimbal_control.gimbal_pitch_motor.absolute_angle_set * 1000);
    pitch_speed_int_1000 = (int32_t)(gimbal_control.gimbal_pitch_motor.motor_gyro * 1000);
    pitch_speed_set_int_1000 = (int32_t)(gimbal_control.gimbal_pitch_motor.motor_gyro_set * 1000);
    pitch_relative_angle_1000 = (int32_t)(gimbal_control.gimbal_pitch_motor.relative_angle * -1000);
    pitch_relative_set_1000 = (int32_t)(gimbal_control.gimbal_pitch_motor.relative_angle_set * 1000);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    filtered_horizontal_pixel_jscope=(int32_t)(filtered_aim_data[0]-(YAW_MID+yaw_mid_offset));//��� �������ϵx�� �е�ĽǶ� = �˲����x���������� - x�����ĵ�
    filtered_vertical_pixel_jscope=(int32_t)(filtered_aim_data[1]-(PITCH_MID+pitch_mid_offset));//��� �������ϵy�� �е�ĽǶ� = �˲����y���������� - y�����ĵ�
    filtered_yaw_motor_speed_jscope=(int32_t)(filtered_aim_data[2]*1000);//�˲����YAW�����ٶ�
    filtered_pitch_motor_speed_jscope=(int32_t)(filtered_aim_data[3]*1000);//�˲����PITCH�����ٶ�

    delayed_yaw_absolute_angle_jscope=(int32_t)(delayed_yaw_absolute_angle*1000);
    delayed_pitch_relative_angle_jscope=(int32_t)(delayed_pitch_relative_angle*-1000);

    //YAW���������սǶ� �Ǹ���ֵ
    final_yaw_angle_set=(int32_t)(filtered_horizontal_pixel_jscope-(delayed_yaw_absolute_angle*RAD_TO_DEGREE));
    //PITCH���������սǶ� �Ǹ���ֵ
    final_pitch_angle_set=(int32_t)(-filtered_vertical_pixel_jscope-gimbal_control.gimbal_pitch_motor.relative_angle*RAD_TO_DEGREE);//����Ҫʹ��delay��ı����������ʣ���Ҳ��֪��Ϊʲô

    filtered_final_yaw_angle_set_jscope=(int32_t)(filtered_final_angle_set[0]);
    filtered_final_pitch_angle_set_jscope=(int32_t)(filtered_final_angle_set[1]);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    temp_absolute_angle_reference_jscope=(int32_t)(temp_absolute_angle_reference*1000);
    yaw_ins_raw_int_1000=(int32_t)(gimbal_control.gimbal_yaw_motor.absolute_angle_raw * 1000);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    prediciton_filtered_final_yaw_angle_set_jscope=(int32_t)(filtered_final_angle_set[0]-PREDICTION_TIME*RAD_TO_DEGREE*filtered_aim_data[2]);
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
    gimbal_feedback_update->gimbal_pitch_motor.absolute_angle = *(gimbal_feedback_update->gimbal_INT_angle_point + INS_PITCH_ADDRESS_OFFSET);


    gimbal_feedback_update->gimbal_pitch_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_pitch_motor.gimbal_motor_measure->ecd,
            gimbal_feedback_update->gimbal_pitch_motor.offset_ecd);
    gimbal_feedback_update->gimbal_pitch_motor.motor_gyro = *(gimbal_feedback_update->gimbal_INT_gyro_point + INS_GYRO_Y_ADDRESS_OFFSET);


    //������������ֱ�Ӷ�ȡ����ֵ������Ϊabsolute_angle_raw
    gimbal_feedback_update->gimbal_yaw_motor.absolute_angle = *(gimbal_feedback_update->gimbal_INT_angle_point + INS_YAW_ADDRESS_OFFSET);


//    ///////////////////////////////////////////////////////////////////////////////////////////////
////    //���¼���absolute_angle��ʹ��ÿ�ο�������ʱ��ǰλ�õ���ֵΪ0//���ȶ�����������ʱ�ᶶ��
//    gimbal_feedback_update->gimbal_yaw_motor.absolute_angle=gimbal_feedback_update->gimbal_yaw_motor.absolute_angle_raw-temp_absolute_angle_reference;
//    if(gimbal_feedback_update->gimbal_yaw_motor.absolute_angle<=(temp_absolute_angle_reference-PI))
//    {
//        gimbal_feedback_update->gimbal_yaw_motor.absolute_angle+=2*PI;
//    }
//    if(gimbal_feedback_update->gimbal_yaw_motor.absolute_angle>PI)
//    {
//        gimbal_feedback_update->gimbal_yaw_motor.absolute_angle-=2*PI;
//    }
//    ///////////////////////////////////////////////////////////////////////////////////////////////



    gimbal_feedback_update->gimbal_yaw_motor.relative_angle = motor_ecd_to_angle_change(gimbal_feedback_update->gimbal_yaw_motor.gimbal_motor_measure->ecd,
            gimbal_feedback_update->gimbal_yaw_motor.offset_ecd);

    gimbal_feedback_update->gimbal_yaw_motor.motor_gyro = arm_cos_f32(gimbal_feedback_update->gimbal_pitch_motor.relative_angle) * (*(gimbal_feedback_update->gimbal_INT_gyro_point + INS_GYRO_Z_ADDRESS_OFFSET))
            - arm_sin_f32(gimbal_feedback_update->gimbal_pitch_motor.relative_angle) * (*(gimbal_feedback_update->gimbal_INT_gyro_point + INS_GYRO_X_ADDRESS_OFFSET));
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
    //YAW���״̬���л���������
    if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_yaw_motor.raw_cmd_current = gimbal_mode_change->gimbal_yaw_motor.current_set = gimbal_mode_change->gimbal_yaw_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_GYRO && gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        gimbal_mode_change->gimbal_yaw_motor.absolute_angle_set = gimbal_mode_change->gimbal_yaw_motor.absolute_angle;
    }
    else if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCONDE && gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        gimbal_mode_change->gimbal_yaw_motor.relative_angle_set = gimbal_mode_change->gimbal_yaw_motor.relative_angle;
    }
    //�¼����������ģʽ
    else if (gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_AIM && gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_AIM)
    {
        //����������-״̬�л�
//        temp_absolute_angle_reference=gimbal_mode_change->gimbal_yaw_motor.absolute_angle_raw;//��absolute_angle_raw��Ϊ����ֵ
        gimbal_mode_change->gimbal_yaw_motor.final_absolute_angle_set=gimbal_mode_change->gimbal_yaw_motor.absolute_angle;
        vTaskDelay(50);//�Է���������ʱ����
    }
    gimbal_mode_change->gimbal_yaw_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_yaw_motor.gimbal_motor_mode;


    //PITCH���״̬���л���������
    if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_RAW && gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        gimbal_mode_change->gimbal_pitch_motor.raw_cmd_current = gimbal_mode_change->gimbal_pitch_motor.current_set = gimbal_mode_change->gimbal_pitch_motor.given_current;
    }
    else if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_GYRO && gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        gimbal_mode_change->gimbal_pitch_motor.absolute_angle_set = gimbal_mode_change->gimbal_pitch_motor.absolute_angle;
    }
    else if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_ENCONDE && gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        gimbal_mode_change->gimbal_pitch_motor.relative_angle_set = gimbal_mode_change->gimbal_pitch_motor.relative_angle;
    }
    //�¼����������ģʽ
    else if (gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode != GIMBAL_MOTOR_AIM && gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_AIM)
    {
        //����������-״̬�л�
        gimbal_mode_change->gimbal_pitch_motor.relative_angle=0;
        gimbal_mode_change->gimbal_pitch_motor.offset_ecd=gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_measure->ecd;
        gimbal_mode_change->gimbal_pitch_motor.relative_angle_set = 0;
				gimbal_mode_change->gimbal_pitch_motor.relative_angle_set = gimbal_mode_change->gimbal_pitch_motor.relative_angle;
    }

    gimbal_mode_change->gimbal_pitch_motor.last_gimbal_motor_mode = gimbal_mode_change->gimbal_pitch_motor.gimbal_motor_mode;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//��̨����������
static void GIMBAL_Set_Contorl(Gimbal_Control_t *gimbal_set_control)
{
    if (gimbal_set_control == NULL)
    {
        return;
    }

    fp32 add_yaw_angle = 0.0f;
    fp32 add_pitch_angle = 0.0f;

    gimbal_behaviour_control_set(&add_yaw_angle, &add_pitch_angle, gimbal_set_control);
    //yaw���ģʽ����
    if (gimbal_set_control->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //rawģʽ�£�ֱ�ӷ��Ϳ���ֵ
        gimbal_set_control->gimbal_yaw_motor.raw_cmd_current = add_yaw_angle;
    }
    else if (gimbal_set_control->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyroģʽ�£������ǽǶȿ���
        GIMBAL_absolute_angle_limit(&gimbal_set_control->gimbal_yaw_motor, add_yaw_angle);
    }
    else if (gimbal_set_control->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //encondeģʽ�£��������Ƕȿ���
        GIMBAL_relative_angle_limit(&gimbal_set_control->gimbal_yaw_motor, add_yaw_angle);
    }

    //pitch���ģʽ����
    if (gimbal_set_control->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //rawģʽ�£�ֱ�ӷ��Ϳ���ֵ
        gimbal_set_control->gimbal_pitch_motor.raw_cmd_current = add_pitch_angle;
    }
    else if (gimbal_set_control->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyroģʽ�£������ǽǶȿ���
        GIMBAL_absolute_angle_limit(&gimbal_set_control->gimbal_pitch_motor, add_pitch_angle);
    }
    else if (gimbal_set_control->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //encondeģʽ�£��������Ƕȿ���
        GIMBAL_relative_angle_limit(&gimbal_set_control->gimbal_pitch_motor, add_pitch_angle);
    }
}
//������ ����������
static void GIMBAL_absolute_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add)
{
    static fp32 bias_angle;
    static fp32 angle_set;
    if (gimbal_motor == NULL)
    {
        return;
    }
    //��ǰ�������Ƕ�
    bias_angle = rad_format(gimbal_motor->absolute_angle_set - gimbal_motor->absolute_angle);
    //��̨��ԽǶ�+ ���Ƕ� + �����Ƕ� ������� ����е�Ƕ�
    if (gimbal_motor->relative_angle + bias_angle + add > gimbal_motor->max_relative_angle)
    {
        //�����������е�Ƕȿ��Ʒ���
        if (add > 0.0f)
        {
            //�����һ��������ӽǶȣ�
            add = gimbal_motor->max_relative_angle - gimbal_motor->relative_angle - bias_angle;
        }
    }
    else if (gimbal_motor->relative_angle + bias_angle + add < gimbal_motor->min_relative_angle)
    {
        if (add < 0.0f)
        {
            add = gimbal_motor->min_relative_angle - gimbal_motor->relative_angle - bias_angle;
        }
    }
    angle_set = gimbal_motor->absolute_angle_set;
    gimbal_motor->absolute_angle_set = rad_format(angle_set + add);
}

static void GIMBAL_relative_angle_limit(Gimbal_Motor_t *gimbal_motor, fp32 add)
{
    if (gimbal_motor == NULL)
    {
        return;
    }
    gimbal_motor->relative_angle_set += add;
    //�Ƿ񳬹���� ��Сֵ
    if (gimbal_motor->relative_angle_set > gimbal_motor->max_relative_angle)
    {
        gimbal_motor->relative_angle_set = gimbal_motor->max_relative_angle;
    }
    else if (gimbal_motor->relative_angle_set < gimbal_motor->min_relative_angle)
    {
        gimbal_motor->relative_angle_set = gimbal_motor->min_relative_angle;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��̨����״̬ʹ�ò�ͬ����pid
static void GIMBAL_Control_loop(Gimbal_Control_t *gimbal_control_loop)
{
    if (gimbal_control_loop == NULL)
    {
        return;
    }
    //YAW��ͬģʽ���ڲ�ͬ�Ŀ��ƺ���
    if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw����
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_yaw_motor);
    }
    else if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
        //gyro�Ƕȿ���
        gimbal_motor_drive_mode_control_yaw(&gimbal_control_loop->gimbal_yaw_motor);//��ʻ��ʽ-���̸�����̨����
    }
    else if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //enconde�Ƕȿ���
        gimbal_motor_relative_angle_control_yaw(&gimbal_control_loop->gimbal_yaw_motor);//��ͨģʽ-��̨����
    }
    //�¼����������
    else if (gimbal_control_loop->gimbal_yaw_motor.gimbal_motor_mode == GIMBAL_MOTOR_AIM)
    {
        //����ģʽ-������
        //gimbal_motor_aim_control_yaw(&gimbal_control_loop->gimbal_yaw_motor);
        //����ģʽ-������
        gimbal_motor_aim_control_gyro_yaw(&gimbal_control_loop->gimbal_yaw_motor);
    }



    //PITCH��ͬģʽ���ڲ�ͬ�Ŀ��ƺ���
    if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_RAW)
    {
        //raw����
        gimbal_motor_raw_angle_control(&gimbal_control_loop->gimbal_pitch_motor);
    }
    else if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_GYRO)
    {
//        //enconde�Ƕȿ���
//        gimbal_motor_relative_angle_control_pitch(&gimbal_control_loop->gimbal_pitch_motor);
////				//gyro�Ƕȿ���
////        gimbal_motor_absolute_angle_control(&gimbal_control_loop->gimbal_pitch_motor);
			  //gyro�Ƕȿ���
        gimbal_motor_drive_mode_control_pitch(&gimbal_control_loop->gimbal_pitch_motor);//��ʻģʽ-���̸�����̨����
    }
    else if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_ENCONDE)
    {
        //enconde�Ƕȿ���
        gimbal_motor_relative_angle_control_pitch(&gimbal_control_loop->gimbal_pitch_motor);
    }
    //�¼����������
    else if (gimbal_control_loop->gimbal_pitch_motor.gimbal_motor_mode == GIMBAL_MOTOR_AIM)
    {
        //enconde�Ƕȿ���
        gimbal_motor_aim_control_pitch(&gimbal_control_loop->gimbal_pitch_motor);
    }
}



//static void gimbal_motor_absolute_angle_control(Gimbal_Motor_t *gimbal_motor)
//{
//    if (gimbal_motor == NULL)
//    {
//        return;
//    }
//    //�ǶȻ����ٶȻ�����pid����
//    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_absolute_angle_pid, gimbal_motor->absolute_angle, gimbal_motor->absolute_angle_set, gimbal_motor->motor_gyro);
//    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_gyro_pid, gimbal_motor->motor_gyro, gimbal_motor->motor_gyro_set);
//    //����ֵ��ֵ
//    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
//}


//��ʻģʽ���޷����飬��Ϊ��ʻ������
static void gimbal_motor_drive_mode_control_yaw(Gimbal_Motor_t *gimbal_motor)//��ʻģʽ��YAW���
{
    if (gimbal_motor == NULL)
    {
        return;
    }
		//�ж��Ƿ��������������
    if (tx2.raw_horizontal_pixel!=9999 && tx2.raw_horizontal_pixel!=0)//���ԭʼ�������ݲ����ظ�����δ����
    {
        tx2.horizontal_pixel=tx2.raw_horizontal_pixel;//��ֵ��������
    }	
		
    static fp32 delta_yaw;//yaw����Ƕ�Ŀ�����
    delta_yaw=(fp32)(gimbal_control.gimbal_rc_ctrl->rc.ch[YawChannel])*-0.00025f+(fp32)(gimbal_control.gimbal_rc_ctrl->mouse.x)*-0.0075f;

    //�ǶȻ����ٶȻ�����pid����
    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_relative_angle_pid, gimbal_motor->relative_angle, gimbal_motor->relative_angle+delta_yaw, gimbal_motor->motor_gyro);
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_gyro_pid, gimbal_motor->motor_gyro, gimbal_motor->motor_gyro_set);
    //����ֵ��ֵ
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}
//��ʻģʽ���޷����飬��Ϊ��ʻ������
static void gimbal_motor_drive_mode_control_pitch(Gimbal_Motor_t *gimbal_motor)//��ʻģʽ��PITCH���
{
    if (gimbal_motor == NULL)
    {
        return;
    }
		
    //�ж��Ƿ���������ݸ���
    if (tx2.raw_vertical_pixel!=9999 && tx2.raw_vertical_pixel!=0)//���ԭʼ�������ݲ����ظ�����δ����
    {
        tx2.vertical_pixel=tx2.raw_vertical_pixel;//��ֵ��������
    }
		
		static fp32 delta_pitch;//pitch����Ƕ�Ŀ�����
		delta_pitch=(fp32)(gimbal_control.gimbal_rc_ctrl->rc.ch[PitchChannel])*(-0.00001f)+(fp32)(gimbal_control.gimbal_rc_ctrl->mouse.y)*0.00005f;
		//����relative_angle_set��ֵ���ﵽ����λ�û�
		gimbal_motor->relative_angle_set+=delta_pitch;
		    
		//������̨pitch������·���
    fp32 pitch_limit=DEGREE_TO_RAD*200;//��С�����Ͻ磬��������Ͻ�
//		fp32 pitch_limit_offset=DEGREE_TO_RAD*0;//��С�����½磬��������½�
    if(gimbal_motor->relative_angle_set>pitch_limit)//�½磬20��
    {
        gimbal_motor->relative_angle_set=pitch_limit;
        buzzer_on(60,5000);
    }
    else if(gimbal_motor->relative_angle_set<-pitch_limit*3.0f)//�Ͻ磬60��
    {
        gimbal_motor->relative_angle_set=-pitch_limit*3.0f;
        buzzer_on(60,5000);
    }
    else
    {
        buzzer_off();
    }
		
		
    //�ǶȻ����ٶȻ�����pid����
    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_relative_angle_pid, gimbal_motor->relative_angle, gimbal_motor->relative_angle_set, gimbal_motor->motor_gyro);
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_gyro_pid, gimbal_motor->motor_gyro, gimbal_motor->motor_gyro_set);
    //����ֵ��ֵ
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}

//����ģʽ-������-YAW���
static void gimbal_motor_aim_control_gyro_yaw(Gimbal_Motor_t *gimbal_motor)
{
    if (gimbal_motor == NULL)
    {
        return;
    }
    static fp32 delta_yaw;//yaw����Ƕ�Ŀ�����
//    static fp32 final_absolute_yaw_angle_set; //yaw����������Ŀ��Ƕ�


    delta_yaw=(fp32)((filtered_final_angle_set[0]-(PREDICTION_TIME+prediction_time_offset)*RAD_TO_DEGREE*filtered_aim_data[2])//Ԥ��
                     *-DEGREE_TO_RAD*1.0f);

    //��ҡ��/������absolute_angle_set��ֵ����һ����Χ����ʱ�ֶ�����׼��
    gimbal_motor->absolute_angle_set=(fp32)(gimbal_control.gimbal_rc_ctrl->rc.ch[2])*-0.00025f+(fp32)(gimbal_control.gimbal_rc_ctrl->mouse.x)*-0.0035f;



    //�ж��Ƿ����
    if (tx2.raw_horizontal_pixel==9999 || tx2.raw_horizontal_pixel==0)//���ԭʼ�������ݷ��ظ���
    {
        gimbal_motor->final_absolute_angle_set=gimbal_motor->absolute_angle;//set��Ϊ��ǰ�Ƕ�ʹ��ֹ̨ͣ�ƶ�
    }
    else
    {
        tx2.horizontal_pixel=tx2.raw_horizontal_pixel;//��ֵ��������

        //�ı����վ��ԽǶ�

        gimbal_motor->final_absolute_angle_set=gimbal_motor->absolute_angle_set+delta_yaw;

    }
    
		//�ǶȻ����ٶȻ�����pid����
    /*******************************************************************/
    /*************��error��ʱ�ֶ�target��ǰ �Ѳ��� Ч����***************/
    /*******************************************************************/
    if(abs((int)delta_yaw)>PI/6)
    {
        gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_absolute_angle_pid, gimbal_motor->absolute_angle, gimbal_motor->final_absolute_angle_set, gimbal_motor->motor_gyro*0.3f);
    }
    else
    {
        gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_absolute_angle_pid, gimbal_motor->absolute_angle, gimbal_motor->final_absolute_angle_set, gimbal_motor->motor_gyro);
    }
    /********************************************************************/
    /*************************������ѧ��ɾ��*****************************/
    /********************************************************************/

		
//				//������̨yaw������ҷ���
//		fp32 yaw_limit=DEGREE_TO_RAD*900;//������չ���ҽ磬��С�������ҽ�
//		if(gimbal_motor->final_absolute_angle_set>yaw_limit)//���
//		{
//			gimbal_motor->final_absolute_angle_set=yaw_limit;
//			buzzer_on(60,5000);
//		}
//		else if(gimbal_motor->final_absolute_angle_set<-yaw_limit)//�ҽ�
//		{
//			gimbal_motor->final_absolute_angle_set=-yaw_limit;
//			buzzer_on(60,5000);
//		}
//		else
//		{
//			buzzer_off();
//		}
		
		
		
//		gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_absolute_angle_pid, gimbal_motor->absolute_angle, gimbal_motor->final_absolute_angle_set, gimbal_motor->motor_gyro);
		
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_gyro_pid, gimbal_motor->motor_gyro, gimbal_motor->motor_gyro_set);
    //����ֵ��ֵ
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);

}
//����ģʽ-ͨ��PITCH���
static void gimbal_motor_aim_control_pitch(Gimbal_Motor_t *gimbal_motor)
{
    if (gimbal_motor == NULL)
    {
        return;
    }

    static fp32 delta_pitch;//pitch����Ƕ�Ŀ�����
//    static fp32 final_relative_pitch_angle_set;



    delta_pitch=(fp32)(filtered_final_angle_set[1])
                *-DEGREE_TO_RAD*1.0f;

//		//����relative_angle_set��ֵ���ﵽ����λ�û�
//		gimbal_motor->relative_angle_set+=delta_pitch;


    //�ı����վ��ԽǶ�
    gimbal_motor->final_relative_angle_set=gimbal_motor->relative_angle_set+delta_pitch;

    //�ж��Ƿ����
    if (tx2.raw_vertical_pixel==9999 || tx2.raw_vertical_pixel==0)//���ԭʼ�������ݷ��ظ���
    {
        gimbal_motor->final_relative_angle_set=gimbal_motor->relative_angle;//set��Ϊ��ǰ�Ƕ�ʹ��ֹ̨ͣ�ƶ�
    }
    else
    {
        tx2.vertical_pixel=tx2.raw_vertical_pixel;//��ֵ��������
        //�ı����վ��ԽǶ�
        gimbal_motor->final_relative_angle_set=gimbal_motor->relative_angle_set+delta_pitch;
    }



//		//������̨pitch������·���
//		fp32 pitch_limit=data_to_deg_ratio*300;//��С�����Ͻ磬��������Ͻ�
//		fp32 pitch_limit_offset=DEGREE_TO_RAD*100;//��С�����½磬��������½�
//		if(final_relative_pitch_angle_set>pitch_limit-pitch_limit_offset)//�½�
//		{
//			final_relative_pitch_angle_set=pitch_limit-pitch_limit_offset;
//			buzzer_on(60,5000);
//		}
//		else if(final_relative_pitch_angle_set<-pitch_limit)//�Ͻ�
//		{
//			final_relative_pitch_angle_set=-pitch_limit;
//			buzzer_on(60,5000);
//		}
//		else
//		{
//			//buzzer_off();
//		}

    //�ǶȻ����ٶȻ�����pid����
    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_relative_angle_pid, gimbal_motor->relative_angle, gimbal_motor->final_relative_angle_set, gimbal_motor->motor_gyro);
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_gyro_pid, gimbal_motor->motor_gyro, gimbal_motor->motor_gyro_set);
    //����ֵ��ֵ
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);
}
static void gimbal_motor_relative_angle_control_yaw(Gimbal_Motor_t *gimbal_motor)//��ͨģʽ��YAW���
{
    if (gimbal_motor == NULL)
    {
        return;
    }

//    static fp32 delta_yaw;//yaw����Ƕ�Ŀ�����

//    delta_yaw=(fp32)(gimbal_control.gimbal_rc_ctrl->rc.ch[YawChannel])*-0.000005f
//              +(fp32)(gimbal_control.gimbal_rc_ctrl->mouse.x)*-0.00025f;////-0.000025f//relative_angle_set��������ϵ��///////-0.0025f;//relative_angle+delta_yaw��������ϵ��




//    //����relative_angle_set��ֵ���ﵽ����λ�û�
//    gimbal_motor->relative_angle_set+=delta_yaw;
//    
		//�ж��Ƿ��������������
    if (tx2.raw_horizontal_pixel!=9999 && tx2.raw_horizontal_pixel!=0)//���ԭʼ�������ݲ����ظ�����δ����
    {
        tx2.horizontal_pixel=tx2.raw_horizontal_pixel;//��ֵ��������
    }		
		
    //������̨yaw������ҷ���
    fp32 yaw_limit=DEGREE_TO_RAD*900;//������չ���ҽ磬��С�������ҽ�
    if(gimbal_motor->relative_angle_set>yaw_limit)//���
    {
        gimbal_motor->relative_angle_set=yaw_limit;
        buzzer_on(60,5000);
    }
    else if(gimbal_motor->relative_angle_set<-yaw_limit)//�ҽ�
    {
        gimbal_motor->relative_angle_set=-yaw_limit;
        buzzer_on(60,5000);
    }
    else
    {
//        buzzer_off();
    }


    //�ǶȻ����ٶȻ�����pid����
    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_relative_angle_pid, gimbal_motor->relative_angle, gimbal_motor->relative_angle_set, gimbal_motor->motor_gyro);
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_gyro_pid, gimbal_motor->motor_gyro, gimbal_motor->motor_gyro_set);

    //��ȡ��������ֵ
    //(gimbal_motor->gimbal_motor_measure->ecd<=5097)//���� 1417��5143�����м� 1C17��7191��������0418��8191+1048��

    //����ֵ��ֵ
    gimbal_motor->given_current = (int16_t)(gimbal_motor->current_set);

}

static void gimbal_motor_relative_angle_control_pitch(Gimbal_Motor_t *gimbal_motor)//��ͨģʽ��PITCH���
{
    if (gimbal_motor == NULL)
    {
        return;
    }

//    static fp32 delta_pitch;//pitch����Ƕ�Ŀ�����

//    delta_pitch=(fp32)(gimbal_control.gimbal_rc_ctrl->rc.ch[PitchChannel])*-0.000005f
//                +(fp32)(gimbal_control.gimbal_rc_ctrl->mouse.y)*0.00025f;////0.000025f//relative_angle_setʱ��������ϵ��///////-0.0025f;//relative_angle+delta_pitch��������ϵ��

//		//����relative_angle_set��ֵ���ﵽ����λ�û�
//    gimbal_motor->relative_angle_set+=delta_pitch;
		
		
    //�ж��Ƿ���������ݸ���
    if (tx2.raw_vertical_pixel!=9999 && tx2.raw_vertical_pixel!=0)//���ԭʼ�������ݲ����ظ�����δ����
    {
        tx2.vertical_pixel=tx2.raw_vertical_pixel;//��ֵ��������
    }
		


    //������̨pitch������·���
    fp32 pitch_limit=DEGREE_TO_RAD*300;//��С�����Ͻ磬��������Ͻ�
//		fp32 pitch_limit_offset=DEGREE_TO_RAD*0;//��С�����½磬��������½�
    if(gimbal_motor->relative_angle_set>pitch_limit*1.5f)//�½�
    {
        gimbal_motor->relative_angle_set=pitch_limit*1.5f;
        buzzer_on(60,5000);
    }
    else if(gimbal_motor->relative_angle_set<-pitch_limit)//�Ͻ�
    {
        gimbal_motor->relative_angle_set=-pitch_limit;
        buzzer_on(60,5000);
    }
    else
    {
        buzzer_off();
    }


    //�ǶȻ����ٶȻ�����pid����
    gimbal_motor->motor_gyro_set = GIMBAL_PID_Calc(&gimbal_motor->gimbal_motor_relative_angle_pid, gimbal_motor->relative_angle, gimbal_motor->relative_angle_set, gimbal_motor->motor_gyro);
    gimbal_motor->current_set = PID_Calc(&gimbal_motor->gimbal_motor_gyro_pid, gimbal_motor->motor_gyro, gimbal_motor->motor_gyro_set);
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
