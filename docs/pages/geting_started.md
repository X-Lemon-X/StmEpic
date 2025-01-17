# Getting started

This page will guide you through the process of setting up the project for STM32 microcontrollers with:

- C++,
- FreeRTOS,
- Cmake
- StmEpic libary
- STM32CubeMX code generation tool
- ccach

## Requirements

### Compiler:

Building the project requires GNU Arm Embedded Toolchain to be installed.

1. Download it from this site [GNU Arm Embedded Toolchain Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) or directly from this [link](https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz)
2. Move the extracted files in some directory for example "$HOME/.local/share/gccarm",
3. Add this in your **_.profile_** file

```bash
# Add the arm-none-aebi to the path
if [ -d "$HOME/.local/share/gccarm/bin" ]; then
    PATH="$HOME/.local/share/gccarm/bin:$PATH"
fi
```

4. restart the pc or run source ~/.profile

### Software:

1. You will most likely need to install the following packages:

```bash
sudo apt-get install -y cmake ninja-build clang ccache dfu-util stlink-tools python3-pip python3-venv
```

2. Install StmCubeIDE you can install it from [official site](https://www.st.com/en/development-tools/stm32cubeide.html).
3. Install StmCubeMX you can install it from [official site](https://www.st.com/en/development-tools/stm32cubemx.html).

# How to add the library to your project

1. Clone the repository to your project directory as submodule:

```bash
git submodule add https://github.com/X-Lemon-X/StmEpic.git StmEpic
git submodule update --init --recursive
```

2. Add the following line to your CMakeLists.txt file:

```cmake
set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "") # set appropriate port for your microcontroller
set(FREERTOS_HEAP 4 CACHE STRING "") # set appropriate heap implementation
set(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")
# set appropriate path to FreeRTOSConfig.h config file
# for more information about the FreeRTOS configuration see StmEpic/examples or below
add_subdirectory(StmEpic)
target_link_libraries( <YOUR_PROEJCT_NAME> PUBLIC  stmepic)
```

**_FreeRTOSConfig.h_**
You can use StmCubeMX to generate the FreeRTOSConfig.h file by enabling FreeRTOS in the project settings. However this would also add FreeRTOS source files to the project whitch we want to avoid.
So you can generate it add only the FreeRTOSConfig.h file to the project then to disable FreeRTOS in the StmCubeMX project settings and generate the project again.

3. Now you can simply include header files.

## How to use the library

The library is designed to be used with the code generation tool STM32CubeMX.

1. Create a new project in STM32CubeMX.
2. In the project settings, set generate code as CMakelist.
3. Include all source files from the library to the project.
4. don't forget to have ARM GCC toolchain installed. You can download it from [here](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm) or get docker image to build the project from [here](https://hub.docker.com/repository/docker/xlemonx/arm-gnu-toolchain).

# Generating the project from scratch

We won't go into many details since there is many guides on how to use STM32CubeMX.
In this example we asume that the you project name is the same as the folder where the project is located.

1. Open the StmCubeMX click File->New Project
2. Select appropriate microcontroller, in this case [for this example] it is **_STM32F446RET_**
3. Configure the peripherals and their pinout in Pinout & Configuration tab
4. Clock configuration can be done in Clock Configuration tab
5. In Project Manager tab set:
   Toolchain/IDE to **_CMake_**
   Project Name to **_project_name_**.
   Project location to **_The path where you repo is_**.
   The CubeMX IDE doesn't allow relative paths, so if you set something incorrectly you might have to copy the project **_\*.ioc_** file in to the project root directory and open direclty from there.Otherwise the files genrated by CubeMX will we genrated in some other location.
6. Save the project and generate the code using the GENERATE CODE button
7. In you repo folder there should be bunch of new folders and files like:

```

- CMakeLists.txt
- cmake/
- Core/
- Drivers/
- project_name.ioc
- startup_stm32xxx.s
  ...

```

8. The project is configured to be built with CMake, However the C++ language is not enabled yet
   so We still have to add our libraries and source files to the project.

## Adding C++ support

1. In cmake/stm32cubemx/CMakeLists.txt find main.c and replace it with main.cpp
2. Create some heder file that will have single function that will be called in the main.cpp file to enter our part of the program. For example:

```cpp
//creat file main_prog.hpp with content:
#pragma once
void main_prog();

//creat file main_prog.cpp with content:
#include "main_prog.hpp"
void main_prog(){
  // Your code here
}
```

2. in main.cpp add the following code:

```cpp
// in include section
#include "main_prog.hpp"

// in main function bofore while(1)
// call the function to enter your part of the program
main_prog();
```

3. Now you can add your source files and libraries to the project. Since the generated fiels and outr code is separated We don't have to wory with StmCubeMX messing with our files.
4. Since the StmEpic library requires interface provided by the HAl library. If you generate minimalistic projecty you might not be able to build it, since not all HAL library components are enabled by default. To fix this you have to enable the HAL library in the **_Core/Inc/stm32f4xx_hal_conf.h_** file (or somethin similar). The file should look like this:

```c
  /* #define HAL_CRYP_MODULE_ENABLED */
/* #define HAL_ADC_MODULE_ENABLED */
#define HAL_CAN_MODULE_ENABLED
/* #define HAL_CRC_MODULE_ENABLED */
/* #define HAL_CAN_LEGACY_MODULE_ENABLED */
/* #define HAL_HASH_MODULE_ENABLED */
#define HAL_I2C_MODULE_ENABLED   // This might need to be enabled manually
/* #define HAL_I2S_MODULE_ENABLED */
/* #define HAL_IWDG_MODULE_ENABLED */
/* #define HAL_LTDC_MODULE_ENABLED */
/* #define HAL_RNG_MODULE_ENABLED */
/* #define HAL_RTC_MODULE_ENABLED */
/* #define HAL_SAI_MODULE_ENABLED */
/* #define HAL_SD_MODULE_ENABLED */
/* #define HAL_MMC_MODULE_ENABLED */
#define HAL_SPI_MODULE_ENABLED // This might need to be enabled manually
#define HAL_TIM_MODULE_ENABLED // This might need to be enabled manually
#define HAL_UART_MODULE_ENABLED // This might need to be enabled manually
//...

```

5. Now you should be able to build the project with CMake

```bash
cmake -B build -G Ninja
cmake --build build
```

## Adding source files and libraries in CMakeLists.txt

There are highlihgted section in the CMakeLists.txt file where you can add your source files and libraries.

```cmake
cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME embeded_stmepic_base_project)

############################################################################################################
# C++ Configuration  [1/4]
set(CMAKE_CXX_STANDARD 17)

# aditional compiler flags
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -u_printf_float -Wreturn-type")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}  -u_printf_float -Wreturn-type")

# aditional linker flags
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS}  -u_printf_float -Wreturn-type")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_C_LINK_FLAGS}  -u_printf_float -Wreturn-type")

## enable ccache if available
find_program(CCACHE_PROGRAM ccache)
if(NOT CCACHE_PROGRAM)
    message(STATUS "ccache not found")
else()
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

############################################################################################################

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

############################################################################################################
# C++ Configuration  [2/4]

# Enable CMake support for ASM and C languages
enable_language(CXX C ASM)

############################################################################################################

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)


############################################################################################################
# Add sources subdirectory and include directories link libriries  [3/4]
# congigure the project here
set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "") # set appropriate port for your microcontroller
set(FREERTOS_HEAP 4 CACHE STRING "") # set appropriate heap implementation
set(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src") # set appropriate path to FreeRTOSConfig.h config file
add_subdirectory(StmEpic)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  # Add user sources here
  "src/main_prog.cpp"
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  # Add user defined include paths
  "src"
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
  stm32cubemx
  stmepic
    # Add user defined libraries
)
############################################################################################################

############################################################################################################
# Configure nice task to make life easy  [4/4]

add_custom_command(
	TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf firmware.bin
	COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf firmware.hex
	COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_PROJECT_NAME}.elf firmware.elf
  # Show ccache statistics
 )

if(CCACHE_PROGRAM)
  add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CCACHE_PROGRAM} -s
  )
endif()
```

# Example project using the StmEpic library

- [StmEpic blank example project](https://github.com/KoNarRobotics/embeded_stmepic_base_project)
- [StmEpic used in drivers for 6dof manipualotor ](https://github.com/KoNarRobotics/konarm_software_low)
