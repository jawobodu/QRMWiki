#include "main.h"
#include "math.h"
#define Kp 0.3f                	// proportional gain governs rate of convergence to accelerometer/magnetometer
#define Ki 0.001f                	// 0.001  integral gain governs rate of convergence of gyroscope biases

#define IMU_INTEGRAL_LIM  ( 2.0f *ANGLE_TO_RADIAN )
#define NORM_ACC_LPF_HZ 10  		//(Hz)
#define REF_ERR_LPF_HZ  1				//(Hz)

xyz_f_t reference_v;
ref_t 	ref;
#define _xyz_f_t xyz_f_t
//xyz_f_t Gravity_Vec;  				//�������������
	
float Roll,Pitch,Yaw;    				//��̬��

float ref_q[4] = {1,0,0,0};
float norm_acc,norm_q;
float norm_acc_lpf;

float mag_norm ,mag_norm_xyz ;
void simple_3d_trans(_xyz_f_t *ref, _xyz_f_t *in, _xyz_f_t *out) //С��Χ����ȷ��
{
	static s8 pn;
	static float h_tmp_x,h_tmp_y;
	
	h_tmp_x = my_sqrt(my_pow(ref->z) + my_pow(ref->y));
	h_tmp_y = my_sqrt(my_pow(ref->z) + my_pow(ref->x));
	
	pn = ref->z < 0? -1 : 1;
	
	  out->x = ( h_tmp_x *in->x - pn *ref->x *in->z ) ;
		out->y = ( pn *h_tmp_y *in->y - ref->y *in->z ) ;
	
// 	 out->x = h_tmp_x *in->x - ref->x *in->z;
// 	 out->y = ref->z *in->y - ref->y *in->z;
	
	out->z = ref->x *in->x + ref->y *in->y + ref->z *in->z ;

}
xyz_f_t mag_sim_3d,acc_3d_hg,acc_ng,acc_ng_offset;

