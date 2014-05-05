CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(IMAGE_PROCESSING)

find_package(Boost 1.45.0)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
endif(Boost_FOUND)


set(SOURCE base)
set(LIBRARIES)

option(OpenCL "OpenCL" ON)
if(OpenCL)
  if (APPLE)
     FIND_LIBRARY(OPENCL_LIBRARY OpenCL DOC "OpenCL lib for OSX")
     LINK_DIRECTORIES(${OPENCL_LIBRARY})
	 message(${OPENCL_LIBRARY})
	 set(LIBRARIES ${LIBRARIES} ${OPENCL_LIBRARY})
     FIND_PATH(OPENCL_INCLUDE_DIR OpenCL/cl.h DOC "Include for OpenCL on OSX")
  	 message("Apple found")
  endif(APPLE)


  message("-- Generating build with OpenCL...\n")
  INCLUDE_DIRECTORIES($ENV{OPENCL_INCLUDE_DIR})  
  LINK_DIRECTORIES($ENV{OPENCL_LIB_DIR})


  set(SOURCE ${SOURCE} opencl)  

  set(LIBRARIES ${LIBRARIES})
endif(OpenCL)

ADD_LIBRARY(ip ${SOURCE})
TARGET_LINK_LIBRARIES(ip ${LIBRARIES})


enable_testing()

add_executable(testOpenCL test)
target_link_libraries(testOpenCL ip)
add_test(NAME testOpenCL
         COMMAND testOpenCL)
