#include "hw.h"




bool hwInit(void)
{  
  logInit();  

  uartInit();
  for (int i=0; i<HW_UART_MAX_CH; i++)
  {
    uartOpen(i, 115200);
  }

  logOpen(HW_LOG_CH, 115200);
  logPrintf("\r\n[ FSBL Begin... ]\r\n");

  logPrintf("\n");
  logPrintf("[  ] ICache  %s\n", (SCB->CCR & SCB_CCR_IC_Msk) ? "ON":"OFF");
  logPrintf("[  ] DCache  %s\n", (SCB->CCR & SCB_CCR_DC_Msk) ? "ON":"OFF");
    
  return true;
}