include_directories(inc)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(plant_moisture_monitor C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(plant_moisture_monitor
    src/plant_moisture_monitor.c
)

pico_set_program_name(plant_moisture_monitor "plant_moisture_monitor")
pico_set_program_version(plant_moisture_monitor "0.1")

# Enable USB stdio (disable UART)
pico_enable_stdio_uart(plant_moisture_monitor 0)
pico_enable_stdio_usb(plant_moisture_monitor 1)


# Include directories
target_include_directories(plant_moisture_monitor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Link all required libraries
target_link_libraries(plant_moisture_monitor
    pico_stdlib
    hardware_adc
    hardware_pio
    hardware_clocks
)

# Generate additional output formats
pico_add_extra_outputs(plant_moisture_monitor)