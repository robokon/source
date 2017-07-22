#include "ev3api.h"
#include "app.h"
#include "balancer.h"
#include "common.h"
#include "stair.h"
#include "module_cfg.h"
#include "Distance.h" 

signed char forward;      /* 前後進命令 */
signed char turn;         /* 旋回命令 */
signed char pwm_L, pwm_R; /* 左右モータPWM出力 */


static int counter = 0;
static int gyro_str = 0;
static int gyro_log[3000];
static int status = 0;

/* プロトタイプ宣言 */
void stair_A(void);
void log_commit(void);

//*****************************************************************************
// 関数名 : stair_main
// 引数 : 
// 返り値 : なし
// 概要 : 
//       
//*****************************************************************************
void stair_main()
{
    if (ev3_touch_sensor_is_pressed(touch_sensor) == 1)
    {
      log_commit(); /* タッチセンサが押された */
    }
	
    /* ジャイロ角速度にて階段を検知する */
    if((counter % 10) == 0)
    {
        /* 40 msごとにgyro_strにジャイロ角速度センサー値を入手 (4 ms周期と仮定) */
        gyro_str = ev3_gyro_sensor_get_rate(gyro_sensor);
     }
    
     /* 40 ms前のセンサー値と今のセンサー値の差分が規定値より大きい場合階段にぶつかったと判断する */
    if((gyro_str - ev3_gyro_sensor_get_rate(gyro_sensor)) > 250 ||
      (gyro_str - ev3_gyro_sensor_get_rate(gyro_sensor)) < (250 * (-1)) )
    {
         /* 階段検知を音で示す */
       ev3_speaker_set_volume(100); 
        ev3_speaker_play_tone(NOTE_C4, 100);
	} 
	

	status = 0;
    gyro_log[counter] = ev3_gyro_sensor_get_rate(gyro_sensor);
    
    switch(status){
       case 0:
         stair_A();
         break;
           
       default:
         break;
    }

    counter++;
    
}


void stair_A(void)
{
    int32_t motor_ang_l, motor_ang_r;
    int gyro, volt;

        tail_control(TAIL_ANGLE_DRIVE); /* バランス走行用角度に制御 */

        if (sonar_alert() == 1) /* 障害物検知 */
        {
            forward = turn = 0; /* 障害物を検知したら停止 */
        }
        else
        {
            forward = 60; /* 前進命令 */
            turn =  0; /* 左旋回命令 */
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

void log_commit(void)
{
    FILE *fp; /* ファイルポインタ */
	int  i;   /* インクリメント */

    /* Logファイル作成 */
	fp=fopen("170708_Stair_Log.csv","a");
	/* 列タイトル挿入 */
	fprintf(fp,"ジャイロセンサ角速度　\n");
	
	/* Logの出力 */
	for(i = 0 ; i < 3000; i++)
	{
		fprintf(fp,"%d\n", gyro_log[i]);
	}
	
	fclose(fp);
}

/* end of file */
