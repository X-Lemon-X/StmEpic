<p align="center">
  <img src="https://stmepic.nihilia.xyz/stmepic-high-resolution-logo-transparent.webp" alt="StmEpic Logo" width="40%"/>
</p>

## StmEpic library

<p align="center">
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/formater.yml/badge.svg?branch=main" alt="formater"/>
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/build_gnu_arm14.yml/badge.svg?branch=main" alt="gnu14"/>
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/build_gnu_arm13.yml/badge.svg?branch=main" alt="gnu13"/>
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/build_gnu_arm12.yml/badge.svg?branch=main" alt="gnu12"/>
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/build_gnu_arm11.yml/badge.svg?branch=main" alt="gnu11"/>
</p>

This library is a collection of algorithms and interfaces to be used in robotics projects. The library is designed to be used with the STM32 series of microcontrollers, C++ and Cmake.

The library provides a set of classes and interfaces that can be used to quickly combine bunch of sensors, engines, servos and communication protocols in a meaningful way. The library aims to provide elements that can work with each other out of the box with little to none configuration.

All is glued together with the FreeRTOS system. Most of the devices run tasks in the background to handle trafic.
The library is written in C++ and is designed to be used with the code generation tool STM32CubeMX for super quick startup and changes.

## Features

- **FreeRTOS** embedded FreeRTOS as task scheduler.
- **ETL** Included **Embedded Template Library** library for containers and so on [link](https://github.com/ETLCPP/etl).
- **Device**: Interface to handle device status, reset or connection, Allow to handle devices like ICs to other Boards or drivers with unified interface.
- **Dfu**: Class that can be used to enable the USB as programming using DFU [hardware support required].
- **Drivers**: Set of drivers that enable usage of interfaces like I2C, SPI, UART, CAN with FreeRtos with ease out of the box, adding few useful features in using them.
  Interfaces drivers allows multiple tasks to use them without blocking the CPU while simultaneously reading and writing data for the interacting thread, as well as add handy interfaces to interact with specific protocols in a simple way.
- **Encoders**: Interface to read data from different types of encoders.
- **Filters**: Set of filters that can be used to filter data.
- **Gpio**: Warper for the GPIO pins on the microcontroller.
- **Hash**: Hashing algorithms, hardware support Coming soon.
- **Logger**: Log data to the console via USB in json format with useful fields like current time or software version and so on (debug,info,warning,and error).
- **Memory**: Set of classes that can be used to read and write data to FRAM or FLASH memory.
- **Motors**: Interface that can be used to control different type of engines STEPERS, BLDC, DC motors. With the same control algorithms and interfaces.
- **Movement**: Interface and algorithms that can be used to control different motors, usually with closed loop control.
- **Sensors**: Set of classes that can be used to read data from different sensors.
- **Status** Return types for proper error handling.
- **Timing**: Interface to create software timers for some stuff.

## Tested STM32 microcontrollers series

Disclaimer: The library will most likely work with other STM32 series, but it wasn't tested.

- **STM32F4**
- **STM32H7** (in progress)
- **STM32U5** (in progress)

## How to add the library to your project

1. Clone the repository to your project directory **DON'T FORGET TO INITIALIZE SUBMODULES**.
2. Add the following line to your CMakeLists.txt file:

```cmake
add_subdirectory(StmEpic)
target_link_libraries( <YOUR_PROEJCT_NAME> PUBLIC  stmepic)
```

3. Now you can simply include header files.

## How to use the library

The library is designed to be used with the code generation tool STM32CubeMX.

1. Create a new project in STM32CubeMX.
2. In the project settings, set generate code as CMakelist.
3. Enable FreeRTOS/CMSISV2 in the project settings.
4. Include all source files from the library to the project.
5. Don't forget to have ARM GCC toolchain installed. You can download it from [here](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm) or get docker image to build the project from [here](https://hub.docker.com/repository/docker/xlemonx/arm-gnu-toolchain).

## Documentation

You can find StmEpic documentation on our [website](https://stmepic.d3lab.dev).
