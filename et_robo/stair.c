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
	int counter = 0; /* 4msごとのカウンタ値 */
	int loop = 1;
	int False = 0;
	int gyro_str = 0;
	int stair_num = 0;
	int limit_count = 25000; /* 100 s */
	#define GYRO_DIV                200
	
	
    tail_control(TAIL_ANGLE_STAND_UP); /* 倒立制御を削除 尻尾を下す */
	
	while(loop)
	{
		/* ジャイロ角速度にて階段を検知する */
		if((counter % 10) == 0)
		{
			/* 40 msごとにgyro_strにジャイロ角速度センサー値を格納 (4 ms周期と仮定) */
			gyro_str = ev3_gyro_sensor_get_rate(gyro_sensor);
		}

		/* 40 ms前のセンサー値と今のセンサー値の差分が規定値より大きい場合　階段にぶつかったと判断する */
		if((gyro_str - ev3_gyro_sensor_get_rate(gyro_sensor)) > GYRO_DIV ||
		(gyro_str - ev3_gyro_sensor_get_rate(gyro_sensor)) < (GYRO_DIV * (-1)) )
	    {
			counter++;
            /* 階段を検知 */
			stair_num++;
			
			/* 階段検知を音で示す */
			ev3_speaker_set_volume(100); 
		    ev3_speaker_play_tone(NOTE_C4, 100);
			
			/* 10msecウェイト */
			tslp_tsk(10);
			
			/* ちょっとバック */
			ev3_motor_rotate (left_motor, -360, 30, false );
			ev3_motor_rotate (right_motor, -360, 30, true );
			
			/* 勢いつけて進む */
			ev3_motor_rotate (left_motor, 3600, 90, false );
			ev3_motor_rotate (right_motor, 3600, 90, true );
			
			/* 40msecウェイト */
			tslp_tsk(40);
			
		    /* 階段上ったことを音で示す */
		    ev3_speaker_play_tone(NOTE_E6, 100);
			continue;
		  
		    if(stair_num == 2)
			{
				/* 二回目の階段検知をしたらloopから抜ける */
				loop = False;
			}
		}
		else
		{
			counter++;
			/* 階段を検知するまで前進命令 */
			ev3_motor_rotate (left_motor, 360, 30, false );
			ev3_motor_rotate (right_motor, 360, 30, false );
			
			/* 安全装置 一定時間経過しても2回目の階段検知できなければloopから脱出 */
			if(counter > limit_count)
			{
				loop = False;
			}
			continue;
		}

	}
	
}

/* end of file */
