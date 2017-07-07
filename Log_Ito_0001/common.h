/*
 * common.h
 * 
 * 共通定義ファイル
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ev3api.h"
#include "balancer.h"

/**
 * 戻り値定義
 */
typedef enum{
    RET_OK = 0,
    RET_ERROR
} RETCODE ;

/**
 * 状態定義
 */
typedef enum{
    STAT_UNKNOWN = 0,
    STAT_NORMAL,
    STAT_STAIR,
    STAT_LOOK_UP_GATE,
    STAT_GAREGE,
} STATUS ;

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

/* 下記のマクロは個体/環境に合わせて変更する必要があります */
/* sample_c1マクロ */
#define GYRO_OFFSET  0          /* ジャイロセンサオフセット値(角速度0[deg/sec]時) */

/* sample_c2マクロ */
#define SONAR_ALERT_DISTANCE 30 /* 超音波センサによる障害物検知距離[cm] */
/* sample_c3マクロ */
#define TAIL_ANGLE_STAND_UP  85 /* 完全停止時の角度[度] */
#define TAIL_ANGLE_START     98 /* スタート時の角度[度] */
#define TAIL_ANGLE_DRIVE      3 /* バランス走行時の角度[度] */
#define P_GAIN             2.5F /* 完全停止用モータ制御比例係数 */
#define PWM_ABS_MAX          60 /* 完全停止用モータ制御PWM絶対最大値 */

#define DISTANCE_NOTIFY (1000.0)

/* sample_c4マクロ */
//#define DEVICE_NAME     "ET0"  /* Bluetooth名 hrp2/target/ev3.h BLUETOOTH_LOCAL_NAMEで設定 */
//#define PASS_KEY        "1234" /* パスキー    hrp2/target/ev3.h BLUETOOTH_PIN_CODEで設定 */
#define CMD_START         '1'    /* リモートスタートコマンド */

/* LCDフォントサイズ */
#define CALIB_FONT (EV3_FONT_SMALL)
#define CALIB_FONT_WIDTH (6/*TODO: magic number*/)
#define CALIB_FONT_HEIGHT (8/*TODO: magic number*/)


/* 関数プロトタイプ宣言 */
extern float tail_control(signed int angle);
extern int sonar_alert(void);


#ifdef __cplusplus
}
#endif

#endif /* _COMMON_H_ */

/* end of file */
