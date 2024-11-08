
#include "psram.h"

#ifdef _USE_HW_PSRAM
#include "psram/aps256xx.h"
#ifdef _USE_HW_CLI
#include "cli.h"
#endif



#define PSRAM_MAX_CH              1


#define PSRAM_ADDR_OFFSET         0x90000000
#define PSRAM_MAX_SIZE            (32*1024*1024)
#define PSRAM_SECTOR_SIZE         (4*1024)
#define PSRAM_PAGE_SIZE           (1024)
#define PSRAM_MAX_SECTOR          (FLASH_MAX_SIZE / FLASH_SECTOR_SIZE)


static bool psramInitHw(void);
static bool psramInitGpio();
static bool psramInitReg(void);
static bool psramGetVendorID(uint8_t *vendorId);
static bool psramReset(void);
static bool psramEnterMemoyMaped(void);


static uint32_t psram_addr   = PSRAM_ADDR_OFFSET;
static uint32_t psram_length = 0;

typedef struct
{
  bool     is_init;
  uint32_t id;
  uint32_t length;

} psram_tbl_t;


static psram_tbl_t psram_tbl[PSRAM_MAX_CH];

static XSPI_HandleTypeDef hxspi;


#ifdef _USE_HW_CLI
static void cliPsram(cli_args_t *args);
#endif





bool psramInit(void)
{
  bool ret = true;


  psram_tbl[0].is_init = false;
  psram_tbl[0].length = 0;


  logPrintf("[  ] psramInit()\n");

  if (psramInitHw() == true)
  {
    psram_length += PSRAM_MAX_SIZE;
  }

  if (psram_length == 0)
  {
    ret = false;
  }
  else
  {
    psram_tbl[0].is_init = false;
    psram_tbl[0].id = 0;
    ret = psramGetVendorID((uint8_t *)&psram_tbl[0].id);

    logPrintf("     ID : 0x%X\n", psram_tbl[0].id);

    if (ret == true && psram_tbl[0].id == 0x1F0D)
    {
      psram_tbl[0].is_init = true;
      psram_tbl[0].length = PSRAM_MAX_SIZE;

      logPrintf("     APS25608 Found\n");

      psramEnterMemoyMaped();
    }
    ret = psram_tbl[0].is_init;
  }

  logPrintf("[%s] psramInit()\n", ret ? "OK":"E_");

#ifdef _USE_HW_CLI
  cliAdd("psram", cliPsram);
#endif

  return ret;
}

uint32_t psramGetAddr(void)
{
  return psram_addr;
}

uint32_t psramGetLength(void)
{
  return psram_length;
}

bool psramInitHw(void)
{
  XSPIM_CfgTypeDef sXspiManagerCfg = {0};
  uint32_t xspi_clk, xspi_max_freq;


  psramInitGpio();


  xspi_max_freq = 200 * 1000000U; // 200Mhz
  xspi_clk      = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI1);

  logPrintf("     xspi_max_freq %d Mhz\n", xspi_max_freq/1000000);
  logPrintf("     xspi_clk      %d Mhz\n", xspi_clk/1000000);


  hxspi.Init.ClockPrescaler = (xspi_clk / xspi_max_freq);
  if ((xspi_clk % xspi_max_freq) == 0U)
  {
    hxspi.Init.ClockPrescaler = hxspi.Init.ClockPrescaler - 1U;
  }
  logPrintf("     prescaler     %d \n", hxspi.Init.ClockPrescaler);
  logPrintf("     xspi_clk      %d Mhz\n", xspi_clk/1000000/(hxspi.Init.ClockPrescaler + 1));

  hxspi.Instance                     = XSPI1;
  hxspi.Init.FifoThresholdByte       = 4;
  hxspi.Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  hxspi.Init.MemoryType              = HAL_XSPI_MEMTYPE_APMEM;
  hxspi.Init.MemorySize              = HAL_XSPI_SIZE_256MB;
  hxspi.Init.ChipSelectHighTimeCycle = 1;
  hxspi.Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi.Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  hxspi.Init.WrapSize                = HAL_XSPI_WRAP_32_BYTES;
  // hxspi.Init.ClockPrescaler          = 0;
  hxspi.Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi.Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_ENABLE;
  hxspi.Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_16KB;
  hxspi.Init.MaxTran                 = 0;
  hxspi.Init.Refresh                 = 0;
  hxspi.Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  if (HAL_XSPI_Init(&hxspi) != HAL_OK)
  {
    return false;
  }
  sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sXspiManagerCfg.IOPort      = HAL_XSPIM_IOPORT_1;
  if (HAL_XSPIM_Config(&hxspi, &sXspiManagerCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return false;
  }

  if (!psramReset())
  {
    logPrintf("[E_] psramReset()\n");
    return false;
  }

  if (!psramInitReg())
  {
    logPrintf("[E_] psramInitReg()\n");
    return false;
  }

  return true;
}

