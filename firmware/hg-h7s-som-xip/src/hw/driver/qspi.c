#include "qspi.h"



#ifdef _USE_HW_QSPI
#include "qspi/mx25lm25645g.h"
#include "cli.h"




/* QSPI Error codes */
#define QSPI_OK             ((uint8_t)0x00)
#define QSPI_ERROR          ((uint8_t)0x01)
#define QSPI_BUSY           ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED  ((uint8_t)0x04)
#define QSPI_SUSPENDED      ((uint8_t)0x08)



/* QSPI Base Address */
#define QSPI_BASE_ADDRESS   0x70000000
#define QSPI_CMD_TIMEOUT    100



/* QSPI Info */
typedef struct {
  uint32_t FlashSize;          /*!< Size of the flash */
  uint32_t EraseSectorSize;    /*!< Size of sectors for the erase operation */
  uint32_t EraseSectorsNumber; /*!< Number of sectors for the erase operation */
  uint32_t ProgPageSize;       /*!< Size of pages for the program operation */
  uint32_t ProgPagesNumber;    /*!< Number of pages for the program operation */

  uint8_t  device_id[20];
} QSPI_Info;


static bool is_init = false;
static XSPI_HandleTypeDef hqspi;


uint8_t BSP_QSPI_Init(void);
uint8_t BSP_QSPI_DeInit(void);
uint8_t BSP_QSPI_Read(uint8_t* p_data, uint32_t addr, uint32_t length);
uint8_t BSP_QSPI_Write(uint8_t* p_data, uint32_t addr, uint32_t length);
uint8_t BSP_QSPI_Erase_Block(uint32_t block_addr);
uint8_t BSP_QSPI_Erase_Sector(uint32_t SectorAddress);
uint8_t BSP_QSPI_Erase_Chip (void);
uint8_t BSP_QSPI_GetStatus(void);
uint8_t BSP_QSPI_GetInfo(QSPI_Info* p_info);
uint8_t BSP_QSPI_EnableMemoryMappedMode(void);
uint8_t BSP_QSPI_GetID(QSPI_Info* p_info);
uint8_t BSP_QSPI_Config(void);
uint8_t BSP_QSPI_Reset(void);
uint8_t BSP_QSPI_Abort(void);

#if CLI_USE(HW_QSPI)
static void cliCmd(cli_args_t *args);
#endif






bool qspiInit(void)
{
  bool ret = true;
  QSPI_Info info;


  logPrintf("[  ] qspiInit()\n");

  if (BSP_QSPI_Init() == QSPI_OK)
  {
    ret = true;
  }
  else
  {
    ret = false;
  }


  if (BSP_QSPI_GetID(&info) == QSPI_OK)
  {
    if (info.device_id[0] == 0xC2 && info.device_id[2] == 0x85 && info.device_id[4] == 0x39)
    {
      uint32_t qspi_clk;

      qspi_clk = HAL_RCC_GetPLL2SFreq() / (hqspi.Init.ClockPrescaler + 1);

      logPrintf("[OK] qspiInit()\n");
      logPrintf("     MX25LM25645G Found\r\n");
      logPrintf("     CLK : %d Mhz\r\n", qspi_clk/1000000);
      
      ret = true;
    }
    else
    {
      logPrintf("[E_] qspiInit()\n");
      logPrintf("     MX25LM25645G Not Found %X %X %X\r\n", info.device_id[0], info.device_id[1], info.device_id[2]);
      ret = false;
    }
  }
  else
  {
    logPrintf("[E_] qspiInit()\n");
    ret = false;
  }



  is_init = ret;

#if CLI_USE(HW_QSPI)
  cliAdd("qspi", cliCmd);
#endif
  return ret;
}

bool qspiReset(void)
{
  bool ret = false;

  if (is_init == true)
  {
    if (BSP_QSPI_Reset() == QSPI_OK)
    {
      ret = true;
    }
  }

  return ret;
}

bool qspiAbort(void)
{
  bool ret = false;

  if (is_init == true)
  {
    if (BSP_QSPI_Abort() == QSPI_OK)
    {
      ret = true;
    }
  }

  return ret;
}

bool qspiIsInit(void)
{
  return is_init;
}

