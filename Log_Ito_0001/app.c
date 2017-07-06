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

/**
 * �Z���T�[�A���[�^�[�̐ڑ����`���܂�
 */
static const sensor_port_t
    touch_sensor    = EV3_PORT_1,
    sonar_sensor    = EV3_PORT_2,
    color_sensor    = EV3_PORT_3,
    gyro_sensor     = EV3_PORT_4;

static const motor_port_t
    left_motor      = EV3_PORT_C,
    right_motor     = EV3_PORT_B,
    tail_motor      = EV3_PORT_A;

static int      bt_cmd = 0;     /* Bluetooth�R�}���h 1:�����[�g�X�^�[�g */
static FILE     *bt = NULL;     /* Bluetooth�t�@�C���n���h�� */

static int LIGHT_WHITE=0;         /* ���F�̌��Z���T�l */
static int LIGHT_BLACK=100;          /* ���F�̌��Z���T�l */

/* ���L�̃}�N���͌�/���ɍ��킹�ĕύX����K�v������܂� */
/* sample_c1�}�N�� */
#define GYRO_OFFSET  0          /* �W���C���Z���T�I�t�Z�b�g�l(�p���x0[deg/sec]��) */

/* sample_c2�}�N�� */
#define SONAR_ALERT_DISTANCE 30 /* �����g�Z���T�ɂ���Q�����m����[cm] */
/* sample_c3�}�N�� */
#define TAIL_ANGLE_STAND_UP  93 /* ���S��~���̊p�x[�x] */
#define TAIL_ANGLE_DRIVE      3 /* �o�����X���s���̊p�x[�x] */
#define P_GAIN             2.5F /* ���S��~�p���[�^������W�� */
#define PWM_ABS_MAX          60 /* ���S��~�p���[�^����PWM��΍ő�l */
/* sample_c4�}�N�� */
//#define DEVICE_NAME     "ET0"  /* Bluetooth�� hrp2/target/ev3.h BLUETOOTH_LOCAL_NAME�Őݒ� */
//#define PASS_KEY        "1234" /* �p�X�L�[    hrp2/target/ev3.h BLUETOOTH_PIN_CODE�Őݒ� */
#define CMD_START         '1'    /* �����[�g�X�^�[�g�R�}���h */

/* LCD�t�H���g�T�C�Y */
#define CALIB_FONT (EV3_FONT_SMALL)
#define CALIB_FONT_WIDTH (6/*TODO: magic number*/)
#define CALIB_FONT_HEIGHT (8/*TODO: magic number*/)


#if (LOG_TASK == TASK_ON)
/* Log �̍ő�� */
#define  LOG_MAX   1000

/* Log file�̖��O */
#define  LOG_FILE_NAME  "Log_yymmdd.csv"

/* Log�p�̍\���� */
/*  ���ˌ��Z���T�[�l
   �W���C���Z���T�p�ʒu
   �W���C���Z���T�p���x */
 typedef struct{
    uint8_t Reflect;
    int16_t Gyro_angle;
    int16_t Gyro_rate;  
    int16_t Turn;
 	int16_t P;
 	int16_t D;
}Logger;

/* Log�񐔂̊i�[�ϐ� */
int LogNum = 0;

/* Log�i�[�z�� */
Logger gst_Log_str[LOG_MAX]; /* (1s == 250) */
#endif

/* �֐��v���g�^�C�v�錾 */
static int sonar_alert(void);
static void tail_control(signed int angle);


