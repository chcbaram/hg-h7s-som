#include "ltdc.h"



#ifdef _USE_HW_LTDC

void MX_LTDC_Init(void);


LTDC_HandleTypeDef hltdc;


bool ltdcInit(void)
{
  logPrintf("[  ] lcd buf celan..\n");

  memset((void *)0x90000000, 0xFF, 800*480*3*2);

  MX_LTDC_Init();

  logPrintf("[OK] ltdcInit()\n");
  return true;
}




/* LTDC init function */
void MX_LTDC_Init(void)
{

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 7;
  hltdc.Init.VerticalSync = 7;
  hltdc.Init.AccumulatedHBP = 57;
  hltdc.Init.AccumulatedVBP = 27;
  hltdc.Init.AccumulatedActiveW = 857;
  hltdc.Init.AccumulatedActiveH = 507;
  hltdc.Init.TotalWidth = 867;
  hltdc.Init.TotalHeigh = 517;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 800;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 480;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0x90000000;
  pLayerCfg.ImageWidth = 800;
  pLayerCfg.ImageHeight = 480;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef* ltdcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(ltdcHandle->Instance==LTDC)
  {

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInit.LtdcClockSelection = RCC_LTDCCLKSOURCE_PLL3R;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* LTDC clock enable */
    __HAL_RCC_LTDC_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**LTDC GPIO Configuration
    PB3(JTDO/TRACESWO)     ------> LTDC_R4
    PF0     ------> LTDC_R2
    PG0     ------> LTDC_R7
    PG1     ------> LTDC_R6
    PA15(JTDI)     ------> LTDC_R5
    PA11     ------> LTDC_B3
    PA9     ------> LTDC_B5
    PA10     ------> LTDC_B4
    PA8     ------> LTDC_B6
    PF8     ------> LTDC_G1
    PF7     ------> LTDC_G0
    PF10     ------> LTDC_R1
    PB12     ------> LTDC_G5
    PA4     ------> LTDC_R3
    PA3     ------> LTDC_DE
    PA7     ------> LTDC_B1
    PB0     ------> LTDC_VSYNC
    PD8     ------> LTDC_R0
    PB10     ------> LTDC_G7
    PB13     ------> LTDC_G4
    PA1     ------> LTDC_G2
    PA5     ------> LTDC_CLK
    PB2     ------> LTDC_B2
    PB11     ------> LTDC_G6
    PA0     ------> LTDC_G3
    PA2     ------> LTDC_B7
    PA6     ------> LTDC_HSYNC
    PF11     ------> LTDC_B0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_12|GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_LTDC;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_LTDC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_LTDC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_11|GPIO_PIN_8|GPIO_PIN_3
                          |GPIO_PIN_7|GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_LTDC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_LTDC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_LTDC;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* LTDC interrupt Init */
    HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
    HAL_NVIC_SetPriority(LTDC_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(LTDC_ER_IRQn);
  }
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* ltdcHandle)
{

  if(ltdcHandle->Instance==LTDC)
  {
    /* Peripheral clock disable */
    __HAL_RCC_LTDC_CLK_DISABLE();

    /**LTDC GPIO Configuration
    PB3(JTDO/TRACESWO)     ------> LTDC_R4
    PF0     ------> LTDC_R2
    PG0     ------> LTDC_R7
    PG1     ------> LTDC_R6
    PA15(JTDI)     ------> LTDC_R5
    PA11     ------> LTDC_B3
    PA9     ------> LTDC_B5
    PA10     ------> LTDC_B4
    PA8     ------> LTDC_B6
    PF8     ------> LTDC_G1
    PF7     ------> LTDC_G0
    PF10     ------> LTDC_R1
    PB12     ------> LTDC_G5
    PA4     ------> LTDC_R3
    PA3     ------> LTDC_DE
    PA7     ------> LTDC_B1
    PB0     ------> LTDC_VSYNC
    PD8     ------> LTDC_R0
    PB10     ------> LTDC_G7
    PB13     ------> LTDC_G4
    PA1     ------> LTDC_G2
    PA5     ------> LTDC_CLK
    PB2     ------> LTDC_B2
    PB11     ------> LTDC_G6
    PA0     ------> LTDC_G3
    PA2     ------> LTDC_B7
    PA6     ------> LTDC_HSYNC
    PF11     ------> LTDC_B0
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_12|GPIO_PIN_0|GPIO_PIN_10
                          |GPIO_PIN_13|GPIO_PIN_2|GPIO_PIN_11);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0|GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_10
                          |GPIO_PIN_11);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0|GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15|GPIO_PIN_11|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_8|GPIO_PIN_4|GPIO_PIN_3|GPIO_PIN_7
                          |GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_0|GPIO_PIN_2
                          |GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8);

    /* LTDC interrupt Deinit */
    HAL_NVIC_DisableIRQ(LTDC_IRQn);
    HAL_NVIC_DisableIRQ(LTDC_ER_IRQn);
  }
}

void LTDC_IRQHandler(void)
{
  HAL_LTDC_IRQHandler(&hltdc);
}

void LTDC_ER_IRQHandler(void)
{
  HAL_LTDC_IRQHandler(&hltdc);
}

#endif