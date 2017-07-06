/**
 ******************************************************************************
 ** ファイル名 : app.c
 **
 ** 概要 : 2輪倒立振子ライントレースロボットのTOPPERS/HRP2用Cサンプルプログラム
 **
 ** 注記 : sample_c4 (sample_c3にBluetooth通信リモートスタート機能を追加)
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
 * センサー、モーターの接続を定義します
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

static int      bt_cmd = 0;     /* Bluetoothコマンド 1:リモートスタート */
static FILE     *bt = NULL;     /* Bluetoothファイルハンドル */

static int LIGHT_WHITE=0;         /* 白色の光センサ値 */
static int LIGHT_BLACK=100;          /* 黒色の光センサ値 */

/* 下記のマクロは個体/環境に合わせて変更する必要があります */
/* sample_c1マクロ */
#define GYRO_OFFSET  0          /* ジャイロセンサオフセット値(角速度0[deg/sec]時) */

/* sample_c2マクロ */
#define SONAR_ALERT_DISTANCE 30 /* 超音波センサによる障害物検知距離[cm] */
/* sample_c3マクロ */
#define TAIL_ANGLE_STAND_UP  93 /* 完全停止時の角度[度] */
#define TAIL_ANGLE_DRIVE      3 /* バランス走行時の角度[度] */
#define P_GAIN             2.5F /* 完全停止用モータ制御比例係数 */
#define PWM_ABS_MAX          60 /* 完全停止用モータ制御PWM絶対最大値 */
/* sample_c4マクロ */
//#define DEVICE_NAME     "ET0"  /* Bluetooth名 hrp2/target/ev3.h BLUETOOTH_LOCAL_NAMEで設定 */
//#define PASS_KEY        "1234" /* パスキー    hrp2/target/ev3.h BLUETOOTH_PIN_CODEで設定 */
#define CMD_START         '1'    /* リモートスタートコマンド */

/* LCDフォントサイズ */
#define CALIB_FONT (EV3_FONT_SMALL)
#define CALIB_FONT_WIDTH (6/*TODO: magic number*/)
#define CALIB_FONT_HEIGHT (8/*TODO: magic number*/)


#if (LOG_TASK == TASK_ON)
/* Log の最大回数 */
#define  LOG_MAX   1000

/* Log fileの名前 */
#define  LOG_FILE_NAME  "Log_yymmdd.csv"

/* Log用の構造体 */
/*  反射光センサー値
   ジャイロセンサ角位置
   ジャイロセンサ角速度 */
 typedef struct{
    uint8_t Reflect;
    int16_t Gyro_angle;
    int16_t Gyro_rate;  
    int16_t Turn;
 	int16_t P;
 	int16_t D;
}Logger;

/* Log回数の格納変数 */
int LogNum = 0;

/* Log格納配列 */
Logger gst_Log_str[LOG_MAX]; /* (1s == 250) */
#endif

/* 関数プロトタイプ宣言 */
static int sonar_alert(void);
static void tail_control(signed int angle);


#if (LOG_TASK == TASK_ON)
void log_str(uint8_t reflect, int16_t rate, int16_t turn,int16_t p,int16_t d);
void log_commit(void);
#endif


