
# cmake_minimum_required(VERSION 3.15)

# Set the project name and version
# project(${UPPER_PROJECT_NAME} VERSION 1.0)

# add_library(${UPPER_PROJECT_NAME} MODULE)

set(UPPER_PROJECT_NAME ${CMAKE_PROJECT_NAME})


# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

cmake_policy(SET CMP0076 NEW)

option(STMEPIC_BARD_ID "Enable BoardID" OFF)
option(STMEPIC_CAN_CONTROLER "Enable CAN Controler" OFF)
option(STMEPIC_ENCODERS "Enable Encoders module" OFF)
option(STMEPIC_LOGGER "Enable LOGGER module" OFF)
option(STMEPIC_MOVEMENT "Enable Movement controler module" OFF)
option(STMEPIC_SENSORS "Enable Sensors module" OFF)
option(STMEPIC_USB_PROGRAMING "Enable Usb programing module" OFF)

# Add the library target

# find_library(stm32cubemx REQUIRED)
# find_package(stm32cubemx REQUIRED)

# Add the core modules
add_subdirectory(Containers)
add_subdirectory(Timing)
add_subdirectory(Gpio)
add_subdirectory(Filters)

# add_subdirectory(../cmake/stm32cubemx)

# Add the extra modules

if(STMEPIC_BARD_ID)
  add_subdirectory(BoardId)
endif()

if(STMEPIC_CAN_CONTROLER)
  add_subdirectory(CanControl)
endif()


if(STMEPIC_ENCODERS)
add_subdirectory(Encoders)
endif()

if(STMEPIC_LOGGER)
add_subdirectory(Logger)
endif()

if(STMEPIC_MOVEMENT)
  add_subdirectory(MovementControl)
  add_subdirectory(SteperMotor)
endif()

if(STMEPIC_SENSORS)
  add_subdirectory(Sensors)
endif()

if(STMEPIC_USB_PROGRAMING)
  add_subdirectory(UsbPrograming)
endif()

link_libraries(${UPPER_PROJECT_NAME}
  stm32cubemx
)




