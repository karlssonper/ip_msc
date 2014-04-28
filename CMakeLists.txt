CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(IMAGE_PROCESSING)


set(SOURCE base)
set(LIBRARIES)

option(OpenCL "OpenCL" ON)
if(OpenCL)
  message("-- Generating build with OpenCL...\n")
  INCLUDE_DIRECTORIES($ENV{OPENCL_INCLUDE_DIR})  
  LINK_DIRECTORIES($ENV{OPENCL_LIB_DIR})


  set(SOURCE ${SOURCE} opencl)  

  set(LIBRARIES ${LIBRARIES} OpenCL)
endif(OpenCL)

ADD_LIBRARY(ip ${SOURCE})
TARGET_LINK_LIBRARIES(ip ${LIBRARIES})


enable_testing()

add_executable(testOpenCL test)
target_link_libraries(testOpenCL ip)
add_test(NAME testOpenCL
         COMMAND testOpenCL)