bool qspiRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  uint8_t ret;

  if (qspiGetXipMode() == true)
  {
    return false;
  }

  if (addr >= qspiGetLength())
  {
    return false;
  }  

  ret = BSP_QSPI_Read(p_data, addr, length);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  uint8_t ret;


  if (addr >= qspiGetLength())
  {
    return false;
  }

  ret = BSP_QSPI_Write(p_data, addr, length);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiEraseBlock(uint32_t block_addr)
{
  uint8_t ret;

  ret = BSP_QSPI_Erase_Block(block_addr);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiEraseSector(uint32_t sector_addr)
{
  uint8_t ret;

  ret = BSP_QSPI_Erase_Sector(sector_addr);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }  
}
bool qspiErase(uint32_t addr, uint32_t length)
{
  bool ret = true;
  uint32_t flash_length;
  uint32_t block_size;
  uint32_t block_begin;
  uint32_t block_end;
  uint32_t i;


  flash_length = MX25LM25645G_FLASH_SIZE;
  block_size   = MX25LM25645G_SECTOR_SIZE;


  if ((addr > flash_length) || ((addr+length) > flash_length))
  {
    return false;
  }
  if (length == 0)
  {
    return false;
  }


  block_begin = addr / block_size;
  block_end   = (addr + length - 1) / block_size;


  for (i=block_begin; i<=block_end; i++)
  {
    ret = qspiEraseSector(block_size*i);
    if (ret == false)
    {
      break;
    }
  }

  return ret;
}

