CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(IMAGE_PROCESSING)

#SET(CMAKE_CXX_COMPILER  /opt/local/bin/g++-mp-4.4)
#SET(CMAKE_C_COMPILER  /opt/local/bin/gcc-mp-4.4 )

#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.8)

set(SOURCE src/base)
set(LIBRARIES)


# compiler definitions for non-windows builds
if (NOT WIN32)
  add_definitions(-fPIC)
endif()

option(OpenCL "OpenCL" ON)
if(OpenCL)
  message("-- Generating build with OpenCL...\n")
  if (APPLE)
     FIND_LIBRARY(OPENCL_LIBRARY OpenCL DOC "OpenCL lib for OSX")
     LINK_DIRECTORIES(${OPENCL_LIBRARY})
	 message(${OPENCL_LIBRARY})
	 set(LIBRARIES ${LIBRARIES} ${OPENCL_LIBRARY})
     FIND_PATH(OPENCL_INCLUDE_DIR OpenCL/cl.h DOC "Include for OpenCL on OSX")
  	 message("Apple found")
  endif(APPLE)

  INCLUDE_DIRECTORIES($ENV{OPENCL_INCLUDE_DIR})  
  LINK_DIRECTORIES($ENV{OPENCL_LIB_DIR})

  set(SOURCE ${SOURCE} src/opencl)  

  set(LIBRARIES ${LIBRARIES} OpenCL)
endif(OpenCL)

option(CUDA "CUDA" ON)
if (CUDA)
  find_package(CUDA REQUIRED)
  include_directories(${CUDA_INCLUDE})

  set(SOURCE ${SOURCE} src/ccuda)  

  set(LIBRARIES ${LIBRARIES} cuda cudart)

endif(CUDA)

option(OpenGL "OpenGL" ON)
if(OpenGL)
    message("-- Generating build with OpenGL...\n")
	find_package(OpenGL REQUIRED)   
	find_package(GLUT REQUIRED)
	find_package(GLEW REQUIRED)
	INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})  

	set(SOURCE ${SOURCE} src/opengl)  
	set(LIBRARIES ${LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY})
endif(OpenGL)

ADD_LIBRARY(ip ${SOURCE})
TARGET_LINK_LIBRARIES(ip ${LIBRARIES})

option(Python "Python" ON)
if (Python)

    #SET(Boost_USE_STATIC_LIBS OFF)
	#SET(Boost_USE_MULTITHREADED ON)		
    #SET(Boost_USE_STATIC_RUNTIME OFF)
    find_package(Boost 1.45.0 COMPONENTS python)
	find_package(PythonLibs)
	INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIR})
  
   include_directories(/Users/per/Downloads/boost_1_55_0)
   add_library(PyIp SHARED src/python.cpp )
   LINK_DIRECTORIES(/Users/per/Downloads/boost_1_55_0/stage/lib)
   set_target_properties( PyIp PROPERTIES PREFIX "")
   set_target_properties( PyIp PROPERTIES SUFFIX ".so")
   target_link_libraries(PyIp ip boost_python  ${PYTHON_LIBRARIES} boost_numpy)
endif(Python)





enable_testing()

add_executable(test test/test)
target_link_libraries(test ip)
add_test(NAME test
         COMMAND test)

