#include "Distance.h"

#define TIRE_DIAMETER 81.0  /* ^C¼ai81mmj*/
#define PI 3.14159265358    /* ~ü¦ */

static float distance = 0.0;     //s£
static float distance4msL = 0.0; //¶^CÌ4msÔÌ£
static float distance4msR = 0.0; //E^CÌ4msÔÌ£
static float pre_angleL, pre_angleR; // ¶E[^ñ]pxÌßl

/* ú»Ö */
void Distance_init()
{
    //eÏÌlÌú»
    distance = 0.0;
    distance4msR = 0.0;
    distance4msL = 0.0;
    //[^pxÌßlÉ»Ýlðãü
    pre_angleL = ev3_motor_get_counts(left_motor);
    pre_angleR = ev3_motor_get_counts(right_motor);
}

/* £XVi4msÔÌÚ®£ðñÁZµÄ¢éj */
void Distance_update()
{
    float cur_angleL = ev3_motor_get_counts(left_motor); //¶[^ñ]pxÌ»Ýl
    float cur_angleR = ev3_motor_get_counts(right_motor);//E[^ñ]pxÌ»Ýl
    float distance4ms = 0.0;        //4msÌ£

    // 4msÔÌs£ = ((~ü¦ * ^CÌ¼a) / 360) * ([^pxßl@- [^px»Ýl)
    distance4msL = ((PI * TIRE_DIAMETER) / 360.0) * (cur_angleL - pre_angleL);  // 4msÔÌ¶[^£
    distance4msR = ((PI * TIRE_DIAMETER) / 360.0) * (cur_angleR - pre_angleR);  // 4msÔÌE[^£
    distance4ms = (distance4msL + distance4msR) / 2.0; //¶E^CÌs£ð«µÄé
    distance += distance4ms;

    //[^Ìñ]pxÌßlðXV
    pre_angleL = cur_angleL;
    pre_angleR = cur_angleR;
}

/* s£ðæ¾ */
float Distance_getDistance()
{
    return distance;
}

/* E^CÌ4msÔÌ£ðæ¾ */
float Distance_getDistance4msRight()
{
    return distance4msR;
}

/* ¶^CÌ4msÔÌ£ðæ¾ */
float Distance_getDistance4msLeft()
{
    return distance4msL;
}

/* end of file */
