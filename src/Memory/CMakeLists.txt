

target_include_directories(${UPPER_PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
  $<INSTALL_INTERFACE:include/${UPPER_PROJECT_NAME}> 
)

target_sources(${UPPER_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/memory_fram.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fram_i2c.cpp
)