u8 acc_ng_cali;
extern u8 fly_ready;
void IMUupdate(float half_T,float gx, float gy, float gz, float ax, float ay, float az,float *rol,float *pit,float *yaw) 
{		
	float ref_err_lpf_hz;
	static float yaw_correct;
	float mag_norm_tmp;
	static xyz_f_t mag_tmp;
	static float yaw_mag;
	
	mag_norm_tmp = 20 *(6.28f *half_T);	
	
	mag_norm_xyz = my_sqrt(MagValue.x * MagValue.x + MagValue.y * MagValue.y + MagValue.z * MagValue.z);
	
		if( mag_norm_xyz != 0)
	{
		mag_tmp.x += mag_norm_tmp *( (float)MagValue.x /( mag_norm_xyz ) - mag_tmp.x);
		mag_tmp.y += mag_norm_tmp *( (float)MagValue.y /( mag_norm_xyz ) - mag_tmp.y);	
		mag_tmp.z += mag_norm_tmp *( (float)MagValue.z /( mag_norm_xyz ) - mag_tmp.z);	
	}

	/*
	void simple_3d_trans(_xyz_f_t *ref, _xyz_f_t *in, _xyz_f_t *out)
	
	���������ǻ��������µģ��Ҵų�������ƽ���ڵ��棬����ɻ���б��ͶӰ����ĽǶȻ������
	�˺�������һ����Χ��������ת�����ý���ƽ�ʵ�ʽǶȣ���С�ɻ���б��Ӱ�졣
	ע�⣺�ú����ڵļ��㲢������ȷҲ����׼ȷ�ģ���ȷ�ļ�����Ը��ӣ����ﲻ��������δ���İ汾�л��ٸ��¡�
	*/
	//mag_sim_3d=mag_tmp;
	simple_3d_trans(&reference_v,&mag_tmp,&mag_sim_3d); 
	
	mag_norm = my_sqrt(mag_sim_3d.x * mag_sim_3d.x + mag_sim_3d.y *mag_sim_3d.y);
	
	if( mag_sim_3d.x != 0 && mag_sim_3d.y != 0 && mag_sim_3d.z != 0 && mag_norm != 0)
	{
		yaw_mag = fast_atan2( ( mag_sim_3d.y/mag_norm ) , ( mag_sim_3d.x/mag_norm) ) *57.3f;
		
	}
	//=============================================================================
	// �����Ч��������
	reference_v.x = 2*(ref_q[1]*ref_q[3] - ref_q[0]*ref_q[2]);
	reference_v.y = 2*(ref_q[0]*ref_q[1] + ref_q[2]*ref_q[3]);
	reference_v.z = 1 - 2*(ref_q[1]*ref_q[1] + ref_q[2]*ref_q[2]);//ref_q[0]*ref_q[0] - ref_q[1]*ref_q[1] - ref_q[2]*ref_q[2] + ref_q[3]*ref_q[3];

	
	//���ǰ���Ԫ������ɡ��������Ҿ����еĵ����е�����Ԫ�ء�
	//�������Ҿ����ŷ���ǵĶ��壬��������ϵ������������ת����������ϵ��������������Ԫ�ء�
	//���������vx\y\z����ʵ���ǵ�ǰ��ŷ���ǣ�����Ԫ�����Ļ����������ϵ�ϣ����������������λ������       
	//=============================================================================

	if(acc_ng_cali)
	{
		if(acc_ng_cali==2)
		{
			acc_ng_offset.x = 0;
			acc_ng_offset.y = 0;
			acc_ng_offset.z = 0;
		}
			
		acc_ng_offset.x += 10 *0.23926f *(ax - 4096*reference_v.x) *0.0125f ;
		acc_ng_offset.y += 10 *0.23926f *(ay - 4096*reference_v.y) *0.0125f ;
		acc_ng_offset.z += 10 *0.23926f*(az - 4096*reference_v.z) *0.0125f ;	
		
		acc_ng_cali ++;
		if(acc_ng_cali>=82) //start on 2
		{
			acc_ng_cali = 0;
		}
	}
	
	acc_ng.x = 10 *0.23926f *(ax - 4096*reference_v.x) - acc_ng_offset.x;
	acc_ng.y = 10 *0.23926f *(ay - 4096*reference_v.y) - acc_ng_offset.y;
	acc_ng.z = 10 *0.23926f*(az - 4096*reference_v.z) - acc_ng_offset.z;
	
	acc_3d_hg.z = acc_ng.x *reference_v.x + acc_ng.y *reference_v.y + acc_ng.z *reference_v.z;
	

	// ������ٶ�������ģ
	norm_acc = my_sqrt(ax*ax + ay*ay + az*az);   


	if(ABS(ax)<4400 && ABS(ay)<4400 && ABS(az)<4400 )
	{	
		//�ѼӼƵ���ά����ת�ɵ�λ������
		ax = ax / norm_acc;//4096.0f;
		ay = ay / norm_acc;//4096.0f;
		az = az / norm_acc;//4096.0f; 
		
		if( 3800 < norm_acc && norm_acc < 4400 )
		{
			/* ��˵õ���� */
			ref.err_tmp.x = ay*reference_v.z - az*reference_v.y;
			ref.err_tmp.y = az*reference_v.x - ax*reference_v.z;
	    //ref.err_tmp.z = ax*reference_v.y - ay*reference_v.x;
			
			/* ����ͨ */
			ref_err_lpf_hz = REF_ERR_LPF_HZ *(6.28f *half_T);
			ref.err_lpf.x += ref_err_lpf_hz *( ref.err_tmp.x  - ref.err_lpf.x );
			ref.err_lpf.y += ref_err_lpf_hz *( ref.err_tmp.y  - ref.err_lpf.y );
	//			 ref.err_lpf.z += ref_err_lpf_hz *( ref.err_tmp.z  - ref.err_lpf.z );
			
			ref.err.x = ref.err_lpf.x;//
			ref.err.y = ref.err_lpf.y;//
//				ref.err.z = ref.err_lpf.z ;
		}
	}
	else
	{
		ref.err.x = 0; 
		ref.err.y = 0  ;
//		ref.err.z = 0 ;
	}
	/* ������ */
	ref.err_Int.x += ref.err.x *Ki *2 *half_T ;
	ref.err_Int.y += ref.err.y *Ki *2 *half_T ;
	ref.err_Int.z += ref.err.z *Ki *2 *half_T ;
	
	/* �����޷� */
	ref.err_Int.x = LIMIT(ref.err_Int.x, - IMU_INTEGRAL_LIM ,IMU_INTEGRAL_LIM );
	ref.err_Int.y = LIMIT(ref.err_Int.y, - IMU_INTEGRAL_LIM ,IMU_INTEGRAL_LIM );
	ref.err_Int.z = LIMIT(ref.err_Int.z, - IMU_INTEGRAL_LIM ,IMU_INTEGRAL_LIM );
	
	if( reference_v.z > 0.0f )
	{
		if( SysMode == SYS_PREPARESTATE )
		{
			yaw_correct = Kp *2.5f *To_180_degrees(yaw_mag - Yaw);
			//����ʱ�̣����پ���

		}
		else
		{
			yaw_correct = Kp *0.2f *To_180_degrees(yaw_mag - Yaw);
			//�Ѿ�������ֻ��Ҫ���پ�����
		}
// 		if( yaw_correct>360 || yaw_correct < -360  )
// 		{
// 			yaw_correct = 0;
// 			//���ƾ�����Χ+-360�����+-180��ȡֵ����
// 		}
	}
	else
	{
		yaw_correct = 0; //�Ƕȹ���ֹͣ������������Ŀ��ֵ���ܲ���ȷ
	}

	
	ref.g.x = (gx - reference_v.x *yaw_correct) *ANGLE_TO_RADIAN + ( Kp*(ref.err.x + ref.err_Int.x) ) ;     //IN RADIAN
	ref.g.y = (gy - reference_v.y *yaw_correct) *ANGLE_TO_RADIAN + ( Kp*(ref.err.y + ref.err_Int.y) ) ;		  //IN RADIAN
	ref.g.z = (gz - reference_v.z *yaw_correct) *ANGLE_TO_RADIAN;
	
	/* �ò���������PI����������ƫ */

	// integrate quaternion rate and normalise
	ref_q[0] = ref_q[0] +(-ref_q[1]*ref.g.x - ref_q[2]*ref.g.y - ref_q[3]*ref.g.z)*half_T;
	ref_q[1] = ref_q[1] + (ref_q[0]*ref.g.x + ref_q[2]*ref.g.z - ref_q[3]*ref.g.y)*half_T;
	ref_q[2] = ref_q[2] + (ref_q[0]*ref.g.y - ref_q[1]*ref.g.z + ref_q[3]*ref.g.x)*half_T;
	ref_q[3] = ref_q[3] + (ref_q[0]*ref.g.z + ref_q[1]*ref.g.y - ref_q[2]*ref.g.x)*half_T;  

	/* ��Ԫ����һ�� normalise quaternion */
	norm_q = my_sqrt(ref_q[0]*ref_q[0] + ref_q[1]*ref_q[1] + ref_q[2]*ref_q[2] + ref_q[3]*ref_q[3]);
	ref_q[0] = ref_q[0] / norm_q;
	ref_q[1] = ref_q[1] / norm_q;
	ref_q[2] = ref_q[2] / norm_q;
	ref_q[3] = ref_q[3] / norm_q;
	

	*pit = fast_atan2(2*(ref_q[0]*ref_q[1] + ref_q[2]*ref_q[3]),1 - 2*(ref_q[1]*ref_q[1] + ref_q[2]*ref_q[2])) *57.3f;
	*rol = asin(2*(ref_q[1]*ref_q[3] - ref_q[0]*ref_q[2])) *57.3f;
	*yaw = fast_atan2(2*(-ref_q[1]*ref_q[2] - ref_q[0]*ref_q[3]), 2*(ref_q[0]*ref_q[0] + ref_q[1]*ref_q[1]) - 1) *57.3f  ;// 
	FeedDog(DEVICE_INDEX_IMU);
	//*yaw = yaw_mag;

}

