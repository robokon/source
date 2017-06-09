#include "stair.h"

signed char forward;      /* 前後進命令 */
signed char turn;         /* 旋回命令 */
signed char pwm_L, pwm_R; /* 左右モータPWM出力 */

//*****************************************************************************
// 関数名 : stair_main
// 引数 : 
// 返り値 : なし
// 概要 : 
//       
//*****************************************************************************
void stair_main()
{
    int32_t motor_ang_l, motor_ang_r;
    int gyro, volt;

    if (ev3_button_is_pressed(BACK_BUTTON)) return;

    tail_control(TAIL_ANGLE_DRIVE); /* バランス走行用角度に制御 */

    if (sonar_alert() == 1) /* 障害物検知 */
    {
        forward = turn = 0; /* 障害物を検知したら停止 */
    }
    else
    {
        forward = 30; /* 前進命令 */
        if (ev3_color_sensor_get_reflect(color_sensor) >= (LIGHT_WHITE + LIGHT_BLACK)/2)
        {
            turn =  20; /* 左旋回命令 */
        }
        else
        {
            turn = -20; /* 右旋回命令 */
        }
    }

    /* 倒立振子制御API に渡すパラメータを取得する */
    motor_ang_l = ev3_motor_get_counts(left_motor);
    motor_ang_r = ev3_motor_get_counts(right_motor);
    gyro = ev3_gyro_sensor_get_rate(gyro_sensor);
    volt = ev3_battery_voltage_mV();

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
}

/* end of file */
