

target_include_directories(${UPPER_PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
  $<INSTALL_INTERFACE:include/${UPPER_PROJECT_NAME}> 
)

target_sources(${UPPER_PROJECT_NAME} PRIVATE
  src/can_control.cpp
)