/**
  * @brief ���ٿ�������
  * @param None
  * @retval None
  */
float invSqrt(float number) 
{
    volatile long i;
    volatile float x, y;
    volatile const float f = 1.5F;

    x = number * 0.5F;
    y = number;
    i = * (( long * ) &y);
    i = 0x5f375a86 - ( i >> 1 );
    y = * (( float * ) &i);
    y = y * ( f - ( x * y * y ) );
    return y;
}

float q0q0, q0q1, q0q2, q0q3;
float q1q1, q1q2, q1q3;
float q2q2, q2q3;
float q3q3;
float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
float dq0 = 0.0f, dq1 = 0.0f, dq2 = 0.0f, dq3 = 0.0f;	/** quaternion of sensor frame relative to auxiliary frame */
float gyro_bias[3] = {0.0f, 0.0f, 0.0f}; /** bias estimation */


/**
  * @brief AHRS�㷨��ʼ��
  * @param None
  * @retval None
  */
void AHRSInit(float ax,float ay,float az,float mx,float my,float mz)
{
		
		float initialRoll, initialPitch;
    float cosRoll, sinRoll, cosPitch, sinPitch;
    float magX, magY;
    float initialHdg, cosHeading, sinHeading;

    initialRoll = atan2(-ay, -az);
    initialPitch = -atan2(ax, -az);

    cosRoll = cosf(initialRoll);
    sinRoll = sinf(initialRoll);
    cosPitch = cosf(initialPitch);
    sinPitch = sinf(initialPitch);

    magX = mx * cosPitch + my * sinRoll * sinPitch + mz * cosRoll * sinPitch;

    magY = my * cosRoll - mz * sinRoll;

    initialHdg = atan2f(-magY, magX);

    cosRoll = cosf(initialRoll * 0.5f);
    sinRoll = sinf(initialRoll * 0.5f);

    cosPitch = cosf(initialPitch * 0.5f);
    sinPitch = sinf(initialPitch * 0.5f);

    cosHeading = cosf(initialHdg * 0.5f);
    sinHeading = sinf(initialHdg * 0.5f);

    q0 = cosRoll * cosPitch * cosHeading + sinRoll * sinPitch * sinHeading;
    q1 = sinRoll * cosPitch * cosHeading - cosRoll * sinPitch * sinHeading;
    q2 = cosRoll * sinPitch * cosHeading + sinRoll * cosPitch * sinHeading;
    q3 = cosRoll * cosPitch * sinHeading - sinRoll * sinPitch * cosHeading;

    // auxillary variables to reduce number of repeated operations, for 1st pass
    q0q0 = q0 * q0;
    q0q1 = q0 * q1;
    q0q2 = q0 * q2;
    q0q3 = q0 * q3;
    q1q1 = q1 * q1;
    q1q2 = q1 * q2;
    q1q3 = q1 * q3;
    q2q2 = q2 * q2;
    q2q3 = q2 * q3;
    q3q3 = q3 * q3;
}

