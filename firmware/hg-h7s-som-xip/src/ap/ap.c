#include "ap.h"






void apInit(void)
{
  // void (**jump_func)(void) = (void (**)(void))(FLASH_ADDR_FIRM + FLASH_SIZE_TAG + 4); 


  // qspiSetXipMode(true);
  // logPrintf("XIP Enable\r\n"); 

  // logPrintf("[  ] Jump To Firmware\n");
  // if (((uint32_t)*jump_func) >= FLASH_ADDR_FIRM && ((uint32_t)*jump_func) < (FLASH_ADDR_FIRM + FLASH_SIZE_FIRM))
  // {  
  //   logPrintf("[OK] addr : 0x%X\n", (uint32_t)*jump_func);
  //   bspDeInit();

  //   (*jump_func)();
  // }
  // else
  // {
  //   logPrintf("[E_] Invalid Jump Address\n");
  //   logPrintf("     addr : 0x%X\n", (uint32_t)*jump_func);
  // }  

  // qspiSetXipMode(false);
  // logPrintf("XIP Disable\r\n"); 

  #ifdef _USE_HW_CLI
  cliOpen(HW_UART_CH_CLI, 115200);  
  cliLogo();
  logBoot(false);
  #endif
}

void apMain(void)
{
  uint32_t pre_time;

  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 100)
    {
      pre_time = millis();
    }
    #ifdef _USE_HW_CLI
    cliMain();
    #endif    
  }
}
