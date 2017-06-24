#include "stair.h"

signed char forward;      /* �O��i���� */
signed char turn;         /* ���񖽗� */
signed char pwm_L, pwm_R; /* ���E���[�^PWM�o�� */


//*****************************************************************************
// �֐��� : stair_main
// ���� : 
// �Ԃ�l : �Ȃ�
// �T�v : 
//       
//*****************************************************************************
void stair_main()
{
	int32_t LEFT_info;
	int32_t RIGHT_info;
	
	/* ���[�^�̃t���p���[�̃p�[�Z���g�l��ݒ� */
	pwm_L = 30;
	pwm_R = 30;
	
	/* �K�������� */
	tail_control(TAIL_ANGLE_STAND_UP);
	
    /* EV3�ł̓��[�^�[��~���̃u���[�L�ݒ肪���O�ɂł��Ȃ����� */
    /* �o��0���ɁA���̓s�x�ݒ肷�� */
    if (pwm_L == 0 || RIGHT_info >= 1000 )
    {
         ev3_motor_stop(left_motor, true);
    }
    else
    {
    	/* �p�[�Z���g�l��*/
        ev3_motor_set_power(left_motor, -1 * (int)pwm_L);
    	
    	/* �����[�^�̊e�ʒu���擾���� */
    	LEFT_info += ev3_motor_get_counts(left_motor);
    }
    
    if (pwm_R == 0 || RIGHT_info >= 1000 )
    {
         ev3_motor_stop(right_motor, true);
    }
    else
    {
        ev3_motor_set_power(right_motor, (int)pwm_R);
    	
    	/* �E���[�^�̊p�ʒu���擾���� */
    	RIGHT_info += ev3_motor_get_counts(right_motor);
    }
	
	if( RIGHT_info >= 1000 )
	{
		ev3_motor_stop(left_motor, true);
		ev3_motor_stop(right_motor, true);
	}
}

/* end of file */
