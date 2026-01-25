#此文件从模板自动生成! 请勿更改!
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.28)

# specify cross-compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(AxDrFOC_G431 C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# 编译参数
add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# 汇编预处理
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

# 优化等级
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    add_compile_options(-Os)
else ()
    add_compile_options(-Og -g)
endif ()

# ======================
# STM32Cube 路径（统一）
# ======================
set(CUBE_PATH
        "C:/Users/33576/STM32Cube/Repository/STM32Cube_FW_G4_V1.6.1"
)

# ======================
# 头文件路径
# ======================
include_directories(

        Core/Inc

        ${CUBE_PATH}/Drivers/STM32G4xx_HAL_Driver/Inc
        ${CUBE_PATH}/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy

        ${CUBE_PATH}/Drivers/CMSIS/Device/ST/STM32G4xx/Include
        ${CUBE_PATH}/Drivers/CMSIS/Include

        modules/Inc
)

# 宏定义
add_definitions(
        -DDEBUG
        -DUSE_HAL_DRIVER
        -DSTM32G431xx
)

# ======================
# 源文件
# ======================
file(GLOB_RECURSE SOURCES
        "Core/Src/*.c"
        "Drivers/Src/*.c"
        "modules/Src/*.c"
)

# 链接脚本
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G431RBTX_FLASH.ld)

# 链接参数
add_link_options(
        -Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
        -mcpu=cortex-m4
        -mthumb
        -mthumb-interwork
        -T ${LINKER_SCRIPT}
)

# 可执行文件
add_executable(${PROJECT_NAME}.elf
        ${SOURCES}
        ${LINKER_SCRIPT}
)

# 生成 HEX BIN
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD

        COMMAND ${CMAKE_OBJCOPY}
        -Oihex
        $<TARGET_FILE:${PROJECT_NAME}.elf>
        ${HEX_FILE}

        COMMAND ${CMAKE_OBJCOPY}
        -Obinary
        $<TARGET_FILE:${PROJECT_NAME}.elf>
        ${BIN_FILE}

        COMMENT
        "Building ${HEX_FILE}
Building ${BIN_FILE}"
)
