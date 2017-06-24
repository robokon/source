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
	int32_t LEFT_info;
	int32_t RIGHT_info;
	
	/* モータのフルパワーのパーセント値を設定 */
	pwm_L = 30;
	pwm_R = 30;
	
	/* 尻尾を下す */
	tail_control(TAIL_ANGLE_STAND_UP);
	
    /* EV3ではモーター停止時のブレーキ設定が事前にできないため */
    /* 出力0時に、その都度設定する */
    if (pwm_L == 0 || RIGHT_info >= 1000 )
    {
         ev3_motor_stop(left_motor, true);
    }
    else
    {
    	/* パーセント値の*/
        ev3_motor_set_power(left_motor, -1 * (int)pwm_L);
    	
    	/* 左モータの各位置を取得する */
    	LEFT_info += ev3_motor_get_counts(left_motor);
    }
    
    if (pwm_R == 0 || RIGHT_info >= 1000 )
    {
         ev3_motor_stop(right_motor, true);
    }
    else
    {
        ev3_motor_set_power(right_motor, (int)pwm_R);
    	
    	/* 右モータの角位置を取得する */
    	RIGHT_info += ev3_motor_get_counts(right_motor);
    }
	
	if( RIGHT_info >= 1000 )
	{
		ev3_motor_stop(left_motor, true);
		ev3_motor_stop(right_motor, true);
	}
}

/* end of file */
