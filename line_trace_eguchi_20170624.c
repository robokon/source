#include "line_trace.h"
#include "Distance.h"

#define DISTANCE_NOTIFY (500.0)

int grade_test_cnt = 0;     /*  ���J�E���g */
int grade_test_flg = 0;     /*  huragu */
int grade_test_touritu = 0;     /*  ���J�E���g */

//*****************************************************************************
// �֐��� : line_tarce_main
// ���� : 
// �Ԃ�l : �Ȃ�
// �T�v : 
//       
//*****************************************************************************

signed char forward;      /* �O��i���� */
signed char turn;         /* ���񖽗� */
signed char pwm_L, pwm_R; /* ���E���[�^PWM�o�� */

void line_tarce_main()
{
    int32_t motor_ang_l, motor_ang_r;
    int gyro, volt;

    if (ev3_button_is_pressed(BACK_BUTTON)) return;

    tail_control(TAIL_ANGLE_DRIVE); /* �o�����X���s�p�p�x�ɐ��� */

    if (sonar_alert() == 1) /* ��Q�����m */
    {
        forward = turn = 0; /* ��Q�������m�������~ */
    }
    else
    {
        forward = 30; /* �O�i���� */
        if (ev3_color_sensor_get_reflect(color_sensor) >= (LIGHT_WHITE + LIGHT_BLACK)/2)
        {
            turn =  20; /* �����񖽗� */
        }
        else
        {
            turn = -20; /* �E���񖽗� */
        }
    }
    if(grade_test_flg == 1)
    {
        forward = turn = 0;
    }
    

    /* �|���U�q����API �ɓn���p�����[�^���擾���� */
    motor_ang_l = ev3_motor_get_counts(left_motor);
    motor_ang_r = ev3_motor_get_counts(right_motor);
    gyro = ev3_gyro_sensor_get_rate(gyro_sensor);
    volt = ev3_battery_voltage_mV();

    /* �|���U�q����API���Ăяo���A�|�����s���邽�߂� */
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

    if(grade_test_flg == 1)
    {
        grade_test_touritu++;
        if(grade_test_touritu >= 1000)
        {
             ev3_motor_stop(right_motor, true);
             ev3_motor_stop(left_motor, true);
            
            if(grade_test_touritu >= 1250)
            {
                ev3_speaker_set_volume(100); 
                ev3_speaker_play_tone(NOTE_C4, 100);
             tail_control(TAIL_ANGLE_STAND_UP); /* �|��������폜 �K�������� */
            }
        }
    }

    /* EV3�ł̓��[�^�[��~���̃u���[�L�ݒ肪���O�ɂł��Ȃ����� */
    /* �o��0���ɁA���̓s�x�ݒ肷�� */
    if (pwm_L == 0)
    {
         ev3_motor_stop(left_motor, true);
    }
    else
    {
        ev3_motor_set_power(left_motor, (int)pwm_L);
    }
    
    if (pwm_R == 0)
    {
         ev3_motor_stop(right_motor, true);
    }
    else
    {
        ev3_motor_set_power(right_motor, (int)pwm_R);
    }
	
	Distance_update(); /* �ړ��������Z */
	
	if( Distance_getDistance() > DISTANCE_NOTIFY )
	{
		/* DISTANCE_NOTIFY�ȏ�i�񂾂特���o�� */
		ev3_speaker_set_volume(100); 
		ev3_speaker_play_tone(NOTE_C4, 100);
	    grade_test_cnt++;
	    if(grade_test_cnt >= 1)
	    {
	        grade_test_flg  =1;


	    }
		/* �����v���ϐ������� */
		Distance_init();
	}
	
}

/* end of file */
