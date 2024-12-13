# StmEpic library
This library is a collection of algoritms and interfaces to be used in robotics projects. The library is designed to be used with the STM32 series of microcontrollers. 
The library provides a set of classes that can be used to quickly combine bunch of sensors and other devices like motors and so on, in a meaningful way that they can work with each other out of the box. 
The library is written in C++ and is designed to be used with the code generation tool STM32CubeMX for super quick startup. 


## Features
- Included ***ETL*** library for containers and so on [link](https://github.com/ETLCPP/etl).
- ***Status*** for proper error handling.
- **GPIO**: control the GPIO pins on the microcontroller.
- **BOARD_ID**: Can be used to read the unique ID of the microcontroller from hex-encoder.
- **CanControler**: Read and write data from the CAN bus, with easy to use callbacks.
- **Encoders**: Interface to read data from different types of encoders.
- **Filters**: set of filters that can be used to filter data.
- **Logger**:  Log data to the console via USB in json format with usefull fields like current time or software version and so on (debug,info,warning,and error).
- **Movement**: Interface that can be used to control different type of engines STEPERS, BLDC, dc and so on. Using the same control algoritms and interfaces.
- **Timing**:  Interface to add task that run at very specifi fequency (it's not a replacement for RTOS, it's simple if stemnt replacement with proper time handling).
- **Sensors**: Set of classes that can be used to read data from different sensors.
- **Dfu**: Class that can be used to enable the USB as programing using DFU [hardware support required].

## How to add the library to your project
1. Clone the repository to your project directory.
2. Add the following line to your CMakeLists.txt file:
```cmake
add_subdirectory(StmEpic)
target_link_libraries( <YOUR_PROEJCT_NAME> PUBLIC  etl::etl)
```
3. Now you can simply include header files.


## How to use the library
The library is designed to be used with the code generation tool STM32CubeMX.
1. Create a new project in STM32CubeMX.
2. In the project settings, set generate code as CMakelist.
3. And include all source files from the library to the project.
4. don't forget to have ARM GCC toolchain installed. You can download it from [here](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm) or get docker image to build the project from [here](https://hub.docker.com/repository/docker/xlemonx/arm-gnu-toolchain).


