cmake_minimum_required(VERSION 3.15)


set(UPPER_PROJECT_NAME ${CMAKE_PROJECT_NAME})
add_library(stmepic INTERFACE)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17 PARENT_SCOPE)
set(CMAKE_CXX_STANDARD_REQUIRED True PARENT_SCOPE)
set(EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)

# cmake_policy(SET CMP0076 NEW)

############################################
# STMEPIC MODULES OPTIONS
option(STMEPIC_GENERATE_DOCS "Enable documentaion genration" OFF)
option(STMEPIC_ENABLE_EMBEDED_FREERTOS "Enable Memory module" OFF)
option(STMEPIC_FDCAN "Enable FDCAN Controler / if not enabled regualr CAN in enabled" OFF)
option(STMEPIC_ENCODERS "Enable Encoders module" ON)
option(STMEPIC_LOGGER "Enable LOGGER module" ON)
option(STMEPIC_MOVEMENT "Enable Movement controler module" ON)
option(STMEPIC_SENSORS "Enable Sensors module" ON)
option(STMEPIC_DFU_PROGRAMING "Enable Usb programing module" ON)
option(STMEPIC_MEMORY "Enable Memory module" ON)


############################################
# CORE STMEPIC MODULES
# add_subdirectory(src/Containers)
add_subdirectory(src/Timing)
add_subdirectory(src/Filters)
add_subdirectory(src/Inc)
add_subdirectory(src/Device)
add_subdirectory(src/Algorithm)


############################################
# STMEPIC OPTIONAL MODULES
# if(STMEPIC_CAN_CONTROLER)
#   add_subdirectory(src/CanControl)
# endif()

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


############################################
# LIBRARIES

set(LIBRARIES_INCLUDED "")


############################################
# Embedded Template Library (ETL)
add_subdirectory(etl)
list(APPEND LIBRARIES_INCLUDED etl::etl)

############################################
# FREE RTOS CONFIGURATION
if(STMEPIC_ENABLE_EMBEDED_FREERTOS)
  if(NOT FREERTOS_PORT)
    message(FATAL_ERROR 
      "Please set the FREERTOS_PORT variable this will set the port of the FreeRTOS like GCC_ARM_CM4F or GCC_ARM_CM7")
  endif()

  if(NOT FREERTOS_HEAP)
    message(FATAL_ERROR 
      "Please set the FREERTOS_HEAP variable this will set the heap implementation of the FreeRTOS like 1-5 ")
  endif()

  if(NOT FREERTOS_CONFIG_FILE_DIRECTORY)
    message(FATAL_ERROR 
      "Please set the FREERTOS_CONFIG_FILE_DIRECTORY variable this will set the directory of the 
      FreeRTOSConfig.h file you can see example fiel in StmEpic/templates/FreeRTOSConfig.h")
  endif()

  add_library(freertos_config INTERFACE)
  target_include_directories(freertos_config
    INTERFACE
    ${FREERTOS_CONFIG_FILE_DIRECTORY}
  )
  add_subdirectory(FreeRTOS-Kernel)
  list(APPEND LIBRARIES_INCLUDED freertos_kernel freertos_config)
endif()

############################################
## LINK ALL THE STMEPIC MODULES
target_link_libraries(stmepic INTERFACE ${LIBRARIES_INCLUDED})


############################################
## C++ compiler flags
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}" PARENT_SCOPE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -u_printf_float -Wreturn-type -ffunction-sections -fdata-sections " PARENT_SCOPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -u_printf_float -Wreturn-type -ffunction-sections -fdata-sections -fno-exceptions " PARENT_SCOPE)

# additional linker flags

set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS}  -u_printf_float -Wreturn-type -print-memory-usage -lrdimon --specs=rdimon.specs" PARENT_SCOPE)
# set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} ")


set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS}  -u_printf_float -Wreturn-type -Wl,--gc-sections -lrdimon --specs=rdimon.specs" PARENT_SCOPE)

enable_language(CXX C ASM)

############################################
# DOCUMENTATION GENERATION
if(STMEPIC_GENERATE_DOCS)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    message("Doxygen build started in: ${CMAKE_CURRENT_SOURCE_DIR}")
    add_custom_target( doc_doxygen ALL
      COMMAND ${DOXYGEN_EXECUTABLE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Generating API documentation with Doxygen ${CMAKE_CURRENT_SOURCE_DIR}"
      VERBATIM 
    )
  else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
  endif (DOXYGEN_FOUND)
endif()
