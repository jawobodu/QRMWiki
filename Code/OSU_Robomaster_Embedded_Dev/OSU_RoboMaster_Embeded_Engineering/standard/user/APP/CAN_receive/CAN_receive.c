/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       can_receive.c/h
  * @brief      can device transmit and recevice function��receive via CAN interrupt
  * @note       This is NOT a freeRTOS TASK
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. Compete
  *  V1.0.1     Feb-17-2019     Tony-OSU        Add tx2 can bus config
	*  V1.1.0     Feb-21-2019     Tony-OSU        Finish Custom CAN Bus, fully functional
	*  V1.2.0     Mar-01-2019     Tony-OSU        CAN unpackaging simplified. Pixel bias changed.
	* 																						@note some packages ID has CHANGED!! See .h file for detail
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2016 DJI****************************
	**************Modifid by Ohio State University Robomaster Team****************
  */

#include "CAN_Receive.h"

#include "stm32f4xx.h"
#include "rng.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "buzzer.h"
#include "Detect_Task.h"
#include "pid.h"


void CAN_CMD_GIMBAL(int16_t yaw, int16_t pitch, int16_t shoot, int16_t rev);

//Read Chassis Motor data
//���̵�����ݶ�ȡ 3508
//"ecd" represents "encoder"
//Left shift first 8-bit message by 8 bits, then add(by Bitwise Or) second 8-bit message together to generate an entire 16-bit message   
#define get_motor_measure(ptr, rx_message)                                                 \
{                                                                                          \
		(ptr)->last_ecd = (ptr)->ecd;                                                          \
		(ptr)->ecd = (uint16_t)((rx_message)->Data[0] << 8 | (rx_message)->Data[1]);           \
		(ptr)->speed_rpm = (uint16_t)((rx_message)->Data[2] << 8 | (rx_message)->Data[3]);     \
		(ptr)->given_current = (uint16_t)((rx_message)->Data[4] << 8 | (rx_message)->Data[5]); \
		(ptr)->temperate = (rx_message)->Data[6];                                              \
}

//Read Gimbal Motor data
//��̨������ݶ�ȡ
#define get_gimbal_motor_measure(ptr, rx_message)                                          \
{                                                                                          \
		(ptr)->last_ecd = (ptr)->ecd;                                                          \
		(ptr)->ecd = (uint16_t)((rx_message)->Data[0] << 8 | (rx_message)->Data[1]);           \
		(ptr)->given_current = (uint16_t)((rx_message)->Data[2] << 8 | (rx_message)->Data[3]); \
		(ptr)->speed_rpm = (uint16_t)((rx_message)->Data[4] << 8 | (rx_message)->Data[5]);     \
		(ptr)->temperate = (rx_message)->Data[6];                                              \
}


//Process CAN Receive funtion together
//ͳһ����CAN���պ���
static void CAN_hook(CanRxMsg *rx_message);
		
//Declare Motor variables
//�����������
static motor_measure_t motor_arm1, motor_arm2, motor_arm3, motor_arm4, motor_chassis[4];
    
//Declare Gimbal Sending Message
//������е�۵ķ�����Ϣ
static CanTxMsg ARM_TxMessage;

//If Gimbal Motor fails to send CAN message, initially define delay_time as 100 ms
//�����̨�������CANʧ�ܣ���ʼ����delay_timeΪ100ms
#if GIMBAL_MOTOR_6020_CAN_LOSE_SLOVE
static uint8_t delay_time = 100;
#endif

//CAN1 Interrupt
//CAN1�ж�
void CAN1_RX0_IRQHandler(void)
{
    static CanRxMsg rx1_message;

    if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
    {
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);//Clear the CAN1 interrupt flag to avoid entering the interrupt immediately after exiting the interrupt
        CAN_Receive(CAN1, CAN_FIFO0, &rx1_message);
        CAN_hook(&rx1_message);//wait to be processed
    }
}

