/**
 ******************************************************************************
 ** �t�@�C���� : app.c
 **
 ** �T�v : 2�֓|���U�q���C���g���[�X���{�b�g��TOPPERS/HRP2�pC�T���v���v���O����
 **
 ** ���L : sample_c4 (sample_c3��Bluetooth�ʐM�����[�g�X�^�[�g�@�\��ǉ�)
 ******************************************************************************
 **/

#include "ev3api.h"
#include "app.h"
#include "balancer.h"
#include "common.h"

#include "line_trace.h"
#include "Distance.h"

#if defined(BUILD_MODULE)
#include "module_cfg.h"
#else
#include "kernel_cfg.h"
#endif

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

int bt_cmd = 0;     /* Bluetooth�R�}���h 1:�����[�g�X�^�[�g */
FILE *bt = NULL;     /* Bluetooth�t�@�C���n���h�� */

/* �e������� */
STATUS main_status = STAT_UNKNOWN;

/* ���C���^�X�N */
void main_task(intptr_t unused)
{

    /* LCD��ʕ\�� */
    ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);
    ev3_lcd_draw_string("EV3way-ET sample_c4", 0, CALIB_FONT_HEIGHT*1);

    /* �Z���T�[���̓|�[�g�̐ݒ� */
    ev3_sensor_config(sonar_sensor, ULTRASONIC_SENSOR);
    ev3_sensor_config(color_sensor, COLOR_SENSOR);
    ev3_color_sensor_get_reflect(color_sensor); /* ���˗����[�h */
    ev3_sensor_config(touch_sensor, TOUCH_SENSOR);
    ev3_sensor_config(gyro_sensor, GYRO_SENSOR);
    /* ���[�^�[�o�̓|�[�g�̐ݒ� */
    ev3_motor_config(left_motor, LARGE_MOTOR);
    ev3_motor_config(right_motor, LARGE_MOTOR);
    ev3_motor_config(tail_motor, LARGE_MOTOR);
    ev3_motor_reset_counts(tail_motor);

    /* Open Bluetooth file */
    bt = ev3_serial_open_file(EV3_SERIAL_BT);
    assert(bt != NULL);

    /* Bluetooth�ʐM�^�X�N�̋N�� */
    act_tsk(BT_TASK);

    ev3_led_set_color(LED_ORANGE); /* �����������ʒm */

	Distance_init(); /* �����v���ϐ������� */
	
    /* �X�^�[�g�ҋ@ */
    while(1)
    {
        tail_control(TAIL_ANGLE_STAND_UP); /* ���S��~�p�p�x�ɐ��� */

        if (bt_cmd == 1)
        {
            break; /* �����[�g�X�^�[�g */
        }

        if (ev3_touch_sensor_is_pressed(touch_sensor) == 1)
        {
            break; /* �^�b�`�Z���T�������ꂽ */
        }

        tslp_tsk(10); /* 10msec�E�F�C�g */
    }

    /* ���s���[�^�[�G���R�[�_�[���Z�b�g */
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);

    /* �W���C���Z���T�[���Z�b�g */
    ev3_gyro_sensor_reset(gyro_sensor);
    balance_init(); /* �|���U�qAPI������ */

    ev3_led_set_color(LED_GREEN); /* �X�^�[�g�ʒm */
    
    /* �X�^�[�g�ʒm��A�ʏ�̃��C���g���[�X�Ɉڍs����悤�ɐݒ� */
    main_status = STAT_NORMAL; 

    /**
    * Main loop for the self-balance control algorithm
    */
    while(1)
    {
        switch (main_status) {
            /* �ʏ퐧�䒆 */
            case STAT_NORMAL:
                /* �ʏ�̃��C���g���[�X���� */
                line_tarce_main();
                break;

            /* �K�i���䒆 */
            case STAT_STAIR:
                /* T.B.D */
                break;

            /* ���b�N�A�b�v�Q�[�g���䒆 */
            case STAT_LOOK_UP_GATE:
                /* T.B.D */
                break;

            /* �K���[�W���䒆 */
            case STAT_GAREGE:
                /* T.B.D */
                break;

            /* ���̑� */
            default:
                /* T.B.D */
                break;
        }
        
        if (ev3_button_is_pressed(BACK_BUTTON)) break;;        
        tslp_tsk(4); /* 4msec�����N�� */
    }
    ev3_motor_stop(left_motor, false);
    ev3_motor_stop(right_motor, false);

    ter_tsk(BT_TASK);
    fclose(bt);

    ext_tsk();
}

//*****************************************************************************
// �֐��� : sonar_alert
// ���� : ����
// �Ԃ�l : 1(��Q������)/0(��Q������)
// �T�v : �����g�Z���T�ɂ���Q�����m
//*****************************************************************************
int sonar_alert(void)
{
    static unsigned int counter = 0;
    static int alert = 0;

    signed int distance;

    if (++counter == 40/4) /* ��40msec�������ɏ�Q�����m  */
    {
        /*
         * �����g�Z���T�ɂ�鋗����������́A�����g�̌��������Ɉˑ����܂��B
         * NXT�̏ꍇ�́A40msec�������x���o����̍ŒZ��������ł��B
         * EV3�̏ꍇ�́A�v�m�F
         */
        distance = ev3_ultrasonic_sensor_get_distance(sonar_sensor);
        if ((distance <= SONAR_ALERT_DISTANCE) && (distance >= 0))
        {
            alert = 1; /* ��Q�������m */
        }
        else
        {
            alert = 0; /* ��Q������ */
        }
        counter = 0;
    }

    return alert;
}

//*****************************************************************************
// �֐��� : tail_control
// ���� : angle (���[�^�ڕW�p�x[�x])
// �Ԃ�l : ����
// �T�v : ���s�̊��S��~�p���[�^�̊p�x����
//*****************************************************************************
void tail_control(signed int angle)
{
    float pwm = (float)(angle - ev3_motor_get_counts(tail_motor))*P_GAIN; /* ��ᐧ�� */
    /* PWM�o�͖O�a���� */
    if (pwm > PWM_ABS_MAX)
    {
        pwm = PWM_ABS_MAX;
    }
    else if (pwm < -PWM_ABS_MAX)
    {
        pwm = -PWM_ABS_MAX;
    }

    if (pwm == 0)
    {
        ev3_motor_stop(tail_motor, true);
    }
    else
    {
        ev3_motor_set_power(tail_motor, (signed char)pwm);
    }
}

//*****************************************************************************
// �֐��� : bt_task
// ���� : unused
// �Ԃ�l : �Ȃ�
// �T�v : Bluetooth�ʐM�ɂ�郊���[�g�X�^�[�g�B Tera Term�Ȃǂ̃^�[�~�i���\�t�g����A
//       ASCII�R�[�h��1�𑗐M����ƁA�����[�g�X�^�[�g����B
//*****************************************************************************
void bt_task(intptr_t unused)
{
    while(1)
    {
        uint8_t c = fgetc(bt); /* ��M */
        switch(c)
        {
        case '1':
            bt_cmd = 1;
            break;
        default:
            break;
        }
        fputc(c, bt); /* �G�R�[�o�b�N */
    }
}

/* end of file */