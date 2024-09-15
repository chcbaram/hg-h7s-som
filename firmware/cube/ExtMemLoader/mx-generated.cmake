# File automatically-generated by STM32CubeMX - Do not modify

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
	USE_HAL_DRIVER 
	STM32H7S7xx 
	STM32_EXTMEMLOADER_STM32CUBETARGET
    $<$<CONFIG:Debug>:DEBUG>
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ./Core/Inc
    ../Drivers/STM32H7RSxx_HAL_Driver/Inc
    ../Drivers/STM32H7RSxx_HAL_Driver/Inc/Legacy
    ../Middlewares/ST/STM32_ExtMem_Loader/core
    ../Middlewares/ST/STM32_ExtMem_Loader/MDK-ARM
    ../Middlewares/ST/STM32_ExtMem_Loader/STM32Cube
    ../Middlewares/ST/STM32_ExtMem_Manager
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
    ./Core/Src/gpio.c
    ./Core/Src/extmem_manager.c
    ./Core/Src/xspi.c
    ./Core/Src/stm32h7rsxx_hal_msp.c
    ./Core/Src/extmemloader_init.c
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
    ../Middlewares/ST/STM32_ExtMem_Manager/sal/stm32_sal_xspi.c
    ../Middlewares/ST/STM32_ExtMem_Manager/sal/stm32_sal_sd.c
    ../Middlewares/ST/STM32_ExtMem_Manager/nor_sfdp/stm32_sfdp_data.c
    ../Middlewares/ST/STM32_ExtMem_Manager/nor_sfdp/stm32_sfdp_driver.c
    ../Middlewares/ST/STM32_ExtMem_Manager/psram/stm32_psram_driver.c
    ../Middlewares/ST/STM32_ExtMem_Manager/sdcard/stm32_sdcard_driver.c
    ../Middlewares/ST/STM32_ExtMem_Manager/user/stm32_user_driver.c
    ../Middlewares/ST/STM32_ExtMem_Loader/core/memory_wrapper.c
    ../Middlewares/ST/STM32_ExtMem_Loader/core/systick_management.c
    ../Middlewares/ST/STM32_ExtMem_Loader/MDK-ARM/FlashDev.c
    ../Middlewares/ST/STM32_ExtMem_Loader/MDK-ARM/FlashPrg.c
    ../Middlewares/ST/STM32_ExtMem_Loader/STM32Cube/stm32_device_info.c
    ../Middlewares/ST/STM32_ExtMem_Loader/STM32Cube/stm32_loader_api.c
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME} 
)

# Post build commands
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD 
  COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
  COMMAND bash "../postbuild.sh" "${cubeide_cubeprogrammer_path}/ExternalLoader" 
  COMMENT "Executing Post build command")


# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()
