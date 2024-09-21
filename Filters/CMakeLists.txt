

target_include_directories(${UPPER_PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
  $<INSTALL_INTERFACE:include/${UPPER_PROJECT_NAME}> 
)

target_sources(${UPPER_PROJECT_NAME} PRIVATE
  src/filter.cpp
  src/filter_alfa_beta.cpp
  src/filter_moving_avarage.cpp
)