#if (LOG_TASK == TASK_ON)
void log_str(uint8_t reflect, int16_t rate, int16_t turn,int16_t p,int16_t d);
void log_commit(void);
#endif


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

    /*���F�̌��Z���T�l�擾*/
    while(1)
    {
        if (ev3_touch_sensor_is_pressed(touch_sensor) == 1)
        {
            LIGHT_WHITE = ev3_color_sensor_get_reflect(color_sensor);
            log_str(LIGHT_WHITE,0,0,0,0);
            break; /* �^�b�`�Z���T�������ꂽ */
        }
        tslp_tsk(10); /* 10msec�E�F�C�g */
    }
     tslp_tsk(1000); /* 10msec�E�F�C�g */
    /*���F�̌��Z���T�l*/
    while(1)
    {
        if (ev3_touch_sensor_is_pressed(touch_sensor) == 1)
        {
            LIGHT_BLACK = ev3_color_sensor_get_reflect(color_sensor);
            log_str(LIGHT_BLACK,0,0,0,0);
            break; /* �^�b�`�Z���T�������ꂽ */
        }
        tslp_tsk(10); /* 10msec�E�F�C�g */
    }
    tslp_tsk(1000); /* 10msec�E�F�C�g */

    
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

    /**
    * Main loop for the self-balance control algorithm
    */
    // �����n���h���J�n
    ev3_sta_cyc(TEST_EV3_CYC1);
    //ev3_sta_cyc(TEST_EV3_CYC2);

	// �o�b�N�{�^�����������܂ő҂�
	slp_tsk();
    // �����n���h����~
    ev3_stp_cyc(TEST_EV3_CYC1); 
    //ev3_stp_cyc(TEST_EV3_CYC2); 
    
    ev3_motor_stop(left_motor, false);
    ev3_motor_stop(right_motor, false);

	log_commit();
    ter_tsk(BT_TASK);
    fclose(bt);

    ext_tsk();
}

//*****************************************************************************
// �֐��� : test_ev3_cys1
// ���� : ����
// �Ԃ�l :
// �T�v : �����^�X�N�i���C���g���[�X�j
//*****************************************************************************
void test_ev3_cys1(intptr_t idx) 
{
    act_tsk(LINE_TRACE_TASK);
}

//*****************************************************************************
// �֐��� : test_ev3_cys2
// ���� : ����
// �Ԃ�l :
// �T�v : �����^�X�N�i���O�j
//*****************************************************************************
void test_ev3_cys2(intptr_t idx) 
{
    act_tsk(LOG_CREATE_TASK);
}

//*****************************************************************************
// �֐��� : sonar_alert
// ���� : ����
// �Ԃ�l : 1(��Q������)/0(��Q������)
// �T�v : �����g�Z���T�ɂ���Q�����m
//*****************************************************************************
void log_create_task(intptr_t idx) 
{
	//log_str();
}

//*****************************************************************************
// �֐��� : sonar_alert
// ���� : ����
// �Ԃ�l : 1(��Q������)/0(��Q������)
// �T�v : �����g�Z���T�ɂ���Q�����m
//*****************************************************************************
static int sonar_alert(void)
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
static void tail_control(signed int angle)
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


#if (LOG_TASK == TASK_ON)
//*****************************************************************************
// �֐��� : log_str
// ���� : �Ȃ�
// �Ԃ�l : �Ȃ�
// �T�v : �O���[�o���z�� gst_Log_str�Ɍ��݂̃Z���T�[�l���i�[
//
//*****************************************************************************
void log_str(uint8_t reflect, int16_t rate, int16_t turn, int16_t p, int16_t d)
{
	if(LogNum < LOG_MAX)
	{
	    gst_Log_str[LogNum].Reflect = reflect;
	    //gst_Log_str[LogNum].Gyro_angle = ev3_gyro_sensor_get_angle(gyro_sensor);
	    gst_Log_str[LogNum].Gyro_rate = rate;
	    gst_Log_str[LogNum].Turn = turn;
	    gst_Log_str[LogNum].P = p;
		gst_Log_str[LogNum].D = d;
		
	    LogNum++;	
	}
}