/**
  * @brief ��̬����
  * @param None
  * @retval None
  * @details copy from PX4
  */
void AHRSUpdate(float dt,float ax,float ay,float az,float gx,float gy,float gz,float mx,float my,float mz,float *pit,float *rol,float *yaw,float twoKp,float twoKi)
{
	float recipNorm;
	float halfex = 0.0f, halfey = 0.0f, halfez = 0.0f;
	static u8 StartFlag=0;
	gx*=ANGLE_TO_RADIAN;
	gy*=ANGLE_TO_RADIAN;
	gz*=ANGLE_TO_RADIAN;
	//��ʼʱ�̳�ʼ������ʼ��20������Ϊһ��ʼ�����Ƶ����ݿ��ܻ�û���£��������1ms����һ�Σ�������5ms����һ��
	if(StartFlag<20) {
		AHRSInit(ax,ay,az,mx,my,mz);
		StartFlag++;
	}
        	
	//������������������ͼ��������
	if(!((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f))) {
		float hx, hy, hz, bx, bz;
		float halfwx, halfwy, halfwz;
			//��һ��
    	recipNorm = invSqrt(mx * mx + my * my + mz * mz);
    	mx *= recipNorm;
    	my *= recipNorm;
    	mz *= recipNorm;
    
    	/*
			�����Ҫ��������
		|mx|
		|my|  �ǻ�������ϵbϵ�´�������
		|mz|
		��bϵ��ο�ϵRϵ�ı任����ΪC1
				|	q0^2+q1^2-q2^2-q3^2			2(q1q2-q0q3)    					2(q1q3+q0q2)				|
				|																																				|
		C1=	|	2(q1q2+q0q3)						q0^2-q1^2+q2^2-q3^2				2(q0q1-q2q3)				|
				|																																				|
				|	2(q1q3-q0q2)						2(q2q3+q0q1)							q0^2-q1^2-q2^2+q3^2	|				
		��Ԫ���Ļ���������q0^2+q1^2+q2^2+q3^2=1
		
		C1���Լ�Ϊ
		
				|	1-2q2^2-2q3^2						2(q1q2-q0q3)    					2(q1q3+q0q2)				|
				|																																				|
		C1=	|	2(q1q2+q0q3)						1-2q1^2-2q3^2							2(q0q1-q2q3)				|
				|																																				|
				|	2(q1q3-q0q2)						2(q2q3+q0q1)							1-2q1^2-2q2^2				|
				
		
		������֤C1���Ǹ�����������ô���Եõ�C1�������ΪC2
		
				|	1-2q2^2-2q3^2						2(q1q2+q0q3)    					2(q1q3-q0q2)				|
				|																																				|
		C2=	|	2(q1q2-q0q3)						1-2q1^2-2q3^2							2(q0q1+q2q3)				|
				|																																				|
				|	2(q1q3+q0q2)						2(q2q3-q0q1)							1-2q1^2-2q2^2				|
				
		C2���ǴӲο�����ϵR�任����������ϵbϵ�ľ���
		��������
		|hx|
		|hy|�ǲο�����ϵ�µĴ�������
		|hz|
		
					|	1-2q2^2-2q3^2						2(q1q2-q0q3)    					2(q1q3+q0q2)		|
		|hx|	|																																		|	|mx|
		|hy|=	|	2(q1q2+q0q3)						1-2q1^2-2q3^2							2(q0q1-q2q3)		|	|my|
		|hz|	|																																		|	|mz|
					|	2(q1q3-q0q2)						2(q2q3+q0q1)							1-2q1^2-2q2^2		|

				
		|hx|	|(1-2q2^2-2q3^2)*mx+2(q1q2-q0q3)*my+2(q1q3+q0q2)*mz|
		|hy|=	|2(q1q2+q0q3)*mx+(1-2q1^2-2q3^2)*my+2(q0q1-q2q3)*mz|
		|hz|	|2(q1q3-q0q2)*mx+2(q2q3+q0q1)*my+(1-2q1^2-2q2^2)*mz|
		**/
    	hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
    	hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
			hz = 2.0f * mx * (q1q3 - q0q2) + 2.0f * my * (q2q3 + q0q1) + 2.0f * mz * (0.5f - q1q1 - q2q2);
    /*
		����ų���ˮƽ����ķ����ķ���Ϊx����ֱ����Ϊy��
		�ɰ�
		|bx|	|sqrt(hx*hx+hy*hy)|
		|by|=	|				0					|
		|bz|	|				hz				|

		**/
			
			bx = sqrt(hx * hx + hy * hy);
    	bz = hz;
    /*
					|bx|
		�ٰ�	|by|	ת����������ϵbϵ��
					|bz|
		
		|wx|			|bx|
		|wy| = C2 |by|
		|wz|			|bz|
		
		
						|	1-2q2^2-2q3^2						2(q1q2+q0q3)    					2(q1q3-q0q2)	|
		|wx|		|																																	|		|bx|
		|wy|	=	|	2(q1q2-q0q3)						1-2q1^2-2q3^2							2(q0q1+q2q3)	|		|by|
		|wz|		|																																	|		|bz|
						|	2(q1q3+q0q2)						2(q2q3-q0q1)							1-2q1^2-2q2^2	|
	
		|wx|		|(1-2q2^2-2q3^2)*bx+2(q1q2+q0q3)*by+2(q1q3-q0q2)*bz|
		|wy|	=	|2(q1q2-q0q3)*bx+(1-2q1^2-2q3^2)*by+2(q0q1+q2q3)*bz|
		|wz|		|2(q1q3+q0q2)*bx+2(q2q3-q0q1)*by+(1-2q1^2+2q2^2)*bz|
		*/
    	// Estimated direction of magnetic field
    	halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
    	halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
    	halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);
    /*
			|wx|		|mx|
			|wy| x 	|my|
			|wz|		|mz|
			��˽Ƕ����Խ�󣬲�˽��Խ��
		**/
    	// Error is sum of cross product between estimated direction and measured direction of field vectors
    	halfex +=(my * halfwz - mz * halfwy);
    	halfey +=(mz * halfwx - mx * halfwz);
    	halfez += (mx * halfwy - my * halfwx);
	}

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
		float halfvx, halfvy, halfvz;
	
		// ��һ��
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;

		/*
		���ﲻ����������Ǳ�һ���Ȱѻ�������ϵת����������ϵ��ת���ش�����ϵ
		�������ٶ�Ϊ
		|0|
		|0|
		|1|
		��ô
			
		|vx|			|0|
		|vy| = C2 |0|
		|vz|			|0|
		
		
						|	1-2q2^2-2q3^2						2(q1q2+q0q3)    					2(q1q3-q0q2)	|
		|vx|		|																																	|		|0|
		|vy|	=	|	2(q1q2-q0q3)						1-2q1^2-2q3^2							2(q0q1+q2q3)	|		|0|
		|vz|		|																																	|		|1|
						|	2(q1q3+q0q2)						2(q2q3-q0q1)							1-2q1^2-2q2^2	|
	
		|vx|		|	2(q1q3-q0q2)	|
		|vy|	=	|	2(q0q1+q2q3)	|
		|vz|		|(1-2q1^2+2q2^2)|

		**/
		halfvx = q1q3 - q0q2;
		halfvy = q0q1 + q2q3;
		halfvz = q0q0 - 0.5f + q3q3;
	
		// ��������
		halfex += ay * halfvz - az * halfvy;
		halfey += az * halfvx - ax * halfvz;
		halfez += ax * halfvy - ay * halfvx;
	}

	// ��������PI����
	if(halfex != 0.0f && halfey != 0.0f && halfez != 0.0f) {
		// Compute and apply integral feedback if enabled
		if(twoKi > 0.0f) {
			gyro_bias[0] += twoKi * halfex * dt;	// integral error scaled by Ki
			gyro_bias[1] += twoKi * halfey * dt;
			gyro_bias[2] += twoKi * halfez * dt;
			
			// apply integral feedback
			gx += gyro_bias[0];
			gy += gyro_bias[1];
			gz += gyro_bias[2];
		}
		else {
			gyro_bias[0] = 0.0f;	// prevent integral windup
			gyro_bias[1] = 0.0f;
			gyro_bias[2] = 0.0f;
		}

		// Apply proportional feedback
		gx += twoKp * halfex;
		gy += twoKp * halfey;
		gz += twoKp * halfez;
	}
	
	//! Integrate rate of change of quaternion