//CAN2 Interrupt
//CAN2�ж�
void CAN2_RX0_IRQHandler(void)
{
    static CanRxMsg rx2_message;
    if (CAN_GetITStatus(CAN2, CAN_IT_FMP0) != RESET)
    {
        CAN_ClearITPendingBit(CAN2, CAN_IT_FMP0);//Clear the CAN2 interrupt flag to avoid entering the interrupt immediately after exiting the interrupt
        CAN_Receive(CAN2, CAN_FIFO0, &rx2_message);
        CAN_hook(&rx2_message);//wait to be processed
    }
}

//If Gimbal Motor fails to send CAN message, try to solve by sending command in random delay time
//�����̨���CAN����ʧ�ܣ�����ʹ�� ����ӳ� ���Ϳ���ָ��ķ�ʽ���
#if GIMBAL_MOTOR_6020_CAN_LOSE_SLOVE
void GIMBAL_lose_slove(void)
{
        delay_time = RNG_get_random_range(13,239);
}
#endif
//Transmit ARM Control command, "rev" is reserved data
//���ͻ�е�ۿ����������revΪ�����ֽ�
void CAN_CMD_ARM(int16_t arm1, int16_t arm2, int16_t arm3, int16_t arm4)
{
    ARM_TxMessage.StdId = CAN_ARM_ALL_ID;
    ARM_TxMessage.IDE = CAN_ID_STD;//CAN_identifier_type=standard
    ARM_TxMessage.RTR = CAN_RTR_DATA;
    ARM_TxMessage.DLC = 0x08;//length of data
    ARM_TxMessage.Data[0] = (arm1 >> 8);
    ARM_TxMessage.Data[1] = arm1;
    ARM_TxMessage.Data[2] = (arm2 >> 8);
    ARM_TxMessage.Data[3] = arm2;
    ARM_TxMessage.Data[4] = (arm3 >> 8);
    ARM_TxMessage.Data[5] = arm3;
    ARM_TxMessage.Data[6] = (arm4 >> 8);
    ARM_TxMessage.Data[7] = arm4;
//If Gimbal Motor fails to send CAN message
#if GIMBAL_MOTOR_6020_CAN_LOSE_SLOVE

    TIM6->CNT = 0;//clear count of TIM6 
    TIM6->ARR = delay_time ;//set Auto-Reload Register as delay_time

    TIM_Cmd(TIM6,ENABLE);//Enable TIM6
#else
    CAN_Transmit( ARM_CAN,  &ARM_TxMessage );
#endif

}
//TIM6 Timer Interrupt
//TIM6��ʱ���ж�
void TIM6_DAC_IRQHandler(void)
{
    if( TIM_GetITStatus( TIM6, TIM_IT_Update )!= RESET )
    {

        TIM_ClearFlag( TIM6, TIM_IT_Update );
#if GIMBAL_MOTOR_6020_CAN_LOSE_SLOVE
        CAN_Transmit( GIMBAL_CAN,  &GIMBAL_TxMessage );
#endif
        TIM_Cmd(TIM6,DISABLE);//Disable TIM6
    }
}
//CAN transmits the data of 0x700's ID��trigger M3508 Gear Motor into Quick ID Setting Mode
//CAN ���� 0x700��ID�����ݣ�������M3508�����������IDģʽ
void CAN_CMD_CHASSIS_RESET_ID(void)
{

    CanTxMsg TxMessage;
    TxMessage.StdId = 0x700;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = 0;
    TxMessage.Data[1] = 0;
    TxMessage.Data[2] = 0;
    TxMessage.Data[3] = 0;
    TxMessage.Data[4] = 0;
    TxMessage.Data[5] = 0;
    TxMessage.Data[6] = 0;
    TxMessage.Data[7] = 0;

    CAN_Transmit(CAN2, &TxMessage);
}

