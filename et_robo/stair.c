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
	int counter = 0; /* 4ms���Ƃ̃J�E���^�l */
	int loop = 1;
	int False = 0;
	int gyro_str = 0;
	int stair_num = 0;
	int limit_count = 25000; /* 100 s */
	#define GYRO_DIV                200
	
	
    tail_control(TAIL_ANGLE_STAND_UP); /* �|��������폜 �K�������� */
	
	while(loop)
	{
		/* �W���C���p���x�ɂĊK�i�����m���� */
		if((counter % 10) == 0)
		{
			/* 40 ms���Ƃ�gyro_str�ɃW���C���p���x�Z���T�[�l���i�[ (4 ms�����Ɖ���) */
			gyro_str = ev3_gyro_sensor_get_rate(gyro_sensor);
		}

		/* 40 ms�O�̃Z���T�[�l�ƍ��̃Z���T�[�l�̍������K��l���傫���ꍇ�@�K�i�ɂԂ������Ɣ��f���� */
		if((gyro_str - ev3_gyro_sensor_get_rate(gyro_sensor)) > GYRO_DIV ||
		(gyro_str - ev3_gyro_sensor_get_rate(gyro_sensor)) < (GYRO_DIV * (-1)) )
	    {
			counter++;
            /* �K�i�����m */
			stair_num++;
			
			/* �K�i���m�����Ŏ��� */
			ev3_speaker_set_volume(100); 
		    ev3_speaker_play_tone(NOTE_C4, 100);
			
			/* 10msec�E�F�C�g */
			tslp_tsk(10);
			
			/* ������ƃo�b�N */
			ev3_motor_rotate (left_motor, -360, 30, false );
			ev3_motor_rotate (right_motor, -360, 30, true );
			
			/* �������Đi�� */
			ev3_motor_rotate (left_motor, 3600, 90, false );
			ev3_motor_rotate (right_motor, 3600, 90, true );
			
			/* 40msec�E�F�C�g */
			tslp_tsk(40);
			
		    /* �K�i��������Ƃ����Ŏ��� */
		    ev3_speaker_play_tone(NOTE_E6, 100);
			continue;
		  
		    if(stair_num == 2)
			{
				/* ���ڂ̊K�i���m��������loop���甲���� */
				loop = False;
			}
		}
		else
		{
			counter++;
			/* �K�i�����m����܂őO�i���� */
			ev3_motor_rotate (left_motor, 360, 30, false );
			ev3_motor_rotate (right_motor, 360, 30, false );
			
			/* ���S���u ��莞�Ԍo�߂��Ă�2��ڂ̊K�i���m�ł��Ȃ����loop����E�o */
			if(counter > limit_count)
			{
				loop = False;
			}
			continue;
		}

	}
	
}

/* end of file */
