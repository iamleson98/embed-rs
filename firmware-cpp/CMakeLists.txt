cmake_minimum_required(VERSION 3.15)

# Toolchain configuration
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(thermal_monitor_cpp C CXX ASM)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MCU configuration
set(MCU_FLAGS "-mcpu=cortex-m3 -mthumb")

# Compiler flags
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -Wextra -Werror -std=c11 -Os -g3 -ffunction-sections -fdata-sections -fno-common")
set(CMAKE_CXX_FLAGS "${MCU_FLAGS} -Wall -Wextra -Werror -Os -g3 -ffunction-sections -fdata-sections -fno-common -fno-exceptions -fno-rtti")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${MCU_FLAGS} -T ${CMAKE_SOURCE_DIR}/linker.ld -Wl,--gc-sections -Wl,-Map=${CMAKE_BINARY_DIR}/thermal_monitor.map --specs=nosys.specs -nostartfiles -lc -lnosys")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Source files
set(SOURCES
    src/main.cpp
    src/system.cpp
    src/gpio.cpp
    src/uart.cpp
    src/adc.cpp
    src/timer.cpp
    src/startup.c
)

# Executable
add_executable(thermal_monitor.elf ${SOURCES})

# Generate binary and hex files
add_custom_command(TARGET thermal_monitor.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:thermal_monitor.elf> ${CMAKE_BINARY_DIR}/thermal_monitor.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:thermal_monitor.elf> ${CMAKE_BINARY_DIR}/thermal_monitor.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:thermal_monitor.elf>
    COMMENT "Building thermal_monitor.bin and thermal_monitor.hex"
)

# Flash target (optional)
add_custom_target(flash
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program ${CMAKE_BINARY_DIR}/thermal_monitor.bin 0x08000000 verify reset exit"
    DEPENDS thermal_monitor.elf
    COMMENT "Flashing firmware to STM32F103"
)
