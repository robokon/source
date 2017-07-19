/*
 * line_tarce.h
 * 
 */

#include "line_trace.h"

//*****************************************************************************
// 関数名 : line_tarce_main
// 引数 : 
// 返り値 : なし
// 概要 : 
//       
//*****************************************************************************

#define DELTA_T 0.004
signed char forward;              /* 前後進命令 */
signed char turn;                 /* 旋回命令 */
signed char pwm_L, pwm_R;         /* 左右モータPWM出力 */
static float integral=0;          /* I制御 */
static int diff [2];              /* カラーセンサの差分 */ 
int count  = 0;                   /* ログ出力 */
/* PIDパラメータ */
#define KP 0.5
#define KI 0.0
#define KD 0.03

void line_tarce_main(int color_sensor)
{
    signed char forward;      /* 前後進命令 */
    signed char turn;         /* 旋回命令 */
    signed char pwm_L, pwm_R; /* 左右モータPWM出力 */

    int32_t motor_ang_l, motor_ang_r;
    int gyro, volt;
    uint8_t color_sensor_reflect;
    
    tail_control(TAIL_ANGLE_DRIVE); /* バランス走行用角度に制御 */

    color_sensor_reflect= ev3_color_sensor_get_reflect(color_sensor);

    int temp_p=1000;
    int temp_d=1000;

    if (sonar_alert() == 1) /* 障害物検知 */
    {
        forward = turn = 0; /* 障害物を検知したら停止 */
    }
    else
    {
    	/* PID制御 */
        forward = 80; /* 前進命令 */
        float p,i,d;
        diff[0] = diff[1];
        diff[1] = color_sensor_reflect - ((color_sensor)/2);
        integral += (diff[1] + diff[0]) / 2.0 * DELTA_T;
        
        p = KP * diff[1];
        i = KI * integral;
        d = KD * (diff[1]-diff[0]) / DELTA_T;
        
        turn = p + i + d;
        temp_p = p;
        temp_d = d;
        
        /* モータ値調整 */
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

    /* ログ出力 */
    count++;
    if(count%25==0)
    {
        log_str(color_sensor_reflect,(int16_t)gyro, (int16_t)temp_p, (int16_t)temp_d, (int16_t)count);
    }

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
    
    /* 戻るボタンor転んだら終了 */
    if(ev3_button_is_pressed(BACK_BUTTON))
    {
        wup_tsk(MAIN_TASK);
    }
    if(gyro < -150 || 150 < gyro)
    {
        wup_tsk(MAIN_TASK);
    }
}

/* end of file */
