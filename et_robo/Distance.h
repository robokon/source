#ifndef _DISTANCE_H_
#define _DISTANCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "ev3api.h"

/* 初期化関数 */
void Distance_init();

/* 距離を更新 */
void Distance_update();

/* 走行距離を取得 */
float Distance_getDistance();

/* 右タイヤの4ms間の距離を取得 */
float Distance_getDistance4msRight();

/* 左タイヤの4ms間の距離を取得 */
float Distance_getDistance4msLeft();

#ifdef __cplusplus
}
#endif

#endif /* _LINE_TRACE_H_ */

/* end of file */
