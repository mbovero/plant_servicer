include_directories(inc)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Check if inc directory exists
if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/inc)
    message(WARNING "inc directory not found at ${CMAKE_CURRENT_LIST_DIR}/inc")
else()
    message(STATUS "Found inc directory at ${CMAKE_CURRENT_LIST_DIR}/inc")
endif()

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

# Find all source files in src directory
file(GLOB SOURCES "src/*.c")

# Find all header files in inc directory (for IDE support)
file(GLOB HEADERS "inc/*.h")

# Add executable
add_executable(plant_moisture_monitor
    ${SOURCES}
    ${HEADERS}  # Optional: includes headers in project for IDE
)

pico_set_program_name(plant_moisture_monitor "plant_moisture_monitor")
pico_set_program_version(plant_moisture_monitor "0.1")

# Enable USB stdio (disable UART)
pico_enable_stdio_uart(plant_moisture_monitor 0)
pico_enable_stdio_usb(plant_moisture_monitor 1)

# Add the standard library to the build
target_link_libraries(plant_moisture_monitor
    pico_stdlib
)

# Include directories - more robust version
target_include_directories(plant_moisture_monitor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc
    # Add other include paths here if needed
)

# Add any user requested libraries
target_link_libraries(plant_moisture_monitor 
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_adc
    # Add other libraries here
)

# Generate additional output formats
pico_add_extra_outputs(plant_moisture_monitor)