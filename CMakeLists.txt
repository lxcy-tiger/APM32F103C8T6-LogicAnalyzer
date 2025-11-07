cmake_minimum_required(VERSION 3.22)

# 项目名称
set(CMAKE_PROJECT_NAME APM32LogicAnalyzer)

# 设置构建类型（Debug/Release）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# 包含工具链文件（必须在 project() 之前）
include(cmake/gcc-arm-none-eabi.cmake)
# 创建项目
project(${CMAKE_PROJECT_NAME} C ASM)

# 启用 C 和 ASM
enable_language(C ASM)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target: APM32F103C8 (Medium/High Density)")

# 创建可执行文件（最终生成 .elf）
add_executable(${CMAKE_PROJECT_NAME}
        Source/usbd_cdc.c)

# ==============================
# 源文件
# ==============================
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # CMSIS
    Libraries/Device/Geehy/APM32F10x/Source/system_apm32f10x.c
    Libraries/Device/Geehy/APM32F10x/Source/gcc/startup_apm32f10x_hd.S  # 注意：使用 .S（大写）以便预处理

    # StdPeriphDriver
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_adc.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_bakpr.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_can.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_crc.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_dac.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_dbgmcu.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_dma.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_dmc.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_eint.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_smc.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_fmc.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_gpio.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_i2c.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_iwdt.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_misc.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_pmu.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_qspi.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_rcm.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_rtc.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_sci2c.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_sdio.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_spi.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_tmr.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_usart.c
    Libraries/APM32F10x_StdPeriphDriver/src/apm32f10x_wwdt.c

    # Application
    Source/main.c
    Source/apm32f10x_int.c

    # Board
    Boards/Board.c

        # APP/CDC
        Source/usbd_cdc.c
        Source/usbd_descriptor.c
        Source/usb_bsp.c
        Libraries/USB_Device_Lib/Core_Device/Class/CDC/src/usbd_class_cdc.c

        # USB_Protocol (Standard Core)
        Libraries/USB_Device_Lib/Core_Device/Standard/src/usbd_init.c
        Libraries/USB_Device_Lib/Core_Device/Standard/src/usbd_core.c
        Libraries/USB_Device_Lib/Core_Device/Standard/src/usbd_interrupt.c
        Libraries/USB_Device_Lib/Core_Device/Standard/src/usbd_stdReq.c

        # USB_Driver
        Libraries/USB_Device_Lib/Driver/src/drv_usb_device.c
)

# ==============================
# 头文件路径
# ==============================
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        Include
        Boards
        Libraries/CMSIS/Include
        Libraries/Device/Geehy/APM32F10x/Include
        Libraries/APM32F10x_StdPeriphDriver/inc
        Libraries/USB_Device_Lib/Core_Device/Class/CDC/inc
        Libraries/USB_Device_Lib/Core_Device/Standard/inc
        Libraries/USB_Device_Lib/Driver/inc
)

# ==============================
# 宏定义（来自 Keil 工程）
# ==============================
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    APM32F10X_HD
    APM32F103_MINI
    USE_STDPERIPH_DRIVER
)

# ==============================
# 链接脚本（关键！）
# ==============================
# 覆盖 toolchain 中的链接脚本路径
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Device/Geehy/APM32F10x/Source/gcc/gcc_APM32F10xx8.ld)
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -T ${LINKER_SCRIPT}
)

# ==============================
# 链接选项（已在 toolchain 中设置，这里可补充）
# ==============================
# 注意：toolchain 已设置 nano.specs、gc-sections 等，无需重复

# ==============================
# 生成 hex 和 bin（可选）
# ==============================
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMENT "Generating HEX and BIN files"
)

# 导出 compile_commands.json（用于 clangd 等）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# O2优化
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")