/* メインタスク */
void main_task(intptr_t unused)
{

    /* LCD画面表示 */
    ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);
    ev3_lcd_draw_string("EV3way-ET sample_c4", 0, CALIB_FONT_HEIGHT*1);

    /* センサー入力ポートの設定 */
    ev3_sensor_config(sonar_sensor, ULTRASONIC_SENSOR);
    ev3_sensor_config(color_sensor, COLOR_SENSOR);
    ev3_color_sensor_get_reflect(color_sensor); /* 反射率モード */
    ev3_sensor_config(touch_sensor, TOUCH_SENSOR);
    ev3_sensor_config(gyro_sensor, GYRO_SENSOR);
    /* モーター出力ポートの設定 */
    ev3_motor_config(left_motor, LARGE_MOTOR);
    ev3_motor_config(right_motor, LARGE_MOTOR);
    ev3_motor_config(tail_motor, LARGE_MOTOR);
    ev3_motor_reset_counts(tail_motor);

    /* Open Bluetooth file */
    bt = ev3_serial_open_file(EV3_SERIAL_BT);
    assert(bt != NULL);

    /* Bluetooth通信タスクの起動 */
    act_tsk(BT_TASK);

    ev3_led_set_color(LED_ORANGE); /* 初期化完了通知 */

    /*白色の光センサ値取得*/
    while(1)
    {
        if (ev3_touch_sensor_is_pressed(touch_sensor) == 1)
        {
            LIGHT_WHITE = ev3_color_sensor_get_reflect(color_sensor);
            log_str(LIGHT_WHITE,0,0,0,0);
            break; /* タッチセンサが押された */
        }
        tslp_tsk(10); /* 10msecウェイト */
    }
     tslp_tsk(1000); /* 10msecウェイト */
    /*黒色の光センサ値*/
    while(1)
    {
        if (ev3_touch_sensor_is_pressed(touch_sensor) == 1)
        {
            LIGHT_BLACK = ev3_color_sensor_get_reflect(color_sensor);
            log_str(LIGHT_BLACK,0,0,0,0);
            break; /* タッチセンサが押された */
        }
        tslp_tsk(10); /* 10msecウェイト */
    }
    tslp_tsk(1000); /* 10msecウェイト */

    
    /* スタート待機 */
    while(1)
    {
        tail_control(TAIL_ANGLE_STAND_UP); /* 完全停止用角度に制御 */

        if (bt_cmd == 1)
        {
            break; /* リモートスタート */
        }

        if (ev3_touch_sensor_is_pressed(touch_sensor) == 1)
        {
            break; /* タッチセンサが押された */
        }
        tslp_tsk(10); /* 10msecウェイト */
    }
    
    /* 走行モーターエンコーダーリセット */
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);

    /* ジャイロセンサーリセット */
    ev3_gyro_sensor_reset(gyro_sensor);
    balance_init(); /* 倒立振子API初期化 */

    ev3_led_set_color(LED_GREEN); /* スタート通知 */

    /**
    * Main loop for the self-balance control algorithm
    */
    // 周期ハンドラ開始
    ev3_sta_cyc(TEST_EV3_CYC1);
    //ev3_sta_cyc(TEST_EV3_CYC2);

	// バックボタンが押されるまで待つ
	slp_tsk();
    // 周期ハンドラ停止
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
// 関数名 : test_ev3_cys1
// 引数 : 無し
// 返り値 :
// 概要 : 周期タスク（ライントレース）
//*****************************************************************************
void test_ev3_cys1(intptr_t idx) 
{
    act_tsk(LINE_TRACE_TASK);
}

//*****************************************************************************
// 関数名 : test_ev3_cys2
// 引数 : 無し
// 返り値 :
// 概要 : 周期タスク（ログ）
//*****************************************************************************
void test_ev3_cys2(intptr_t idx) 
{
    act_tsk(LOG_CREATE_TASK);
}

//*****************************************************************************
// 関数名 : sonar_alert
// 引数 : 無し
// 返り値 : 1(障害物あり)/0(障害物無し)
// 概要 : 超音波センサによる障害物検知
//*****************************************************************************
void log_create_task(intptr_t idx) 
{
	//log_str();
}

//*****************************************************************************
// 関数名 : sonar_alert
// 引数 : 無し
// 返り値 : 1(障害物あり)/0(障害物無し)
// 概要 : 超音波センサによる障害物検知
//*****************************************************************************
static int sonar_alert(void)
{
    static unsigned int counter = 0;
    static int alert = 0;

    signed int distance;

    if (++counter == 40/4) /* 約40msec周期毎に障害物検知  */
    {
        /*
         * 超音波センサによる距離測定周期は、超音波の減衰特性に依存します。
         * NXTの場合は、40msec周期程度が経験上の最短測定周期です。
         * EV3の場合は、要確認
         */
        distance = ev3_ultrasonic_sensor_get_distance(sonar_sensor);
        if ((distance <= SONAR_ALERT_DISTANCE) && (distance >= 0))
        {
            alert = 1; /* 障害物を検知 */
        }
        else
        {
            alert = 0; /* 障害物無し */
        }
        counter = 0;
    }

    return alert;
}