bool psramInitGpio(void)
{
  GPIO_InitTypeDef         GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit   = {0};


  /* Enable the SBS Clock */
  __HAL_RCC_SBS_CLK_ENABLE();


  /** Initializes the peripherals clock
   */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_XSPI1;
  PeriphClkInit.Xspi1ClockSelection  = RCC_XSPI1CLKSOURCE_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }


  /* XSPI1 clock enable */
  __HAL_RCC_XSPIM_CLK_ENABLE();
  __HAL_RCC_XSPI1_CLK_ENABLE();

  __HAL_RCC_GPIOP_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();
  /**XSPI1 GPIO Configuration
  PP5     ------> XSPIM_P1_IO5
  PP2     ------> XSPIM_P1_IO2
  PP6     ------> XSPIM_P1_IO6
  PP7     ------> XSPIM_P1_IO7
  PO0     ------> XSPIM_P1_NCS1
  PP4     ------> XSPIM_P1_IO4
  PP3     ------> XSPIM_P1_IO3
  PO4     ------> XSPIM_P1_CLK
  PP0     ------> XSPIM_P1_IO0
  PO2     ------> XSPIM_P1_DQS0
  PP1     ------> XSPIM_P1_IO1
  */
  GPIO_InitStruct.Pin       = GPIO_PIN_5 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P1;
  HAL_GPIO_Init(GPIOP, &GPIO_InitStruct);

  GPIO_InitStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_4 | GPIO_PIN_2;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P1;
  HAL_GPIO_Init(GPIOO, &GPIO_InitStruct);

  return true;
}

bool psramGetVendorID(uint8_t *vendorId)
{
  if (APS256XX_ReadID(&hxspi, vendorId, (uint32_t)APS256XX_READ_REG_LATENCY((uint32_t)(DEFAULT_READ_LATENCY_CODE))) != APS256XX_OK)
  {
    return false;
  }
  return true;
}

bool psramReset(void)
{
  if (APS256XX_Reset(&hxspi) != APS256XX_OK)
  {
    return false;
  }
  return true;
}

