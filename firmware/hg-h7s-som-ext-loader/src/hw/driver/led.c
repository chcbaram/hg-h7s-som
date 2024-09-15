#include "led.h"


#ifdef _USE_HW_LED



bool ledInit(void)
{
  return true;
}

void ledOn(uint8_t ch)
{
  if (ch >= LED_MAX_CH) return;

}

void ledOff(uint8_t ch)
{
  if (ch >= LED_MAX_CH) return;
}

void ledToggle(uint8_t ch)
{
  if (ch >= LED_MAX_CH) return;
}
#endif