#if 0
	gx *= (0.5f * dt);		// pre-multiply common factors
	gy *= (0.5f * dt);
	gz *= (0.5f * dt);
#endif 
	/**
		��Ԫ��΢�ַ�������
	
	|dq0/dt|				|q0	-q1	-q2	-q3	|	|0 |
	|dq1/dt|		1		|q1	q0	-q3	q2	|	|gx|
	|dq2/dt|=		- *	|q2	q3	q0	-q1	|	|gy|
	|dq3/dt|		2		|q3	-q2	q1	q0	|	|gz|
	
	|dq0/dt|			|-q1*gx-q2*gy-q3*gz	|	
	|dq1/dt|	1		|q0*gx-q3*gy+q2*gz	|
	|dq2/dt|=	��	*	|q3*gx+q0*gy-q1*gz	|	
	|dq3/dt|	2		|-q2*gx+q1*gy+q0*gz	|	

	*/
	
	dq0 = 0.5f*(-q1 * gx - q2 * gy - q3 * gz);
	dq1 = 0.5f*(q0 * gx + q2 * gz - q3 * gy);
	dq2 = 0.5f*(q0 * gy - q1 * gz + q3 * gx);
	dq3 = 0.5f*(q0 * gz + q1 * gy - q2 * gx); 
	/*
		һ���������
	q0 = q0 + dq0/dt * T
	q1 = q1 + dq1/dt * T
	q2 = q2 + dq2/dt * T
	q3 = q3 + dq3/dt * T
	**/
	q0 += dt*dq0;
	q1 += dt*dq1;
	q2 += dt*dq2;
	q3 += dt*dq3;
	
	// ��һ��
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;

	// ����һЩ����
    q0q0 = q0 * q0;
    q0q1 = q0 * q1;
    q0q2 = q0 * q2;
    q0q3 = q0 * q3;
    q1q1 = q1 * q1;
    q1q2 = q1 * q2;
   	q1q3 = q1 * q3;
    q2q2 = q2 * q2;
    q2q3 = q2 * q3;
    q3q3 = q3 * q3;   
/**
		��Ϊ�Ƕ�
**/
		*rol = atan2f(2.f * (q2*q3 + q0*q1), q0q0 - q1q1 - q2q2 + q3q3)*57.3f;	//! Roll
		*pit = -asinf(2.f * (q1*q3 - q0*q2))*57.3f;	//! Pitch
		*yaw = atan2f(2.f * (q1*q2 + q0*q3), q0q0 + q1q1 - q2q2 - q3q3)*57.3f;		//! Yaw
		FeedDog(DEVICE_INDEX_IMU);
}

