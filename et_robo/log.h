/*
 * log.h
 */

#ifndef _LOG_H_
#define _LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

extern void log_str(uint8_t reflect, int16_t rate, int16_t p, int16_t d, int16_t count);
extern void log_commit(void);
    
#ifdef __cplusplus
}
#endif


#endif /* _LOG_H_ */

/* end of file */
