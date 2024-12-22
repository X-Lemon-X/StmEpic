
# cmake_minimum_required(VERSION 3.15)

# Set the project name and version
# project(${UPPER_PROJECT_NAME} VERSION 1.0)

# add_library(${UPPER_PROJECT_NAME} MODULE)

set(UPPER_PROJECT_NAME ${CMAKE_PROJECT_NAME})


# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

cmake_policy(SET CMP0076 NEW)

option(STMEPIC_BARD_ID "Enable BoardID" ON)
option(STMEPIC_CAN_CONTROLER "Enable CAN Controler" ON)
option(STMEPIC_ENCODERS "Enable Encoders module" ON)
option(STMEPIC_LOGGER "Enable LOGGER module" ON)
option(STMEPIC_MOVEMENT "Enable Movement controler module" ON)
option(STMEPIC_SENSORS "Enable Sensors module" ON)
option(STMEPIC_DFU_PROGRAMING "Enable Usb programing module" ON)
option(STMEPIC_MEMORY "Enable Memory module" ON)


# Add the core modules
add_subdirectory(src/Containers)
add_subdirectory(src/Timing)
add_subdirectory(src/Gpio)
add_subdirectory(src/Filters)
add_subdirectory(src/config)
add_subdirectory(src/Status)
add_subdirectory(src/Device)
add_subdirectory(src/Hash)
add_subdirectory(etl)


# Add the extra modules
if(STMEPIC_BARD_ID)
  add_subdirectory(src/BoardId)
endif()

if(STMEPIC_CAN_CONTROLER)
  add_subdirectory(src/CanControl)
endif()


if(STMEPIC_ENCODERS)
  add_subdirectory(src/Encoders)
endif()

if(STMEPIC_LOGGER)
  add_subdirectory(src/Logger)
endif()

if(STMEPIC_MOVEMENT)
  add_subdirectory(src/Movement)
  add_subdirectory(src/Motor)
endif()

if(STMEPIC_SENSORS)
  add_subdirectory(src/Sensors)
endif()

if(STMEPIC_DFU_PROGRAMING)
  add_subdirectory(src/DFU)
endif()

if(STMEPIC_MEMORY)
  add_subdirectory(src/Memory)
endif()



find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)