//Transmit Chassis Control command
//���͵��̵����������
void CAN_CMD_CHASSIS(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4)
{
	  //Transmit config
    CanTxMsg TxMessage;
    TxMessage.StdId = CAN_CHASSIS_ALL_ID;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = 0x08;
    TxMessage.Data[0] = motor1 >> 8;
    TxMessage.Data[1] = motor1;
    TxMessage.Data[2] = motor2 >> 8;
    TxMessage.Data[3] = motor2;
    TxMessage.Data[4] = motor3 >> 8;
    TxMessage.Data[5] = motor3;
    TxMessage.Data[6] = motor4 >> 8;
    TxMessage.Data[7] = motor4;

    CAN_Transmit(CHASSIS_CAN, &TxMessage);
}


//���ػ�е��1���������ַ��ͨ��ָ�뷽ʽ��ȡԭʼ����
const motor_measure_t *get_Arm1_Motor_Measure_Point(void)
{
    return &motor_arm1;
}

//���ػ�е��2���������ַ��ͨ��ָ�뷽ʽ��ȡԭʼ����
const motor_measure_t *get_Arm2_Motor_Measure_Point(void)
{
    return &motor_arm2;
}
//���ػ�е��3���������ַ��ͨ��ָ�뷽ʽ��ȡԭʼ����
const motor_measure_t *get_Arm3_Motor_Measure_Point(void)
{
    return &motor_arm3;
}
//���ػ�е��4���������ַ��ͨ��ָ�뷽ʽ��ȡԭʼ����
const motor_measure_t *get_Arm4_Motor_Measure_Point(void)
{
    return &motor_arm4;
}
//Return Chassis Address of motor��retrieve original data through Pointer
//���ص��̵��������ַ��ͨ��ָ�뷽ʽ��ȡԭʼ����
const motor_measure_t *get_Chassis_Motor_Measure_Point(uint8_t i)
{
    return &motor_chassis[(i & 0x03)];
}

//Process CAN Interrupt funtion together��record the time of sending data as reference of offline
//ͳһ����CAN�жϺ��������Ҽ�¼�������ݵ�ʱ�䣬��Ϊ�����ж�����
static void CAN_hook(CanRxMsg *rx_message)
{
    switch (rx_message->StdId)
    {
    case CAN_ARM1_MOTOR_ID:
    {
        //Process ARM1 Gimbal Motor Function
				//����ARM1������ݺ꺯��
        get_motor_measure(&motor_arm1, rx_message);
        //Record time
				//��¼ʱ��
        DetectHook(Arm1MotorTOE);
        break;
    }
    case CAN_ARM2_MOTOR_ID:
    {
        //Process ARM2 Gimbal Motor Function
				//����ARM2������ݺ꺯��
        get_motor_measure(&motor_arm2, rx_message);//3508 get_motor_measure 6020 get_gimbal_motor_measure
				//Record time
				//��¼ʱ��
				DetectHook(Arm2MotorTOE);
        break;
    }
    case CAN_ARM3_MOTOR_ID:
    {
        //Process ARM3 Motor Function
				//����ARM3������ݺ꺯��
				get_motor_measure(&motor_arm3, rx_message);
				//Record time
				//��¼ʱ��
        DetectHook(Arm3MotorTOE);
        break;
    }
		 case CAN_ARM4_MOTOR_ID:
    {
        //Process ARM4 Motor Function
				//����ARM4������ݺ꺯��
				get_motor_measure(&motor_arm4, rx_message);
				//Record time
				//��¼ʱ��
        DetectHook(Arm4MotorTOE);
        break;
    }
    case CAN_3508_M1_ID://�ĸ����̵��
    case CAN_3508_M2_ID:
    case CAN_3508_M3_ID:
    case CAN_3508_M4_ID:
    {
        static uint8_t i = 0;
				//Get Motor ID
        //������ID��
        i = rx_message->StdId - CAN_3508_M1_ID;
				//Process Motor #i Measure
        //�����Ӧ������ݺ꺯��
        get_motor_measure(&motor_chassis[i], rx_message);
				//Record time
        //��¼ʱ��
        DetectHook(ChassisMotor1TOE + i);
        break;
    }
    
    default:
    {
        break;
    }
    }
}
