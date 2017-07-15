#include "stair.h"

signed char forward;      /* �O��i���� */
signed char turn;         /* ���񖽗� */
signed char pwm_L , pwm_R; /* ���E���[�^PWM�o�� */
static int RIGHT_info = 0;

//*****************************************************************************
// �֐��� : stair_main
// ���� : 
// �Ԃ�l : �Ȃ�
// �T�v : 
//       
//*****************************************************************************
void stair_main()
{
	int32_t motor_ang_l, motor_ang_r;
    int gyro, volt;

	
    /* EV3�ł̓��[�^�[��~���̃u���[�L�ݒ肪���O�ɂł��Ȃ����� */
    /* �o��0���ɁA���̓s�x�ݒ肷�� */
//    if (RIGHT_info >= 1000 )
    if (RIGHT_info >= 50000 )
    {
         tail_control(TAIL_ANGLE_DRIVE); /* �o�����X���s�p�p�x�ɐ��� */
         forward = turn = 0; /* �O�r�^�������͈�U�X�g�b�v */
    	
    	/* �|���U�q����API �ɓn���p�����[�^���擾���� */
        motor_ang_l = ev3_motor_get_counts(left_motor);
        motor_ang_r = ev3_motor_get_counts(right_motor);
        gyro = ev3_gyro_sensor_get_rate(gyro_sensor);
        volt = ev3_battery_voltage_mV();

    /* �|���U�q����API���Ăяo��*/
    /* ���E���[�^�o�͒l�𓾂� */
    balance_control(
        (float)forward,
        (float)turn,
        (float)gyro,
        (float)GYRO_OFFSET,
        (float)motor_ang_l,
        (float)motor_ang_r,
        (float)volt,
        (signed char*)&pwm_L,
        (signed char*)&pwm_R);

         ev3_motor_set_power(left_motor, (int)pwm_L);
         ev3_motor_set_power(right_motor, (int)pwm_R);
    	
    }
    else
    {
    	/* �K�������� */
		tail_control(TAIL_ANGLE_STAND_UP);

    	/* ���[�^�̃t���p���[�̃p�[�Z���g�l��ݒ� */
	    pwm_L = 30;
	    pwm_R = 30;
	
        ev3_motor_set_power(left_motor, -1 * (int)pwm_L);
        ev3_motor_set_power(right_motor, (int)pwm_R);    	
    	/* �E���[�^�̊p�ʒu���擾���� */
    	RIGHT_info += ev3_motor_get_counts(right_motor);

    }
    
}

/* end of file */
