#include "ap.h"






void apInit(void)
{  
  logBoot(false);
}

void apMain(void)
{
  void (**jump_func)(void) = (void (**)(void))(0x90400000 + 0x400 + 4); 


  logPrintf("[  ] Jump To Firmware\n");
  if (((uint32_t)*jump_func) >= 0x90400000 && ((uint32_t)*jump_func) < (0x90400000 + 32*1024))
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

  while(1);
}
