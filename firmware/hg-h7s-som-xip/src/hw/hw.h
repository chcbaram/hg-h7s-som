#ifndef HW_H_
#define HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#include "led.h"
#include "uart.h"
#include "cli.h"
#include "log.h"
#include "flash.h"
#include "rtc.h"
#include "reset.h"
#include "qspi.h"
#include "psram.h"


bool hwInit(void);


#ifdef __cplusplus
}
#endif

#endif