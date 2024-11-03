#include "ap.h"



static void cliBoot(cli_args_t *args);



void apInit(void)
{
  qspiSetXipMode(true);
  logPrintf("XIP Enable\r\n"); 

  #if 0
  void (**jump_func)(void) = (void (**)(void))(FLASH_ADDR_FIRM + FLASH_SIZE_TAG + 4); 

  logPrintf("[  ] Jump To Firmware\n");
  if (((uint32_t)*jump_func) >= FLASH_ADDR_FIRM && ((uint32_t)*jump_func) < (FLASH_ADDR_FIRM + FLASH_SIZE_FIRM))
  {  
    logPrintf("[OK] addr : 0x%X\n", (uint32_t)*jump_func);
    bspDeInit();

    (*jump_func)();
  }
  else
  {
    logPrintf("[E_] Invalid Jump Address\n");
    logPrintf("     addr : 0x%X\n", (uint32_t)*jump_func);
  }  

  qspiSetXipMode(false);
  logPrintf("XIP Disable\r\n"); 
  #endif

  #ifdef _USE_HW_CLI
  cliOpen(HW_UART_CH_CLI, 115200);  
  logBoot(false);
  cliAdd("boot", cliBoot);
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

#ifdef _USE_HW_CLI
void cliBoot(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "jump"))
  {
    void (**jump_func)(void) = (void (**)(void))(FLASH_ADDR_FIRM + FLASH_SIZE_TAG + 4); 


    // memcpy((void *)(FLASH_ADDR_PSRAM + FLASH_SIZE_TAG), (void *)0x70100000, 1*1024*1024);

    logPrintf("[  ] Jump To Firmware\n");
    if (((uint32_t)*jump_func) >= FLASH_ADDR_FIRM && ((uint32_t)*jump_func) < (FLASH_ADDR_FIRM + FLASH_SIZE_FIRM))
    {  
      logPrintf("[OK] addr : 0x%X\n", (uint32_t)*jump_func);
      bspDeInit();

      #ifdef _USE_HW_CACHE
      SCB_DisableICache();
      SCB_DisableDCache();
      #endif  

      (*jump_func)();
    }
    else
    {
      logPrintf("[E_] Invalid Jump Address\n");
      logPrintf("     addr : 0x%X\n", (uint32_t)*jump_func);
    }      
    ret = true;
  }

  if (!ret)
  {
    cliPrintf("boot jump\n");
  }
}
#endif