bool psramInitReg(void)
{
  uint8_t reg[2] = {0};

  // Reading the configuration of Mode Register 0
  //
  if (APS256XX_ReadReg(&hxspi, APS256XX_MR0_ADDRESS, reg,
                       (uint32_t)APS256XX_READ_REG_LATENCY((uint32_t)(DEFAULT_READ_LATENCY_CODE))) != APS256XX_OK)
  {
    return false;
  }
  logPrintf("     MR0 : 0x%02X 0x%02X\n", reg[0], reg[1]);

  // Configure the 8-bits Octal RAM memory
  //
  MODIFY_REG(reg[0], ((uint8_t)APS256XX_MR0_LATENCY_TYPE | (uint8_t)APS256XX_MR0_READ_LATENCY_CODE | (uint8_t)APS256XX_MR0_DRIVE_STRENGTH),
             ((uint8_t)(0x00) | // VARIABLE_LATENCY:0x00, FIXED:0x20
              (uint8_t)(DEFAULT_READ_LATENCY_CODE) |
              (uint8_t)CONF_HSPI_DS));

  if (APS256XX_WriteReg(&hxspi, APS256XX_MR0_ADDRESS, reg[0]) != APS256XX_OK)
  {
    return false;
  }

  if (APS256XX_ReadReg(&hxspi, APS256XX_MR0_ADDRESS, reg,
                       (uint32_t)APS256XX_READ_REG_LATENCY((uint32_t)(DEFAULT_READ_LATENCY_CODE))) != APS256XX_OK)
  {
    return false;
  }
  logPrintf("     MR0 : 0x%02X 0x%02X\n", reg[0], reg[1]);


  // Reading the configuration of Mode Register 4
  //
  if (APS256XX_ReadReg(&hxspi, APS256XX_MR4_ADDRESS, reg,
                       (uint32_t)APS256XX_READ_REG_LATENCY((uint32_t)(DEFAULT_READ_LATENCY_CODE))) != APS256XX_OK)
  {
    return false;
  }
  logPrintf("     MR4 : 0x%02X 0x%02X\n", reg[0], reg[1]);

  //  Configure the 8-bits Octal RAM memory
  WRITE_REG(reg[0], ((uint8_t)DEFAULT_WRITE_LATENCY_CODE |
                     (uint8_t)CONF_HSPI_RF |
                     (uint8_t)CONF_HSPI_PASR));

  if (APS256XX_WriteReg(&hxspi, APS256XX_MR4_ADDRESS, reg[0]) != APS256XX_OK)
  {
    return false;
  }
  if (APS256XX_ReadReg(&hxspi, APS256XX_MR4_ADDRESS, reg,
                       (uint32_t)APS256XX_READ_REG_LATENCY((uint32_t)(DEFAULT_READ_LATENCY_CODE))) != APS256XX_OK)
  {
    return false;
  }
  logPrintf("     MR4 : 0x%02X 0x%02X\n", reg[0], reg[1]);


  // Reading the configuration of Mode Register 8
  //
  if (APS256XX_ReadReg(&hxspi, APS256XX_MR8_ADDRESS, reg,
                       (uint32_t)APS256XX_READ_REG_LATENCY((uint32_t)(DEFAULT_READ_LATENCY_CODE))) != APS256XX_OK)
  {
    return false;
  }
  logPrintf("     MR8 : 0x%02X 0x%02X\n", reg[0], reg[1]);


  return true;
}

bool psramEnterMemoyMaped(void)
{
  if (APS256XX_EnableMemoryMappedMode(&hxspi,
                                      (uint32_t)APS256XX_READ_LATENCY(
                                      (uint32_t)(DEFAULT_READ_LATENCY_CODE),
                                      (uint32_t)(0)),
                                      (uint32_t)APS256XX_WRITE_LATENCY(
                                      (uint32_t)(DEFAULT_WRITE_LATENCY_CODE)),
                                      (uint32_t)(0), 0U) != APS256XX_OK)
  {
    return false;
  }

  return true;
}

bool psramRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  if (APS256XX_Read(&hxspi, p_data, addr, length,
                    (uint32_t)APS256XX_READ_LATENCY((uint32_t)(DEFAULT_READ_LATENCY_CODE),
                                                    (uint32_t)(0x00)),
                    (uint32_t)(0), 0U) != APS256XX_OK)
  {
    return false;
  }

  return true;;
}

bool psramWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  if (APS256XX_Write(&hxspi, p_data, addr, length,
                     (uint32_t)APS256XX_WRITE_LATENCY((uint32_t)(DEFAULT_WRITE_LATENCY_CODE)),
                     (uint32_t)(0x00), 0U) != APS256XX_OK)
  {
    return false;
  }

  return true;
}

