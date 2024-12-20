#include "hw.h"



extern uint32_t _fw_flash_begin;

volatile const firm_ver_t firm_ver __attribute__((section(".version"))) = 
{
  .magic_number = VERSION_MAGIC_NUMBER,
  .version_str  = _DEF_FIRMWATRE_VERSION,
  .name_str     = _DEF_BOARD_NAME,
  .firm_addr    = (uint32_t)&_fw_flash_begin
};



bool hwInit(void)
{  
  #ifdef _USE_HW_CLI
  cliInit();
  #endif
  logInit();  

  uartInit();
  for (int i=0; i<HW_UART_MAX_CH; i++)
  {
    uartOpen(i, 115200);
  }

  logOpen(HW_LOG_CH, 115200);
  logPrintf("\r\n[ XIP Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", _DEF_BOARD_NAME);
  logPrintf("Booting..Ver  \t\t: %s\r\n", _DEF_FIRMWATRE_VERSION);  
  logPrintf("Booting..Clock\t\t: %d Mhz\r\n", (int)HAL_RCC_GetSysClockFreq()/1000000);

  rtcInit();
  resetInit();
  psramInit();
  qspiInit();

  #ifdef _USE_HW_CACHE
  SCB_EnableICache();
  SCB_EnableDCache();
  #endif  

  logPrintf("[  ] ICache  %s\n", (SCB->CCR & SCB_CCR_IC_Msk) ? "ON":"OFF");
  logPrintf("[  ] DCache  %s\n", (SCB->CCR & SCB_CCR_DC_Msk) ? "ON":"OFF");

  return true;
}