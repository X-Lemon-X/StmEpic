

target_include_directories(${UPPER_PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/${UPPER_PROJECT_NAME}> 
)

target_sources(${UPPER_PROJECT_NAME} PRIVATE
  i2c.cpp
  device.cpp
  gpio.cpp
  uart.cpp
  simple_task.cpp
)


if(STMEPIC_FDCAN)
  target_include_directories(${UPPER_PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/can/fdcan>
    $<INSTALL_INTERFACE:include/${UPPER_PROJECT_NAME}> 
  )
  target_sources(${UPPER_PROJECT_NAME} PRIVATE
    can/fdcan/fdcan.cpp
  )
endif()

if(NOT STMEPIC_FDCAN)
  target_include_directories(${UPPER_PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/can/can>
    $<INSTALL_INTERFACE:include/${UPPER_PROJECT_NAME}> 
  )
  target_sources(${UPPER_PROJECT_NAME} PRIVATE
  can/can/can2.0.cpp
  ) 

endif()