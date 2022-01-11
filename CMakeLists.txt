# ----------------------------------------------------------------------------
# 
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.2)
include_directories(${CMAKE_BINARY_DIR}/libpng)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}) 
set(CMAKE_C_FLAGS "/EHsc")
set(CMAKE_CXX_FLAGS "/EHsc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
set (CMAKE_CXX_STANDARD 17)

project(fontToPNG)

# ----------------------------------------------------------------------------   
# 
# ----------------------------------------------------------------------------
include_directories(./)
include_directories(freetype/include)
include_directories(libpng)
include_directories(zlib)


add_subdirectory(zlib)
add_subdirectory(libpng)
add_subdirectory(freetype)

add_definitions(-D_USE_MATH_DEFINES)
if(MSVC)
	add_definitions(/W4)
else()
	add_definitions(-Wall -Wextra -Wpedantic)
endif()

find_package(OpenMP)
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_LIBRARY_ARCHITECTURE x64)
  else()
    set(CMAKE_LIBRARY_ARCHITECTURE x86)
  endif()
endif()


set(SRC_MAIN
	main.cpp
)

add_executable(
	${PROJECT_NAME}
	${SRC_MAIN}
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE			  
					  freetype
					  png_static
					  zlib
					  Threads::Threads)
if(MSVC)
# ----------------------------------------------------------------------------
# 
# ----------------------------------------------------------------------------
set_target_properties( ${project_name} PROPERTIES  VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${project_name})
endif()