//*****************************************************************************
// �֐��� : log_commit
// ���� : �Ȃ�
// �Ԃ�l : �Ȃ�
// �T�v : �O���[�o���z�� gst_Log_str�Ɋi�[����Ă���f�[�^���t�@�C���o�͂���
//
//*****************************************************************************
void log_commit(void)
{
    FILE *fp; /* �t�@�C���|�C���^ */
	int  i;   /* �C���N�������g */

    /* Log�t�@�C���쐬 */
	fp=fopen(LOG_FILE_NAME,"a");
	/* ��^�C�g���}�� */
	fprintf(fp,"���ˌ��Z���T�[, �W���C���p�x, �W���C���Z���T�p���x,�^�[��,P,D�@\n");
	
	/* Log�̏o�� */
	for(i = 0 ; i < LOG_MAX; i++)
	{
		fprintf(fp,"%d,%d,%d,%d,%d,%d\n",gst_Log_str[i].Reflect, gst_Log_str[i].Gyro_angle, gst_Log_str[i].Gyro_rate, gst_Log_str[i].Turn, gst_Log_str[i].P, gst_Log_str[i].D);
	}
	
	fclose(fp);
}

#endif
//*****************************************************************************
// �֐��� : line_trace_task
// ���� : unused
// �Ԃ�l : �Ȃ�
// �T�v : 
//
//*****************************************************************************
#define DELTA_T 0.004
#define KP 0.5
#define KI 0.0
#define KD 0.03
static int diff [2];
static float integral=0;

void line_trace_task(intptr_t unused)
{
	signed char forward;      /* �O��i���� */
    signed char turn;         /* ���񖽗� */
    signed char pwm_L, pwm_R; /* ���E���[�^PWM�o�� */

    int32_t motor_ang_l, motor_ang_r;
    int gyro, volt;
    uint8_t color_sensor_reflect;
    
    tail_control(TAIL_ANGLE_DRIVE); /* �o�����X���s�p�p�x�ɐ��� */

    color_sensor_reflect= ev3_color_sensor_get_reflect(color_sensor);


    int temp_turn=1000;
    int temp_p=1000;
	int temp_d=1000;

    if (sonar_alert() == 1) /* ��Q�����m */
    {
        forward = turn = 0; /* ��Q�������m�������~ */
    }
    else
    {
        forward = 80; /* �O�i���� */
        float p,i,d;
        diff[0] = diff[1];
        diff[1] = color_sensor_reflect - ((LIGHT_WHITE + LIGHT_BLACK)/2);
        integral += (diff[1] - diff[0]) / 2.0 * DELTA_T;
        
        p = KP * diff[1];
        i = KI * integral;
        d = KD * (diff[1]-diff[0]) / DELTA_T;
        
        turn = p + i + d;
        temp_turn = turn;
        temp_p = p;
        temp_d = d;
        
        if(100 < turn)
        {
            turn = 100;
        }
        else if(turn < -100)
        {
            turn = -100;
        }
    }

    /* �|���U�q����API �ɓn���p�����[�^���擾���� */
    motor_ang_l = ev3_motor_get_counts(left_motor);
    motor_ang_r = ev3_motor_get_counts(right_motor);
    gyro = ev3_gyro_sensor_get_rate(gyro_sensor);
    volt = ev3_battery_voltage_mV();

    log_str(color_sensor_reflect,(int16_t)gyro,(int16_t)temp_turn, (int16_t)temp_p, (int16_t)temp_d);


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

	if(ev3_button_is_pressed(BACK_BUTTON))
	{
		wup_tsk(MAIN_TASK);
	}
    if(gyro < -150 || 150 < gyro)
    {
        wup_tsk(MAIN_TASK);
    }
}

//*****************************************************************************
// �֐��� : stair_task
// ���� : unused
// �Ԃ�l : �Ȃ�
// �T�v : 
//
//*****************************************************************************
void stair_task(intptr_t unused)
{
    // T.B.D
}

//*****************************************************************************
// �֐��� : garage_task
// ���� : unused
// �Ԃ�l : �Ȃ�
// �T�v : 
//
//*****************************************************************************
void garage_task(intptr_t unused)
{
    // T.B.D
}

//*****************************************************************************
// �֐��� : look_up_gate_task
// ���� : unused
// �Ԃ�l : �Ȃ�
// �T�v : 
//
//*****************************************************************************
void look_up_gate_task(intptr_t unused)
{
    // T.B.D
}
