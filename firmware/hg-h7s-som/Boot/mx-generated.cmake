# File automatically-generated by STM32CubeMX - Do not modify

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
	USE_HAL_DRIVER 
	STM32H7S7xx
    $<$<CONFIG:Debug>:DEBUG>
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ./Core/Inc
    ../Drivers/STM32H7RSxx_HAL_Driver/Inc
    ../Drivers/STM32H7RSxx_HAL_Driver/Inc/Legacy
    ../Middlewares/ST/STM32_ExtMem_Manager
    ../Middlewares/ST/STM32_ExtMem_Manager/boot
    ../Middlewares/ST/STM32_ExtMem_Manager/sal
    ../Middlewares/ST/STM32_ExtMem_Manager/nor_sfdp
    ../Middlewares/ST/STM32_ExtMem_Manager/psram
    ../Middlewares/ST/STM32_ExtMem_Manager/sdcard
    ../Middlewares/ST/STM32_ExtMem_Manager/user
    ../Drivers/CMSIS/Device/ST/STM32H7RSxx/Include
    ../Drivers/CMSIS/Include
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ./Core/Src/main.c
    ./Core/Src/extmem_manager.c
    ./Core/Src/stm32h7rsxx_it.c
    ./Core/Src/stm32h7rsxx_hal_msp.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_cortex.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_rcc.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_rcc_ex.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_flash.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_flash_ex.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_gpio.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_dma.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_dma_ex.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_pwr.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_pwr_ex.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_exti.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_xspi.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_tim.c
    ../Drivers/STM32H7RSxx_HAL_Driver/Src/stm32h7rsxx_hal_tim_ex.c
    ./Core/Src/system_stm32h7rsxx.c
    ../Middlewares/ST/STM32_ExtMem_Manager/stm32_extmem.c
    ../Middlewares/ST/STM32_ExtMem_Manager/boot/stm32_boot_xip.c
    ../Middlewares/ST/STM32_ExtMem_Manager/sal/stm32_sal_xspi.c
    ../Middlewares/ST/STM32_ExtMem_Manager/sal/stm32_sal_sd.c
    ../Middlewares/ST/STM32_ExtMem_Manager/nor_sfdp/stm32_sfdp_data.c
    ../Middlewares/ST/STM32_ExtMem_Manager/nor_sfdp/stm32_sfdp_driver.c
    ../Middlewares/ST/STM32_ExtMem_Manager/psram/stm32_psram_driver.c
    ../Middlewares/ST/STM32_ExtMem_Manager/sdcard/stm32_sdcard_driver.c
    ../Middlewares/ST/STM32_ExtMem_Manager/user/stm32_user_driver.c
    ./Core/Src/sysmem.c
    ./Core/Src/syscalls.c
    ./Core/Startup/startup_stm32h7s7xx.s
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME} 
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()