//*****************************************************************************
// 関数名 : tail_control
// 引数 : angle (モータ目標角度[度])
// 返り値 : 無し
// 概要 : 走行体完全停止用モータの角度制御
//*****************************************************************************
static void tail_control(signed int angle)
{
    float pwm = (float)(angle - ev3_motor_get_counts(tail_motor))*P_GAIN; /* 比例制御 */
    /* PWM出力飽和処理 */
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
// 関数名 : bt_task
// 引数 : unused
// 返り値 : なし
// 概要 : Bluetooth通信によるリモートスタート。 Tera Termなどのターミナルソフトから、
//       ASCIIコードで1を送信すると、リモートスタートする。
//*****************************************************************************
void bt_task(intptr_t unused)
{
    while(1)
    {
        uint8_t c = fgetc(bt); /* 受信 */
        switch(c)
        {
        case '1':
            bt_cmd = 1;
            break;
        default:
            break;
        }
        fputc(c, bt); /* エコーバック */
    }
}


#if (LOG_TASK == TASK_ON)
//*****************************************************************************
// 関数名 : log_str
// 引数 : なし
// 返り値 : なし
// 概要 : グローバル配列 gst_Log_strに現在のセンサー値を格納
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
// 関数名 : log_commit
// 引数 : なし
// 返り値 : なし
// 概要 : グローバル配列 gst_Log_strに格納されているデータをファイル出力する
//
//*****************************************************************************
void log_commit(void)
{
    FILE *fp; /* ファイルポインタ */
	int  i;   /* インクリメント */

    /* Logファイル作成 */
	fp=fopen(LOG_FILE_NAME,"a");
	/* 列タイトル挿入 */
	fprintf(fp,"反射光センサー, ジャイロ角度, ジャイロセンサ角速度,ターン,P,D　\n");
	
	/* Logの出力 */
	for(i = 0 ; i < LOG_MAX; i++)
	{
		fprintf(fp,"%d,%d,%d,%d,%d,%d\n",gst_Log_str[i].Reflect, gst_Log_str[i].Gyro_angle, gst_Log_str[i].Gyro_rate, gst_Log_str[i].Turn, gst_Log_str[i].P, gst_Log_str[i].D);
	}
	
	fclose(fp);
}

#endif
//*****************************************************************************
// 関数名 : line_trace_task
// 引数 : unused
// 返り値 : なし
// 概要 : 
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
	signed char forward;      /* 前後進命令 */
    signed char turn;         /* 旋回命令 */
    signed char pwm_L, pwm_R; /* 左右モータPWM出力 */

    int32_t motor_ang_l, motor_ang_r;
    int gyro, volt;
    uint8_t color_sensor_reflect;
    
    tail_control(TAIL_ANGLE_DRIVE); /* バランス走行用角度に制御 */

    color_sensor_reflect= ev3_color_sensor_get_reflect(color_sensor);


    int temp_turn=1000;
    int temp_p=1000;
	int temp_d=1000;

    if (sonar_alert() == 1) /* 障害物検知 */
    {
        forward = turn = 0; /* 障害物を検知したら停止 */
    }
    else
    {
        forward = 80; /* 前進命令 */
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

    /* 倒立振子制御API に渡すパラメータを取得する */
    motor_ang_l = ev3_motor_get_counts(left_motor);
    motor_ang_r = ev3_motor_get_counts(right_motor);
    gyro = ev3_gyro_sensor_get_rate(gyro_sensor);
    volt = ev3_battery_voltage_mV();

    log_str(color_sensor_reflect,(int16_t)gyro,(int16_t)temp_turn, (int16_t)temp_p, (int16_t)temp_d);


    /* 倒立振子制御APIを呼び出し、倒立走行するための */
    /* 左右モータ出力値を得る */
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

    /* EV3ではモーター停止時のブレーキ設定が事前にできないため */
    /* 出力0時に、その都度設定する */
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
// 関数名 : stair_task
// 引数 : unused
// 返り値 : なし
// 概要 : 
//
//*****************************************************************************
void stair_task(intptr_t unused)
{
    // T.B.D
}

//*****************************************************************************
// 関数名 : garage_task
// 引数 : unused
// 返り値 : なし
// 概要 : 
//
//*****************************************************************************
void garage_task(intptr_t unused)
{
    // T.B.D
}

//*****************************************************************************
// 関数名 : look_up_gate_task
// 引数 : unused
// 返り値 : なし
// 概要 : 
//
//*****************************************************************************
void look_up_gate_task(intptr_t unused)
{
    // T.B.D
}