bool qspiEraseChip(void)
{
  uint8_t ret;

  ret = BSP_QSPI_Erase_Chip();

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiGetStatus(void)
{
  uint8_t ret;

  ret = BSP_QSPI_GetStatus();

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiGetInfo(qspi_info_t* p_info)
{
  uint8_t ret;

  ret = BSP_QSPI_GetInfo((QSPI_Info *)p_info);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiEnableMemoryMappedMode(void)
{
  uint8_t ret;

  ret = BSP_QSPI_EnableMemoryMappedMode();

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiSetXipMode(bool enable)
{
  uint8_t ret = true;

  if (enable)
  {
    if (qspiGetXipMode() == false)
    {
      ret = qspiEnableMemoryMappedMode();
    }
  }
  else
  {
    if (qspiGetXipMode() == true)
    {
      ret = qspiReset();
    }
  }

  return ret;
}

bool qspiGetXipMode(void)
{
  bool ret = false;

  if (HAL_XSPI_GetState(&hqspi) == HAL_XSPI_STATE_BUSY_MEM_MAPPED)
  {
    ret = true;
  }

  return ret;
}

uint32_t qspiGetAddr(void)
{
  return QSPI_BASE_ADDRESS;
}

uint32_t qspiGetLength(void)
{
  return MX25LM25645G_FLASH_SIZE;
}






static uint8_t QSPI_ResetMemory(XSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_WriteEnable(XSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_WriteEnable_OPI(XSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_AutoPollingMemReady(XSPI_HandleTypeDef *hqspi, uint32_t Timeout);
static uint8_t QSPI_AutoPollingMemReady_OPI(XSPI_HandleTypeDef *hqspi, uint32_t Timeout);
// static uint8_t QSPI_SPI_RDCR2(XSPI_HandleTypeDef *hqspi, uint32_t addr, uint8_t *p_data);
static uint8_t QSPI_SPI_WRCR2(XSPI_HandleTypeDef *hqspi, uint32_t addr, uint8_t data);

static uint8_t QSPI_OPI_RDCR2(XSPI_HandleTypeDef *hqspi, uint32_t addr, uint8_t *p_data);
static uint8_t QSPI_OPI_WRCR2(XSPI_HandleTypeDef *hqspi, uint32_t addr, uint8_t data);


uint8_t BSP_QSPI_Init(void)
{
  XSPIM_CfgTypeDef sOspiManagerCfg = {0};


  hqspi.Instance = XSPI2;

  /* Call the DeInit function to reset the driver */
  if (HAL_XSPI_DeInit(&hqspi) != HAL_OK)
  {
    return QSPI_ERROR;
  }


  /* QSPI initialization */
  /* ClockPrescaler set to 1, so QSPI clock = 200MHz / (2) = 100MHz */
  hqspi.Init.FifoThresholdByte       = 4;
  hqspi.Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  hqspi.Init.MemoryType              = HAL_XSPI_MEMTYPE_MACRONIX;
  hqspi.Init.MemorySize              = HAL_XSPI_SIZE_256MB;
  hqspi.Init.ChipSelectHighTimeCycle = 5;
  hqspi.Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  hqspi.Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  hqspi.Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hqspi.Init.ClockPrescaler          = 1;
  hqspi.Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hqspi.Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_DISABLE;
  hqspi.Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_NONE;
  hqspi.Init.MaxTran                 = 0;
  hqspi.Init.Refresh                 = 0;
  hqspi.Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  if (HAL_XSPI_Init(&hqspi) != HAL_OK)
  {
    logPrintf("HAL_XSPI_Init() fail\n");
    return QSPI_ERROR;
  }

  sOspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sOspiManagerCfg.IOPort      = HAL_XSPIM_IOPORT_2;
  sOspiManagerCfg.Req2AckTime = 1;
  if (HAL_XSPIM_Config(&hqspi, &sOspiManagerCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }


  //-- Hardware Reset
  //
  GPIO_InitTypeDef   GPIO_InitStructure;


  __HAL_RCC_GPION_CLK_ENABLE();

  GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull  = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.Pin   = GPIO_PIN_7;
  HAL_GPIO_Init(GPION, &GPIO_InitStructure);
  HAL_GPIO_WritePin(GPION, GPIO_PIN_7, GPIO_PIN_RESET);
  delay(5);
  HAL_GPIO_WritePin(GPION, GPIO_PIN_7, GPIO_PIN_SET);
  delay(5);


  //-- QSPI memory reset
  //
  if (QSPI_ResetMemory(&hqspi) != QSPI_OK)
  {
    logPrintf("QSPI_ResetMemory() fail\n");
    return QSPI_NOT_SUPPORTED;
  }

  if (BSP_QSPI_Config() != QSPI_OK)
  {
    logPrintf("QSPI_Config() fail\n");
    return QSPI_NOT_SUPPORTED;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Reset(void)
{
  if (QSPI_ResetMemory(&hqspi) != QSPI_OK)
  {
    return QSPI_NOT_SUPPORTED;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Abort(void)
{
  if (HAL_XSPI_Abort(&hqspi) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Config(void)
{
  uint8_t reg_cfg;
  uint8_t ret;


  // Set Dummy Cycle to 12
  //
  if (QSPI_SPI_WRCR2(&hqspi, 0x300, 0x04) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  // Enter OPI Mode
  //
  if (QSPI_SPI_WRCR2(&hqspi, 0, 0x02) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  reg_cfg = 0;

  ret = QSPI_OPI_RDCR2(&hqspi, 0, &reg_cfg);
  logPrintf("     OPI 0x%X : 0x%02X %s\n", 0, reg_cfg, ret==QSPI_OK ? "OK":"FAIL");

  return QSPI_OK;
}

uint8_t BSP_QSPI_DeInit(void)
{
  hqspi.Instance = XSPI2;

  /* Call the DeInit function to reset the driver */
  if (HAL_XSPI_DeInit(&hqspi) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Read(uint8_t* p_data, uint32_t addr, uint32_t length)
{
  XSPI_RegularCmdTypeDef s_command = {0};


  /* Initialize the read command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = 0xEE11;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;

  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;

  s_command.DataMode           = HAL_XSPI_DATA_8_LINES;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = MX25LM25645G_DUMMY_CYCLES_READ_OPI;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;

  s_command.Address            = addr;
  s_command.DataLength         = length;

  /* Send the command */
  if (HAL_XSPI_Command(&hqspi, &s_command, QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_XSPI_Receive(&hqspi, p_data, QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Write(uint8_t* p_data, uint32_t addr, uint32_t length)
{
  XSPI_RegularCmdTypeDef s_command = {0};
  uint32_t end_addr, current_size, current_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_size = MX25LM25645G_PAGE_SIZE - (addr % MX25LM25645G_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > length)
  {
    current_size = length;
  }

  /* Initialize the adress variables */
  current_addr = addr;
  end_addr = addr + length;

  /* Initialize the program command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = 0x12ED;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_8_LINES;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;

  /* Perform the write page by page */
  do
  {
    s_command.Address    = current_addr;
    s_command.DataLength = current_size;

    /* Enable write operations */
    if (QSPI_WriteEnable_OPI(&hqspi) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure the command */
    if (HAL_XSPI_Command(&hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* Transmission of the data */
    if (HAL_XSPI_Transmit(&hqspi, p_data, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure automatic polling mode to wait for end of program */
    if (QSPI_AutoPollingMemReady_OPI(&hqspi, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    p_data += current_size;
    current_size = ((current_addr + MX25LM25645G_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MX25LM25645G_PAGE_SIZE;
  } while (current_addr < end_addr);

  return QSPI_OK;
}

uint8_t BSP_QSPI_Erase_Block(uint32_t BlockAddress)
{
  XSPI_RegularCmdTypeDef s_command = {0};


  /* Initialize the erase command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = 0x21DE;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  s_command.Address            = BlockAddress;

  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_NONE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;


  /* Enable write operations */
  if (QSPI_WriteEnable_OPI(&hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_XSPI_Command(&hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady_OPI(&hqspi, MX25LM25645G_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Erase_Sector(uint32_t SectorAddress)
{
  XSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the erase command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = 0xDC23;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  s_command.Address            = SectorAddress;

  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_NONE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;


  /* Enable write operations */
  if (QSPI_WriteEnable_OPI(&hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_XSPI_Command(&hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady_OPI(&hqspi, MX25LM25645G_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Erase_Chip(void)
{
  XSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the erase command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = BULK_ERASE_CMD;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_NONE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_DISABLE;

  /* Enable write operations */
  if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_XSPI_Command(&hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(&hqspi, MX25LM25645G_BULK_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_GetStatus(void)
{
  XSPI_RegularCmdTypeDef s_command = {0};
  uint8_t reg;

  /* Initialize the read flag status register command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = READ_FLAG_STATUS_REG_CMD;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_1_LINE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_DISABLE;

  s_command.DataLength         = 1;

  /* Configure the command */
  if (HAL_XSPI_Command(&hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_XSPI_Receive(&hqspi, &reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Check the value of the register */
  if ((reg & (MX25LM25645G_FSR_PRERR | MX25LM25645G_FSR_VPPERR | MX25LM25645G_FSR_PGERR | MX25LM25645G_FSR_ERERR)) != 0)
  {
    return QSPI_ERROR;
  }
  else if ((reg & (MX25LM25645G_FSR_PGSUS | MX25LM25645G_FSR_ERSUS)) != 0)
  {
    return QSPI_SUSPENDED;
  }
  else if ((reg & MX25LM25645G_FSR_READY) != 0)
  {
    return QSPI_OK;
  }
  else
  {
    return QSPI_BUSY;
  }
}

uint8_t BSP_QSPI_GetID(QSPI_Info* p_info)
{
  XSPI_RegularCmdTypeDef s_command = {0};


  /* Initialize the read flag status register command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = 0x9F60;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  s_command.Address            = 0;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_8_LINES;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = MX25LM25645G_DUMMY_CYCLES_READ_OPI;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;

  s_command.DataLength         = 20;
  
  /* Configure the command */
  if (HAL_XSPI_Command(&hqspi, &s_command, QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_XSPI_Receive(&hqspi, p_info->device_id, QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_GetInfo(QSPI_Info* p_info)
{
  /* Configure the structure with the memory configuration */
  p_info->FlashSize          = MX25LM25645G_FLASH_SIZE;
  p_info->EraseSectorSize    = MX25LM25645G_SUBSECTOR_SIZE;
  p_info->EraseSectorsNumber = (MX25LM25645G_FLASH_SIZE/MX25LM25645G_SUBSECTOR_SIZE);
  p_info->ProgPageSize       = MX25LM25645G_PAGE_SIZE;
  p_info->ProgPagesNumber    = (MX25LM25645G_FLASH_SIZE/MX25LM25645G_PAGE_SIZE);

  return QSPI_OK;
}

uint8_t BSP_QSPI_EnableMemoryMappedMode(void)
{
  XSPI_RegularCmdTypeDef   s_command = {0};
  XSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

  /* Configure the command for the read instruction */
  s_command.OperationType      = HAL_XSPI_OPTYPE_READ_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_8_LINES;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;


  /* Configure the command for read */
  s_command.OperationType      = HAL_XSPI_OPTYPE_READ_CFG;
  s_command.Instruction        = 0xEE11;
  s_command.DummyCycles        = MX25LM25645G_DUMMY_CYCLES_READ_OPI;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(&hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  hqspi.State = HAL_XSPI_STATE_CMD_CFG;

  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation  = HAL_XSPI_TIMEOUT_COUNTER_ENABLE;
  s_mem_mapped_cfg.TimeoutPeriodClock = 0x20;
  if (HAL_XSPI_MemoryMapped(&hqspi, &s_mem_mapped_cfg) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

static uint8_t QSPI_ResetMemory(XSPI_HandleTypeDef *p_hqspi)
{
  XSPI_RegularCmdTypeDef s_command = {0};


  if (HAL_XSPI_GetState(&hqspi) != HAL_XSPI_STATE_READY)
  {
    HAL_XSPI_Abort(p_hqspi);
  }

  /* Initialize the reset enable command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.Instruction        = RESET_ENABLE_CMD;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;

  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_NONE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_DISABLE;

  /* Send the command */
  if (HAL_XSPI_Command(p_hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the reset memory command */
  s_command.Instruction = RESET_MEMORY_CMD;
  if (HAL_XSPI_Command(p_hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  if (QSPI_AutoPollingMemReady(p_hqspi, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

static uint8_t QSPI_WriteEnable(XSPI_HandleTypeDef *p_hqspi)
{
  XSPI_RegularCmdTypeDef  s_command = {0};
  XSPI_AutoPollingTypeDef s_config = {0};

  /* Enable write operations */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = WRITE_ENABLE_CMD;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_NONE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_DISABLE;
  s_command.DataLength         = 0;


  if (HAL_XSPI_Command(p_hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  s_command.Instruction    = READ_STATUS_REG_CMD;
  s_command.DataMode       = HAL_XSPI_DATA_1_LINE;
  s_command.DataLength     = 1;
  if (HAL_XSPI_Command(p_hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.MatchValue      = MX25LM25645G_SR_WREN;
  s_config.MatchMask       = MX25LM25645G_SR_WREN;
  s_config.MatchMode       = HAL_XSPI_MATCH_MODE_AND;
  s_config.IntervalTime    = 0x10;
  s_config.AutomaticStop   = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_XSPI_AutoPolling(p_hqspi, &s_config, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

static uint8_t QSPI_WriteEnable_OPI(XSPI_HandleTypeDef *p_hqspi)
{
  XSPI_RegularCmdTypeDef  s_command = {0};
  XSPI_AutoPollingTypeDef s_config = {0};

  /* Enable write operations */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = 0x06F9;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_NONE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;
  s_command.DataLength         = 0;


  if (HAL_XSPI_Command(p_hqspi, &s_command, QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  s_command.Instruction    = 0x05FA;
  s_command.DataMode       = HAL_XSPI_DATA_8_LINES;
  s_command.DummyCycles    = MX25LM25645G_DUMMY_CYCLES_READ_OPI;
  s_command.DataLength     = 1;
  if (HAL_XSPI_Command(p_hqspi, &s_command, QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.MatchValue      = MX25LM25645G_SR_WREN;
  s_config.MatchMask       = MX25LM25645G_SR_WREN;
  s_config.MatchMode       = HAL_XSPI_MATCH_MODE_AND;
  s_config.IntervalTime    = 0x10;
  s_config.AutomaticStop   = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_XSPI_AutoPolling(p_hqspi, &s_config, QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

static uint8_t QSPI_AutoPollingMemReady(XSPI_HandleTypeDef *p_hqspi, uint32_t Timeout)
{
  XSPI_RegularCmdTypeDef  s_command = {0};
  XSPI_AutoPollingTypeDef s_config = {0};

  /* Configure automatic polling mode to wait for memory ready */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.Instruction        = READ_STATUS_REG_CMD;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_1_LINE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_DISABLE;
  s_command.DataLength         = 1;


  if (HAL_XSPI_Command(p_hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return false;
  }

  s_config.MatchValue      = 0;
  s_config.MatchMask       = MX25LM25645G_SR_WIP;
  s_config.MatchMode       = HAL_XSPI_MATCH_MODE_AND;
  s_config.IntervalTime    = 0x10;
  s_config.AutomaticStop   = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_XSPI_AutoPolling(p_hqspi, &s_config, Timeout) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

static uint8_t QSPI_AutoPollingMemReady_OPI(XSPI_HandleTypeDef *p_hqspi, uint32_t Timeout)
{
  XSPI_RegularCmdTypeDef  s_command = {0};
  XSPI_AutoPollingTypeDef s_config = {0};

  /* Configure automatic polling mode to wait for memory ready */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.Instruction        = 0x05FA;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_8_LINES;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = MX25LM25645G_DUMMY_CYCLES_READ_OPI;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;
  s_command.DataLength         = 1;


  if (HAL_XSPI_Command(p_hqspi, &s_command, QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return false;
  }

  s_config.MatchValue      = 0;
  s_config.MatchMask       = MX25LM25645G_SR_WIP;
  s_config.MatchMode       = HAL_XSPI_MATCH_MODE_AND;
  s_config.IntervalTime    = 0x10;
  s_config.AutomaticStop   = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_XSPI_AutoPolling(p_hqspi, &s_config, Timeout) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

#if 0
static uint8_t QSPI_SPI_RDCR2(XSPI_HandleTypeDef *p_hqspi, uint32_t addr, uint8_t *p_data)
{
  XSPI_RegularCmdTypeDef s_command = {0};
  uint8_t reg;

  /* Initialize the read flag status register command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = CMD_SPI_RDCR2;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  s_command.Address            = addr;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_1_LINE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_DISABLE;
  s_command.DataLength         = 1;


  /* Configure the command */
  if (HAL_XSPI_Command(p_hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_XSPI_Receive(p_hqspi, &reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  *p_data = reg;

  return QSPI_OK;
}
#endif

static uint8_t QSPI_OPI_RDCR2(XSPI_HandleTypeDef *p_hqspi, uint32_t addr, uint8_t *p_data)
{
  XSPI_RegularCmdTypeDef s_command = {0};


  /* Initialize the read flag status register command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = CMD_OPI_RDCR2;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  s_command.Address            = addr;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_8_LINES;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = MX25LM25645G_DUMMY_CYCLES_READ_OPI;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;
  s_command.DataLength         = 2;


  /* Configure the command */
  if (HAL_XSPI_Command(p_hqspi, &s_command, QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  uint8_t rd_buf[2];
  
  /* Reception of the data */
  if (HAL_XSPI_Receive(p_hqspi, &rd_buf[0], QSPI_CMD_TIMEOUT) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  *p_data = rd_buf[0];

  return QSPI_OK;
}

static uint8_t QSPI_SPI_WRCR2(XSPI_HandleTypeDef *p_hqspi, uint32_t addr, uint8_t data)
{
  XSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the program command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = CMD_SPI_WRCR2;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  s_command.Address            = addr;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_1_LINE;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_DISABLE;
  s_command.DataLength         = 1;

  /* Enable write operations */
  if (QSPI_WriteEnable(p_hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure the command */
  if (HAL_XSPI_Command(p_hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Transmission of the data */
  if (HAL_XSPI_Transmit(p_hqspi, &data, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of program */
  if (QSPI_AutoPollingMemReady(p_hqspi, QSPI_CMD_TIMEOUT) != QSPI_OK)
  {
    return QSPI_ERROR;
  }


  return QSPI_OK;
}

static uint8_t QSPI_OPI_WRCR2(XSPI_HandleTypeDef *p_hqspi, uint32_t addr, uint8_t data)
{
  XSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the program command */
  s_command.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  s_command.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  s_command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  s_command.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  s_command.Instruction        = CMD_OPI_WRCR2;
  
  s_command.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  s_command.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  s_command.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  s_command.Address            = addr;
  
  s_command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  
  s_command.DataMode           = HAL_XSPI_DATA_8_LINES;
  s_command.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_XSPI_DQS_ENABLE;
  s_command.DataLength         = 1;

  /* Enable write operations */
  if (QSPI_WriteEnable_OPI(p_hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure the command */
  if (HAL_XSPI_Command(p_hqspi, &s_command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Transmission of the data */
  if (HAL_XSPI_Transmit(p_hqspi, &data, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of program */
  if (QSPI_AutoPollingMemReady_OPI(p_hqspi, QSPI_CMD_TIMEOUT) != QSPI_OK)
  {
    return QSPI_ERROR;
  }


  return QSPI_OK;
}


static uint32_t HAL_RCC_XSPIM_CLK_ENABLED=0;

void HAL_XSPI_MspInit(XSPI_HandleTypeDef* xspiHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if(xspiHandle->Instance==XSPI2)
  {
  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_XSPI2;
    PeriphClkInit.Xspi2ClockSelection = RCC_XSPI2CLKSOURCE_PLL2S;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* XSPI2 clock enable */
    HAL_RCC_XSPIM_CLK_ENABLED++;
    if(HAL_RCC_XSPIM_CLK_ENABLED==1){
      __HAL_RCC_XSPIM_CLK_ENABLE();
    }
    __HAL_RCC_XSPI2_CLK_ENABLE();

    __HAL_RCC_GPION_CLK_ENABLE();
    /**XSPI2 GPIO Configuration
    PN1     ------> XSPIM_P2_NCS1
    PN3     ------> XSPIM_P2_IO1
    PN10     ------> XSPIM_P2_IO6
    PN11     ------> XSPIM_P2_IO7
    PN0     ------> XSPIM_P2_DQS0
    PN9     ------> XSPIM_P2_IO5
    PN2     ------> XSPIM_P2_IO0
    PN6     ------> XSPIM_P2_CLK
    PN4     ------> XSPIM_P2_IO2
    PN5     ------> XSPIM_P2_IO3
    PN8     ------> XSPIM_P2_IO4
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_0|GPIO_PIN_9|GPIO_PIN_2|GPIO_PIN_6
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P2;
    HAL_GPIO_Init(GPION, &GPIO_InitStruct);
  }
}

void HAL_XSPI_MspDeInit(XSPI_HandleTypeDef* xspiHandle)
{

  if(xspiHandle->Instance==XSPI2)
  {
    /* Peripheral clock disable */
    HAL_RCC_XSPIM_CLK_ENABLED--;
    if(HAL_RCC_XSPIM_CLK_ENABLED==0){
      __HAL_RCC_XSPIM_CLK_DISABLE();
    }
    __HAL_RCC_XSPI2_CLK_DISABLE();

    /**XSPI2 GPIO Configuration
    PN1     ------> XSPIM_P2_NCS1
    PN3     ------> XSPIM_P2_IO1
    PN10     ------> XSPIM_P2_IO6
    PN11     ------> XSPIM_P2_IO7
    PN0     ------> XSPIM_P2_DQS0
    PN9     ------> XSPIM_P2_IO5
    PN2     ------> XSPIM_P2_IO0
    PN6     ------> XSPIM_P2_CLK
    PN4     ------> XSPIM_P2_IO2
    PN5     ------> XSPIM_P2_IO3
    PN8     ------> XSPIM_P2_IO4
    */
    HAL_GPIO_DeInit(GPION, GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_0|GPIO_PIN_9|GPIO_PIN_2|GPIO_PIN_6
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8);
  }
}



#if CLI_USE(HW_QSPI)
void cliCmd(cli_args_t *args)
{
  bool ret = false;
  uint32_t i;
  uint32_t addr;
  uint32_t length;
  uint32_t pre_time;
  bool flash_ret;



  if(args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("qspi flash addr  : 0x%X\n", 0);
    cliPrintf("qspi xip   addr  : 0x%X\n", qspiGetAddr());
    cliPrintf("qspi xip   mode  : %s\n", qspiGetXipMode() ? "True":"False");
    cliPrintf("qspi state       : ");

    switch(HAL_XSPI_GetState(&hqspi))
    {
      case HAL_XSPI_STATE_RESET:
        cliPrintf("RESET\n");
        break;
      case HAL_XSPI_STATE_HYPERBUS_INIT:
        cliPrintf("HYPERBUS_INIT\n");
        break;
      case HAL_XSPI_STATE_READY:
        cliPrintf("READY\n");
        break; 
      case HAL_XSPI_STATE_CMD_CFG:
        cliPrintf("CMD_CFG\n");
        break;    
      case HAL_XSPI_STATE_READ_CMD_CFG:
        cliPrintf("READ_CMD_CFG\n");
        break;                                  
      case HAL_XSPI_STATE_WRITE_CMD_CFG:
        cliPrintf("WRITE_CMD_CFG\n");
        break;      
      case HAL_XSPI_STATE_BUSY_CMD:
        cliPrintf("BUSY_CMD\n");
        break;          
      case HAL_XSPI_STATE_BUSY_TX:
        cliPrintf("BUSY_TX\n");
        break;       
      case HAL_XSPI_STATE_BUSY_RX:
        cliPrintf("BUSY_RX\n");
        break;    
      case HAL_XSPI_STATE_BUSY_AUTO_POLLING:
        cliPrintf("BUSY_AUTO_POLLING\n");
        break;          
      case HAL_XSPI_STATE_BUSY_MEM_MAPPED:
        cliPrintf("BUSY_MEM_MAPPED\n");
        break;         
      case HAL_XSPI_STATE_ABORT:
        cliPrintf("ABORT\n");
        break;        
      case HAL_XSPI_STATE_ERROR:
        cliPrintf("ERROR\n");
        break;                                                                                        
      default:
        cliPrintf("UNKWNON\n");
        break;
    }
    ret = true;
  }
  
  if(args->argc == 1 && args->isStr(0, "test"))
  {
    uint8_t rx_buf[256];

    for (int i=0; i<100; i++)
    {
      if (qspiRead(0x1000*i, rx_buf, 256))
      {
        cliPrintf("%d : OK\n", i);
      }
      else
      {
        cliPrintf("%d : FAIL\n", i);
        break;
      }
    }
    ret = true;
  }    

  if (args->argc == 2 && args->isStr(0, "xip"))
  {
    bool xip_enable;

    xip_enable = args->isStr(1, "on") ? true:false;

    if (qspiSetXipMode(xip_enable))
      cliPrintf("qspiSetXipMode() : OK\n");
    else
      cliPrintf("qspiSetXipMode() : Fail\n");
    
    cliPrintf("qspi xip mode  : %s\n", qspiGetXipMode() ? "True":"False");

    ret = true;
  } 

  if (args->argc == 3 && args->isStr(0, "read"))
  {
    uint8_t rd_buf[2];

    addr   = (uint32_t)args->getData(1);
    length = (uint32_t)args->getData(2);

    length = length - (length%2);

    for (i=0; i<length; i += 2)
    {
      flash_ret = qspiRead(addr+i, &rd_buf[0], 2);

      if (flash_ret == true)
      {
        cliPrintf( "addr : 0x%X\t 0x%02X\n", addr+i+0, rd_buf[0]);
        cliPrintf( "addr : 0x%X\t 0x%02X\n", addr+i+1, rd_buf[1]);
      }
      else
      {
        cliPrintf( "addr : 0x%X\t Fail\n", addr+i+0);
        cliPrintf( "addr : 0x%X\t Fail\n", addr+i+1);
      }
    }
    ret = true;
  }
  
  if(args->argc == 3 && args->isStr(0, "erase") == true)
  {
    addr   = (uint32_t)args->getData(1);
    length = (uint32_t)args->getData(2);

    pre_time = millis();
    flash_ret = qspiErase(addr, length);

    cliPrintf( "addr : 0x%X\t len : %d %d ms\n", addr, length, (millis()-pre_time));
    if (flash_ret)
    {
      cliPrintf("OK\n");
    }
    else
    {
      cliPrintf("FAIL\n");
    }
    ret = true;
  }

  if(args->argc == 3 && args->isStr(0, "write") == true)
  {
    uint32_t flash_data;

    addr = (uint32_t)args->getData(1);
    flash_data = (uint32_t )args->getData(2);

    pre_time = millis();
    flash_ret = qspiWrite(addr, (uint8_t *)&flash_data, 4);

    cliPrintf( "addr : 0x%X\t 0x%X %dms\n", addr, flash_data, millis()-pre_time);
    if (flash_ret)
    {
      cliPrintf("OK\n");
    }
    else
    {
      cliPrintf("FAIL\n");
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "speed") == true)
  {
    uint32_t buf[512/4];
    uint32_t cnt;
    uint32_t pre_time;
    uint32_t exe_time;
    uint32_t xip_addr;



    cliPrintf("XIP : %s\n", qspiGetXipMode() ? "ON":"OFF");

    xip_addr = qspiGetAddr();
    cnt = 1024*1024 / 512;
    pre_time = millis();
    for (int i=0; i<cnt; i++)
    {
      if (qspiGetXipMode())
      {
        memcpy(buf, (void *)(xip_addr + i*512), 512);
      }
      else
      {
        if (qspiRead(i*512, (uint8_t *)buf, 512) == false)
        {
          cliPrintf("qspiRead() Fail:%d\n", i);
          break;
        }
      }
    }
    exe_time = millis()-pre_time;
    if (exe_time > 0)
    {
      cliPrintf("%d KB/sec\n", 1024 * 1000 / exe_time);
    }
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "mode"))
  {
    if (args->isStr(1, "0"))
    {
      QSPI_OPI_WRCR2(&hqspi, 0, 0x00);
    }
    if (args->isStr(1, "1"))
    {
      uint8_t reg_cfg;

      QSPI_SPI_WRCR2(&hqspi, 0, 0x02);
      QSPI_OPI_RDCR2(&hqspi, 0, &reg_cfg);
      logPrintf("     OPI 0x%X : 0x%02X\n", 0, reg_cfg);        
    }

    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "test-xip") == true)
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


  if (ret == false)
  {
    cliPrintf("qspi info\n");
    cliPrintf("qspi xip on:off\n");
    cliPrintf("qspi test\n");
    cliPrintf("qspi speed\n");
    cliPrintf("qspi test-xip [addr] [size MB]\n");
    cliPrintf("qspi read  [addr] [length]\n");
    cliPrintf("qspi erase [addr] [length]\n");
    cliPrintf("qspi write [addr] [data]\n");
    cliPrintf("qspi mode 0:1\n");
  }
}
#endif

#endif