#ifdef _USE_HW_CLI
void cliPsram(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    cliPrintf("PSRAM Add : 0x%X\n", psram_addr);
    cliPrintf("PSRAM Len : %dKB\n", psram_length/1024);

    for (int i=0; i<PSRAM_MAX_CH; i++)
    {
      cliPrintf("PSRAM CH%d, Init: %d, 0x%X, Len : %dKB\n", i+1,
                  psram_tbl[i].is_init,
                  psram_tbl[i].id,
                  psram_tbl[i].length/1024);
    }

    uint8_t  reg[2];
    uint8_t  reg_tbl[10];
    uint32_t latency;
     
    latency = APS256XX_READ_REG_LATENCY((uint32_t)(DEFAULT_READ_LATENCY_CODE));

    APS256XX_ReadReg(&hxspi, APS256XX_MR0_ADDRESS, reg, latency);
    reg_tbl[0] = reg[0];
    logPrintf("MR0 : 0x%02X 0x%02X\n", reg[0], reg[1]);

    APS256XX_ReadReg(&hxspi, APS256XX_MR1_ADDRESS, reg, latency);
    reg_tbl[1] = reg[0];
    logPrintf("MR1 : 0x%02X 0x%02X\n", reg[0], reg[1]);

    APS256XX_ReadReg(&hxspi, APS256XX_MR2_ADDRESS, reg, latency);
    reg_tbl[2] = reg[0];
    logPrintf("MR2 : 0x%02X 0x%02X\n", reg[0], reg[1]);

    APS256XX_ReadReg(&hxspi, APS256XX_MR3_ADDRESS, reg, latency);
    reg_tbl[3] = reg[0];
    logPrintf("MR3 : 0x%02X 0x%02X\n", reg[0], reg[1]);

    APS256XX_ReadReg(&hxspi, APS256XX_MR4_ADDRESS, reg, latency);
    reg_tbl[4] = reg[0];
    logPrintf("MR4 : 0x%02X 0x%02X\n", reg[0], reg[1]);

    APS256XX_ReadReg(&hxspi, APS256XX_MR6_ADDRESS, reg, latency);
    reg_tbl[6] = reg[0];
    logPrintf("MR6 : 0x%02X 0x%02X\n", reg[0], reg[1]);

    APS256XX_ReadReg(&hxspi, APS256XX_MR8_ADDRESS, reg, latency);
    reg_tbl[8] = reg[0];
    logPrintf("MR8 : 0x%02X 0x%02X\n", reg[0], reg[1]);

    cliPrintf("Read Latency Code  : %d\n", ((reg_tbl[0]>>2) & 0x7) + 3);

    const uint8_t wr_latency_tbl[] = {3, 7, 5, 0, 4, 0, 6};
    cliPrintf("Write Latency Code : %d\n",wr_latency_tbl[((reg_tbl[4]>>5) & 0x7)]);
    cliPrintf("RBX : %d\n", (reg_tbl[8] & (1<<3)) ? 1:0);

    const uint16_t density_tbl[] = {
      [5]=128,
      [7]=256,
      [6]=512};
    cliPrintf("Density : %d MB\n",density_tbl[((reg_tbl[2]>>0) & 0x7)]);

    ret = true;
  }
  
  if (args->argc == 1 && args->isStr(0, "test") == true)
  {
    uint32_t w_data[128];
    uint32_t r_data[128];

    for (int i=0; i<128; i++)
    {
      w_data[i] = i;
      r_data[i] = 0;
    }

    if (psramWrite(0, (uint8_t *)&w_data, 128*sizeof(uint32_t)) == true)
    {
      cliPrintf("PSRAM Write OK\n");
    }
    else
    {
      cliPrintf("PSRAM Write Fail\n");
    }

    if (psramRead(0, (uint8_t *)&r_data, 128*sizeof(uint32_t)) == true)
    {
      cliPrintf("PSRAM Read OK\n");
    }
    else
    {
      cliPrintf("PSRAM Read Fail\n");
    }

    for (int i=0; i<128; i++)
    {
      cliPrintf("Data %d:%d \n", w_data[i], r_data[i]);

      if (w_data[i] != r_data[i])
      {
        cliPrintf("Fail \n");
      }
    }

    ret = true;
  }
  
  if (args->argc == 2 && args->isStr(0, "test-xip") == true)
  {
    uint32_t *p_data = (uint32_t *)psram_addr;
    uint8_t length_mb;
    uint32_t i;
    uint32_t pre_time;
    uint32_t test_length;
    uint32_t speed;


    length_mb = (uint8_t)args->getData(1);
    test_length = length_mb * 1024*1024;

    cliPrintf("Length.. %d MB\n", length_mb);

    pre_time = millis();
    for (i=0; i<test_length/4; i++)
    {
      p_data[i] = i;
    }
    speed = (test_length * 1000 / (millis()-pre_time)) / 1024;
    cliPrintf( "Write..  %d.%dMB/s\n", speed/1000, speed%1000);

    volatile uint32_t data_sum = 0;
    pre_time = millis();
    for (i=0; i<test_length/4; i++)
    {
      data_sum += p_data[i];
    }
    speed = (test_length * 1000 / (millis()-pre_time)) / 1024;
    cliPrintf( "Read..   %d.%dMB/s\n", speed/1000, speed%1000);


    bool verify_ret = true;

    cliPrintf("Verify.. ");
    for (i=0; i<test_length/4; i++)
    {
      if (p_data[i] != i)
      {
        verify_ret = false;
        break;
      }
    }
    cliPrintf("%s\n", verify_ret ? "OK":"FAIL");
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "test-xip-byte") == true)
  {
    uint8_t *p_data = (uint8_t *)psram_addr;
    uint8_t length_mb;
    uint32_t i;
    uint32_t pre_time;
    uint32_t test_length;
    uint32_t speed;


    length_mb = (uint8_t)args->getData(1);
    test_length = length_mb * 1024 * 1024;

    cliPrintf("Length.. %d MB\n", length_mb);

    pre_time = millis();
    for (i=0; i<test_length; i++)
    {
      p_data[i] = i;
    }
    speed = (test_length * 1000 / (millis()-pre_time)) / 1024;
    cliPrintf( "Write..  %d.%dMB/s\n", speed/1000, speed%1000);

    volatile uint32_t data_sum = 0;
    pre_time = millis();
    for (i=0; i<test_length; i++)
    {
      data_sum += p_data[i];
    }
    speed = (test_length * 1000 / (millis()-pre_time)) / 1024;
    cliPrintf( "Read..   %d.%dMB/s\n", speed/1000, speed%1000);

    bool verify_ret = true;

    cliPrintf("Verify.. ");
    for (i=0; i<test_length; i++)
    {
      if (p_data[i] != (uint8_t)i)
      {
        verify_ret = false;
        break;
      }
    }
    cliPrintf("%s\n", verify_ret ? "OK":"FAIL");
    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "test-xip-read") == true)
  {
    uint32_t *p_data;
    uint8_t length_mb;
    uint32_t i;
    uint32_t pre_time;
    uint32_t test_length;
    uint32_t speed;


    p_data = (uint32_t *)args->getData(1);
    length_mb = (uint8_t)args->getData(2);
    test_length = length_mb * 1024 * 1024;

    cliPrintf("Addr..   0x%X\n", args->getData(1));
    cliPrintf("Length.. %d MB\n", length_mb);


    volatile uint32_t data_sum = 0;
    pre_time = millis();
    for (i=0; i<test_length/4; i++)
    {
      data_sum += p_data[i];
    }
    speed = (test_length * 1000 / (millis()-pre_time)) / 1024;
    cliPrintf( "Read..   %d.%dMB/s\n", speed/1000, speed%1000);

    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "read"))
  {
    uint32_t addr;
    uint32_t length;
    uint8_t  data;
    bool psram_ret;

    addr   = (uint32_t)args->getData(1);
    length = (uint32_t)args->getData(2);

    for (int i=0; i<length; i++)
    {
      psram_ret = psramRead(addr+i, &data, 1);

      if (psram_ret == true)
      {
        cliPrintf( "addr : 0x%X\t 0x%02X\n", addr+i, data);
      }
      else
      {
        cliPrintf( "addr : 0x%X\t Fail\n", addr+i);
      }
    }
    ret = true;
  }
  
  if(args->argc == 3 && args->isStr(0, "write") == true)
  {
    uint32_t addr;
    uint32_t data;
    bool psram_ret;

    addr = (uint32_t)args->getData(1);
    data = (uint32_t )args->getData(2);

    psram_ret = psramWrite(addr, (uint8_t *)&data, 4);

    cliPrintf( "addr : 0x%X\t 0x%X\n", addr, data);
    if (psram_ret)
    {
      cliPrintf("OK\n");
    }
    else
    {
      cliPrintf("FAIL\n");
    }
    ret = true;
  }


  if (ret == false)
  {
    cliPrintf("psram info \n");
    cliPrintf("psram test \n");
    cliPrintf("psram test-xip [size MB]\n");
    cliPrintf("psram test-xip-byte [size MB]\n");
    cliPrintf("psram test-xip-read [addr] [size MB]\n");
    cliPrintf("psram read  [addr] [length]\n");
    cliPrintf("psram write [addr] [data]\n");    
  }
}
#endif

#endif
