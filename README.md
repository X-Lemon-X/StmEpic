<p align="center">
  <img src="https://stmepic.d3lab.dev/stmepic-high-resolution-logo-transparent.webp" alt="StmEpic Logo" width="40%"/>
</p>

## StmEpic library

<p align="center">
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/formater.yml/badge.svg?branch=main" alt="formater"/>
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/build_gnu_arm14.yml/badge.svg?branch=main" alt="gnu14"/>
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/build_gnu_arm13.yml/badge.svg?branch=main" alt="gnu13"/>
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/build_gnu_arm12.yml/badge.svg?branch=main" alt="gnu12"/>
  <img src="https://github.com/X-Lemon-X/StmEpic/actions/workflows/build_gnu_arm11.yml/badge.svg?branch=main" alt="gnu11"/>
</p>

This library provides a collection of proven algorithms and well‑defined interfaces targeting robotics applications. It is implemented in modern C++ (C++20) and is intended for use with STM32 microcontrollers and CMake‑based build systems.

StmEpic supplies a cohesive set of components—drivers, sensor and motor abstractions, control algorithms, memory utilities and logging—that are designed to interoperate with minimal configuration. Components are organized to simplify integration of sensors, actuators and communication peripherals into real-time systems.

The framework integrates with FreeRTOS for task scheduling and is compatible with STM32CubeMX generated projects to accelerate development and hardware configuration.

## Features
- **C++20** — Modern C++ language features and idioms.
- **CMake** — Cross‑platform build and configuration system.
- **STM32CubeMX integration** — Project generation and peripheral configuration.
- **FreeRTOS** — Real‑time task scheduler for embedded applications.
- **ETL (Embedded Template Library)** — Lightweight, header‑only containers and utilities. [link](https://github.com/ETLCPP/etl)
- **Device** — Unified device abstraction for status, reset and connection management.
- **DFU** — USB Device Firmware Upgrade support (hardware dependent).
- **Drivers** — RTOS‑aware, thread‑safe drivers for I2C, SPI, UART and CAN; designed for concurrent, non‑blocking access.
- **Encoders** — Interfaces for common encoder types.
- **Filters** — Signal processing primitives and filter implementations.
- **GPIO** — Portable GPIO abstraction layer.
- **Hash** — Hashing utilities with planned hardware acceleration support.
- **Logger** — Structured JSON logging over USB, with timestamps, levels and metadata.
- **Memory** — Utilities for FRAM and FLASH access and data framing.
- **Motors** — Common control interfaces and implementations for stepper, BLDC and DC motors.
- **Movement** — Motion control algorithms and closed‑loop controllers.
- **Sensors** — Drivers and interfaces for a variety of sensors.
- **Status** — Consistent result and error handling types.
- **Timing** — Software timers and scheduling helpers.

## Tested STM32 microcontrollers series
- **STM32F1** - 103
- **STM32F4** - 405, 412, 446
- **STM32H5** - 563
- **STM32H7** - 750
- **STM32U5** - 585
Disclaimer: The library will most likely work with other STM32 series, but it wasn't tested.

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
